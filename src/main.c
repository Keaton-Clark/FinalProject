#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "io.h"
#include "lcd_twi.h"
#include "steppermotor.h"
#include "ds3231.h"
#include "dht11.h"

// Parameters.
#define VENT_DIRECTION_THRESHOLD 50     // Debouncing threshold, also reduces false movement due to noise
// It takes ~500-600 steps for a full rotation, so divide by 2 to map 0-1024
#define STEPS_PER_ANALOG_UNIT 0.5        // Number of stepper motor steps to take per unit difference between the last "move" and this "move".
#define WATER_LEVEL_THRESHOLD 256       // Analog threshold of water sensor before transitioning to ERROR.
#define TEMPERATURE_THRESHOLD 0       // Minimum temperature threshold for the fan to run.

// Pin definitions. Make sure you're using those defined in io.c!
#define RED_LED_DIGITAL_PIN 28
#define YELLOW_LED_DIGITAL_PIN 26
#define GREEN_LED_DIGITAL_PIN 24
#define BLUE_LED_DIGITAL_PIN 22

#define FAN_MOTOR_DIGITAL_PIN 52

#define STEPPER_MOTOR_DIGITAL_PIN_1 37
#define STEPPER_MOTOR_DIGITAL_PIN_2 35
#define STEPPER_MOTOR_DIGITAL_PIN_3 33
#define STEPPER_MOTOR_DIGITAL_PIN_4 31

#define DHT11_DIGITAL_PIN 50

// ADC channels.
#define STEPPER_POT_ANALOG_CHANNEL 0        // Assuming we want a potentiometer for the vent direction
#define WATER_SENSOR_ANALOG_CHANNEL 1

// External button interrupts.
// If you change these, you must also change the bitmask against EICRA/B and EIMSK below!!
#define START_BUTTON_INTERRUPT_VECTOR INT2_vect // Digital pin 19
#define STOP_BUTTON_INTERRUPT_VECTOR INT3_vect // Digital pin 18
#define RESET_BUTTON_INTERRUPT_VECTOR INT4_vect // Digital pin 2

// Timer interrupts.
#define TIMER_INTERRUPT_VECTOR TIMER1_OVF_vect

// Address definitions.
#define LCD_ADDR 0x27
#define RTC_ADDR 0x68

// These macro values are used to figure out what LED to turn on.
// If we end up using the RGB LED, rewrite set_leds().
uint8_t disabled();
#define DISABLED 0x00
uint8_t idle();
#define IDLE 0x01
uint8_t error();
#define ERROR 0x02
uint8_t running();
#define RUNNING 0x03
uint8_t init();
#define INIT 0x04

uint8_t (*handler[])() = {
	[DISABLED] = disabled,
	[IDLE] = idle,
	[ERROR] = error,
	[RUNNING] = running
};

uint8_t transition_to_disabled();
uint8_t state = DISABLED;

// Interrupt "states"... maybe there's a better way to handle these?
// Handlers must set these back to false once they've handled them
bool start_button_pressed = false;
bool stop_button_pressed = false;
bool reset_button_pressed = false;

// Other global definitions.
// Some are global for debugging purposes, others because I'm lazy
pin_t red_led;
pin_t yellow_led;
pin_t green_led;
pin_t blue_led;
pin_t* leds[] = {&yellow_led, &green_led, &blue_led, &red_led};

pin_t fan_motor;

stepper_t stepper;

FILE lcd;

dht11_t dht11_data;

// Used in tandem with internal clock to determine when to update things.
uint8_t seconds_since_last_lcd_update = 0;
uint8_t seconds_since_last_dht11_poll = 0;

// Contains the last ADC value for the vent direction potentiometer
// that was used to change the vent direction.
uint16_t last_pot_change_value;
uint16_t water_level;

/*** *** *** *** *** *** *** *** *** *** ***
 * Main functions
 *** *** *** *** *** *** *** *** *** *** ***/

void log_message(char* message){
    char time_str[100]; // TODO: "memory safety"
    time_t time = get_time(RTC_ADDR);
    
    tsnprintf(time_str, 100, time);
    printf("[%s] %s\n", time_str, message);
}

void write_lcd(char* message_to_display){
    // BUG: again, DHT11 gets Very Angry if you poll it too quickly.
    // 2.5 sec delay to try and prevent this from being an issue
    _delay_ms(2500);
    dht11_data = dht11_read(DHT11_DIGITAL_PIN);
    
    
   fprintf(&lcd, "%.1fc", message_to_display);
	// Read from DHT11 here

	// Write to LCD here
	// 
	// Chris - It's your choice how you want to handle the error message
	// we display to the LCD in the ERROR state. Maybe we can pass
	// in a bool to this function, or pass in an actual string, etc.
	//
	// Don't forget to update all the uses of write_lcd() if you change
	// the function parameters!
}

