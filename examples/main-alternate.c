/*
State machine prototype. Adapt to new program in main.c.
*/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include "io.h"
#include "lcd_twi.h"
#include "steppermotor.h"
#include "ds3231.h"
#include "dht11.h"

// Parameters.
#define VENT_DIRECTION_THRESHOLD 50     // Debouncing threshold, also reduces false movement due to noise
#define STEPS_PER_ANALOG_UNIT 10        // Number of stepper motor steps to take per unit difference between the last "move" and this "move".
#define WATER_LEVEL_THRESHOLD 256       // Analog threshold of water sensor before transitioning to ERROR.
#define TEMPERATURE_THRESHOLD 0       // Minimum temperature threshold for the fan to run.

// Pin definitions.
#define RED_LED_DIGITAL_PIN 28
#define YELLOW_LED_DIGITAL_PIN 26
#define GREEN_LED_DIGITAL_PIN 24
#define BLUE_LED_DIGITAL_PIN 22

#define FAN_MOTOR_DIGITAL_PIN 52

#define STEPPER_MOTOR_DIGITAL_PIN_1 38
#define STEPPER_MOTOR_DIGITAL_PIN_2 40
#define STEPPER_MOTOR_DIGITAL_PIN_3 42
#define STEPPER_MOTOR_DIGITAL_PIN_4 44

#define DHT11_DIGITAL_PIN 50

// ADC channels.
#define STEPPER_POT_ANALOG_PIN 0        // Assuming we want a potentiometer for the vent direction
#define WATER_SENSOR_ANALOG_PIN 1

// Button interrupt definitions. 
//
// there's probably a better way to be defining these as pins-to-vectors
//
// also, i think we should prefer INTn (external interrupts) to PCINTn (pin interrupts) since:
// - no additional logic needed to figure out which pin
//   threw an interrupt (which is needed in PCINTn, usually)
// - allows us to have the interrupt only occur on rising edge,
//   which eliminates the need to debounce the falling edge
// - no need for software interrupts

// if you change these, you must also change EICRA/B and EIMSK below!!
#define START_BUTTON_INTERRUPT_VECTOR INT2_vect
#define STOP_BUTTON_INTERRUPT_VECTOR INT3_vect
#define RESET_BUTTON_INTERRUPT_VECTOR INT4_vect
#define TIMER_INTERRUPT_VECTOR TIMER1_OVF_vect

#define LCD_ADDR 0x27
#define RTC_ADDR 0x68

// State definitions.
typedef enum {
    DISABLED = 0, 
    IDLE = 1, 
    RUNNING = 2,
    ERROR = 3
} program_state_t;

void transition_to_disabled();
void transition_to_idle();
void transition_to_running();
void transition_to_error();

program_state_t state = DISABLED;

/* Global definitions. */
pin_t red_led;
pin_t yellow_led;
pin_t green_led;
pin_t blue_led;
pin_t* leds[] = {&yellow_led, &green_led, &blue_led, &red_led};

pin_t fan_motor;

stepper_t stepper;

FILE lcd;

dht11_t dht11_data;

// If internal clock approach taken
uint8_t seconds_since_last_update = 0;

// If RTC polled
uint8_t minute_of_last_update;

void log_message(char* message){
    char time_str[100]; // TODO: "memory safety"
    time_t time = get_time(RTC_ADDR);
    
    tsnprintf(time_str, 100, time);
    printf("[%s] %s\n", time_str, message);
}

void write_lcd(char* line_2_msg){
    dht11_data = dht11_read(DHT11_DIGITAL_PIN);
    
    lcd_twi_clear(&lcd);

    // Best to write a conditional here so it's clearer what these are
    // unless a message is needed
    lcd_twi_cursor(&lcd, 0, 0);
    fprintf(&lcd, "%.1fC", dht11_data.temp);

    lcd_twi_cursor(&lcd, 6, 0);
    fprintf(&lcd, "H:%.1f%%", dht11_data.humidity);

    lcd_twi_cursor(&lcd, 0, 1);
    fprintf(&lcd, line_2_msg);
}

// Set the LED we want on (and none of the other LEDs)
void set_leds(program_state_t state){
    for(int i = 0; i < 4; i++){
        pin_t* led = leds[i];
        if(state == i){
            write_pin(*led, HIGH);
        }else{
            write_pin(*led, LOW);
        }
    }
}

void stop_second_timer(){
    TCCR1B &= 0xF8;
}

void start_second_timer(){
    TCNT1 = 65536-15625; // Exact number of ticks for one second interrupts
    TCCR1B = 0xFD; // Max prescaler, all other "normal" options (enables timer)
}


void setup(){
    // Enable interrupts
    sei();

    // Set up button interrupts (INT2, INT3, INT4)
    // Interrupt on rising edge only (not on all logical changes)
    EICRA |= 0b11110000;
    EICRB |= 0b00000011;
    // Enable corresponding external pin interrupts
    EIMSK |= 0b00011100;

    // Set up 1-second timer
    //TIMSK1 = 0b00000001; // Enable timer interrupts
    start_second_timer();

    // Initalize UART, 9600 baud
    uart_init(9600);

    // Initialize TWI for 2x16LCD
    twi_init();

    // Enable ADC
    adc_init();

    // Initialize everything
    red_led = new_pin(RED_LED_DIGITAL_PIN);
    yellow_led = new_pin(YELLOW_LED_DIGITAL_PIN);
    green_led = new_pin(GREEN_LED_DIGITAL_PIN);
    blue_led = new_pin(BLUE_LED_DIGITAL_PIN);
    pin_mode(red_led, OUTPUT);
    pin_mode(yellow_led, OUTPUT);
    pin_mode(green_led, OUTPUT);
    pin_mode(blue_led, OUTPUT);

    fan_motor = new_pin(FAN_MOTOR_DIGITAL_PIN);
    pin_mode(fan_motor, OUTPUT);

    stepper = stepper_init(
        STEPPER_MOTOR_DIGITAL_PIN_1,
        STEPPER_MOTOR_DIGITAL_PIN_2,
        STEPPER_MOTOR_DIGITAL_PIN_3,
        STEPPER_MOTOR_DIGITAL_PIN_4
    );
    
    lcd = lcd_twi_init(LCD_ADDR);

    // Update LCD
    write_lcd("");

    minute_of_last_update = get_time(RTC_ADDR).min;

	// Initial transition
	transition_to_disabled();
}