// Set the LED we want on (and none of the other LEDs)
// Note this is dependent on macro definitions and the order of `leds`
void set_leds(uint8_t state){
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
	sei();				// Enable interrupts
	uart_init(9600);	// Start UART
	twi_init();			// Start 2x16 LCD TWI
	adc_init();			// Start ADC (do not make any adc conversions before this!!)

	// Set up button interrupts (INT2, INT3, INT4)
    // Interrupt on rising edge only (not on all logical changes)
    EICRA |= 0b11110000;
    EICRB |= 0b00000011;
    // Enable corresponding external pin interrupts
    EIMSK |= 0b00011100;

	// Enable timer interrupts
	TIMSK1 = 0b00000001; 
    start_second_timer();

	// Initialize everything else
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

    // Update LCD for the first time
    write_lcd();

    // Update counters
    last_pot_change_value = adc_read(STEPPER_POT_ANALOG_CHANNEL);

	// Initial transition
	state = transition_to_disabled();
}

int main () {
	setup();
	for (;;) {
		if (handler[state]) {
			state = handler[state]();
			
			lcd_twi_clear(&lcd);
			float f = dht11_data.readTemperature(true);
  			float h = dht11_data.readHumidity();
			lcd_twi_cursor(&lcd, 0, 0);
  			write_lcd("Tempature: ");
  			write_lcd(itoa(f));
  			lcd_twi_cursor(&lcd, 0, 0);
  			write_lcd("Humidity: ");
  			write_lcd(itoa(h));
		} else {
			state = handler[ERROR]();
		}
	}
}

/*** *** *** *** *** *** *** *** *** *** ***
 * Behavioral functions
 *** *** *** *** *** *** *** *** *** *** ***/

void check_if_lcd_needs_update(){
	if(seconds_since_last_lcd_update >= 60){
		if(state == ERROR){
			write_lcd();
		}else{
			write_lcd();
		}
		seconds_since_last_lcd_update = 0;
	}
}

void update_stepper_position(){
	// Has the potentiometer changed by a meaningful amount?
	// If so, trigger a change on the stepper motor
	uint16_t current_pot_value = adc_read(STEPPER_POT_ANALOG_CHANNEL);
	uint16_t abs_difference = abs(current_pot_value - last_pot_change_value);
	//printf("current pot value: %u, abs difference: %u\n", current_pot_value, abs_difference);
	if (abs_difference > VENT_DIRECTION_THRESHOLD){
		int32_t difference = (int32_t)current_pot_value - (int32_t)last_pot_change_value;
		int16_t steps = difference * STEPS_PER_ANALOG_UNIT;
		printf("[STEPPER MOTOR MOVING] - difference: %ld, steps: %d\n", difference, steps);
		stepper_rotate(stepper, steps);
		last_pot_change_value = current_pot_value;
	}
}

// Returns an updated state if needed.
uint8_t check_for_state_change(){
	water_level = adc_read(WATER_SENSOR_ANALOG_CHANNEL); 
	if (water_level <= WATER_LEVEL_THRESHOLD){
		return ERROR;
	}

	// Only poll dht11 if three seconds have passed
	if (seconds_since_last_dht11_poll >= 3){
		seconds_since_last_dht11_poll = 0;

		// Chris - your code here

		if (dht11_data.temp <= TEMPERATURE_THRESHOLD && state == RUNNING){
			return IDLE;
		} else if (dht11_data.temp > TEMPERATURE_THRESHOLD && state == IDLE){
			return RUNNING;
		}
	}

	// No state change needed
	return state;
}

uint8_t transition_to_disabled(){
    log_message("Transition to DISABLED");  // Record transition time to UART
    stop_second_timer();                    // Disable the timer that's updating the LCD screen
    set_leds(DISABLED);                     // Turn on the LED
    write_pin(fan_motor, LOW);              // Turn off the fan motor
    return DISABLED;                       // Set global program state to DISABLED
}

uint8_t transition_to_idle(){
    log_message("Transition to IDLE");	// Record transition time to UART
    start_second_timer();				// Enable the timer that's updating the LCD screen		
    set_leds(IDLE);						// Turn on the LED
    write_pin(fan_motor, LOW);			// Turn off the fan motor
    return IDLE;                       // Set global program state to IDLE
}