int main(){
    // Contains the last ADC value for the vent direction potentiometer
    // that was used to change the vent direction.
    uint16_t last_pot_change_value = 0;
    uint16_t water_level;

    setup();

    for(;;){
        // Polling approach
		/*
        time_t current_time = get_time(RTC_ADDR);
        if(current_time.sec == 0 && minute_of_last_update != current_time.min){
            if(state == ERROR){
                write_lcd("Water low");
            }else{
                write_lcd("");
            }

			minute_of_last_update = current_time.min;
        }
		*/
        
        // Timer interrupt approach
        if(seconds_since_last_update >= 60){
            if(state == ERROR){
                write_lcd("Water low");
            }else{
                write_lcd("");
            }
        }
		
        if(state == ERROR){
            // The only way to transition out of ERROR is to press the
            // reset button. There's nothing else special to do here.
            continue;
        }

        // Has the potentiometer changed by a meaningful amount?
        // If so, trigger a change on the stepper motor
        // BUG: this doesn't work!!
        /*
        uint16_t current_pot_value = adc_read(STEPPER_POT_ANALOG_PIN);
        if (abs(current_pot_value - last_pot_change_value) > VENT_DIRECTION_THRESHOLD){
            int32_t difference = current_pot_value - last_pot_change_value;
            int16_t steps = difference * STEPS_PER_ANALOG_UNIT;
            stepper_rotate(stepper, steps);
            last_pot_change_value = current_pot_value;
        }
        */

        if(state == DISABLED){
            // Don't do anything else.
            continue;
        }

        // implied: program state is one of IDLE, RUNNING
        water_level = adc_read(WATER_SENSOR_ANALOG_PIN);
        if (water_level <= WATER_LEVEL_THRESHOLD){
            transition_to_error();
        }

        dht11_data = dht11_read(DHT11_DIGITAL_PIN);
        if (dht11_data.temp <= TEMPERATURE_THRESHOLD && state == RUNNING){
            transition_to_idle();
        } else if (dht11_data.temp > TEMPERATURE_THRESHOLD && state == IDLE){
            transition_to_running();
        }
    }
}

/* State transition handlers */

// Yellow LED on
// No monitoring of temp or water
// Start button monitored with ISR
void transition_to_disabled(){
    // Record transition time to UART
    log_message("Transition to DISABLED");
    
    // Disable the timer that's updating the LCD screen
    stop_second_timer();
    
    // Turn on the LED
    set_leds(DISABLED);

    // Turn off the fan motor
    write_pin(fan_motor, LOW);

    // Set global program state to DISABLED
    state = DISABLED;
}


// Exact time stamp (using real time clock) should record transition times
// Water level should be continuously monitored and state changed to error if level is too low
// GREEN LED should be ON
void transition_to_idle(){
    // Record transition time to UART
    log_message("Transition to IDLE");
    
    // Enable the timer that's updating the LCD screen
    start_second_timer();
    
    // Turn on the LED
    set_leds(IDLE);

    // Turn off the fan motor
    write_pin(fan_motor, LOW);

    // Set global program state to IDLE
    state = IDLE;
}

// Fan motor should be on
// System should transition to IDLE as soon as temperature drops below threshold
// System should transition to ERROR state if water becomes too low
// BLUE LED should be turned on (all other LEDs turned off)
void transition_to_running(){
    // Record transition time to UART
    log_message("Transition to RUNNING");
    
    // Turn on the fan motor
    write_pin(fan_motor, HIGH);
    
    // Turn on the LED
    set_leds(RUNNING);

    // Set global program state to RUNNING
    state = RUNNING;
}

// Motor should be off and not start regardless of temperature
// A reset button should trigger a change to the IDLE stage if the water level is above the threshold
// Error message should be displayed on LCD
// RED LED should be turned on (all other LEDs turned off)
void transition_to_error(){
    // Record transition time to UART
    log_message("Transition to ERROR");

    // Write message to LCD
    write_lcd("Water low");
    
    // Turn off the fan motor
    write_pin(fan_motor, LOW);

    // Turn on red LED
    set_leds(ERROR);

    // Set global program state to ERROR
    state = ERROR;
}

/* ISRs */
ISR(START_BUTTON_INTERRUPT_VECTOR){
    if(state == DISABLED){
        transition_to_idle();
    }
}

ISR(STOP_BUTTON_INTERRUPT_VECTOR){
    if(state == RUNNING || state == IDLE){
        transition_to_disabled();
    }
}

ISR(RESET_BUTTON_INTERRUPT_VECTOR){
    // strictly speaking, the program will jump straight to ERROR if the water
    // level isn't high enough
    if(state == ERROR){
        transition_to_idle();
    }
}

// TODO: is this how we want to update the LCD every minute?

ISR(TIMER_INTERRUPT_VECTOR){
    stop_second_timer();

    seconds_since_last_update++;

    start_second_timer();
}