uint8_t transition_to_running(){
    log_message("Transition to RUNNING");   // Record transition time to UART
    write_pin(fan_motor, HIGH);             // Turn on the fan motor
    set_leds(RUNNING);                      // Turn on the LED
    return RUNNING;                        // Set global program state to RUNNING
}

uint8_t transition_to_error(){
    log_message("Transition to ERROR"); // Record transition time to UART
    write_lcd("Water low");             // Write message to LCD
    write_pin(fan_motor, LOW);          // Turn off the fan motor
    set_leds(ERROR);                    // Turn on red LED
    return ERROR;                      // Set global program state to ERROR
}

/*** *** *** *** *** *** *** *** *** *** ***
 * For each of these handler functions, simply return the macro for the next state you want to transition to
 * E.G. `return ERROR;` will promptly transition the state to error
 *** *** *** *** *** *** *** *** *** *** ***/

// Facilitates (initial) transition to the disabled state
uint8_t init(){
    log_message("Transition to DISABLED"); 	// Record transition time to UART
    stop_second_timer(); 					// Disable the timer that's updating the LCD screen
    set_leds(DISABLED); 					// Turn on the yellow LED
    write_pin(fan_motor, LOW); 				// Turn off the fan motor
	return DISABLED; 						// Always transition to DISABLED
}

// Yellow LED on
// No monitoring of temp or water
// Start button monitored with ISR
uint8_t disabled() {
	update_stepper_position();

	if(start_button_pressed){
		return transition_to_idle();
	}
	return DISABLED;
}

// Exact time stamp (using real time clock) should record transition times
// Water level should be continuously monitored and state changed to error if level is too low
// GREEN LED should be ON
uint8_t idle() {
	check_if_lcd_needs_update();
	update_stepper_position();
	
	// Interrupt takes priority
	if(stop_button_pressed){
		return transition_to_disabled();
	}

	// Otherwise, check the next state
	uint8_t next_state = check_for_state_change();
	if (next_state == RUNNING){
		return transition_to_running();
	} else if (next_state == ERROR){
		return transition_to_error();
	}

	return IDLE;
}

// Motor should be off and not start regardless of temperature
// A reset button should trigger a change to the IDLE stage if the water level is above the threshold
// Error message should be displayed on LCD
// RED LED should be turned on (all other LEDs turned off)
uint8_t error() {
	check_if_lcd_needs_update();
	update_stepper_position();

	if(reset_button_pressed){
		return transition_to_idle();
	}
	return ERROR;
}

// Fan motor should be on
// System should transition to IDLE as soon as temperature drops below threshold
// System should transition to ERROR state if water becomes too low
// BLUE LED should be turned on (all other LEDs turned off)
uint8_t running() {
	check_if_lcd_needs_update();
	update_stepper_position();
	
	// Interrupt takes priority
	if(stop_button_pressed){
		return INIT;
	}

	// Otherwise, check the next state
	uint8_t next_state = check_for_state_change();
	if (next_state == IDLE){
    	return transition_to_idle();
	} else if (next_state == ERROR){
		return transition_to_error();
	}

	return RUNNING;
}

/*** *** *** *** *** *** *** *** *** *** ***
 * ISRs
 *** *** *** *** *** *** *** *** *** *** ***/

// Only raise the flags for the handlers if necessary.
ISR(START_BUTTON_INTERRUPT_VECTOR){
    if(state == DISABLED){
        start_button_pressed = true;
    }
}

ISR(STOP_BUTTON_INTERRUPT_VECTOR){
    if(state == RUNNING || state == IDLE){
        stop_button_pressed = true;
    }
}

ISR(RESET_BUTTON_INTERRUPT_VECTOR){
    if(state == ERROR){
        reset_button_pressed = true;
    }
}

ISR(TIMER_INTERRUPT_VECTOR){
    stop_second_timer();

    //DEBUG
    //dht11_data = dht11_read(DHT11_DIGITAL_PIN);
    //water_level = adc_read(WATER_SENSOR_ANALOG_CHANNEL);
    //uint16_t stepper_level = adc_read(STEPPER_POT_ANALOG_CHANNEL);
    //printf("T: %.2f, RH: %.2f, water level (0-1024): %u, pot(0-1024): %u", dht11_data.temp, dht11_data.humidity, water_level, stepper_level);

    seconds_since_last_lcd_update++;
    seconds_since_last_dht11_poll++;

    start_second_timer();
}
