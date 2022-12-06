/*
State machine prototype. Adapt to new program in main.c.
*/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "io.h"
#include "lcd_twi.h"
#include "steppermotor.h"
#include "ds3231.h"
#include "dht11.h"

// Parameters.
#define VENT_DIRECTION_THRESHOLD 50     // Debouncing threshold, also reduces false movement due to noise
#define WATER_LEVEL_THRESHOLD 256       // Analog threshold of water sensor before transitioning to ERROR.
#define TEMPERATURE_THRESHOLD 70        // Minimum temperature threshold for the fan to run.

// Pin definitions.
#define RED_LED_DIGITAL_PIN 0
#define YELLOW_LED_DIGITAL_PIN 0
#define GREEN_LED_DIGITAL_PIN 0
#define BLUE_LED_DIGITAL_PIN 0

#define FAN_MOTOR_DIGITAL_PIN 0

#define STEPPER_MOTOR_DIGITAL_PIN_1 0
#define STEPPER_MOTOR_DIGITAL_PIN_2 0
#define STEPPER_MOTOR_DIGITAL_PIN_3 0
#define STEPPER_MOTOR_DIGITAL_PIN_4 0

#define DHT11_DIGITAL_PIN 0

// ADC channels.
#define STEPPER_POT_ANALOG_PIN 0        // Assuming we want a potentiometer for the vent direction
#define WATER_SENSOR_ANALOG_PIN 0

// Button interrupt definitions. 
//
// do these macros work? we need at least one button on an ISR (start),
// i don't know if all of them need to be on ISRs. there's also probably
// a better way to be defining these as pins-to-vectors
//
// also, i think we should prefer INTn (external interrupts) to PCINTn (pin interrupts) since:
// - no additional logic needed to figure out which pin
//   threw an interrupt (which is needed in PCINTn, usually)
// - allows us to have the interrupt only occur on rising edge,
//   which eliminates the need to debounce the falling edge
// - no need for software interrupts
#define START_BUTTON_INTERRUPT_VECTOR INT0_vect
#define STOP_BUTTON_INTERRUPT_VECTOR INT0_vect
#define RESET_BUTTON_INTERRUPT_VECTOR INT0_vect
ISR(START_BUTTON_INTERRUPT_VECTOR);
ISR(STOP_BUTTON_INTERRUPT_VECTOR);
ISR(REST_BUTTON_INTERRUPT_VECTOR);
// TODO: write code to initialize these interrupts
//       (change ISCn0:ISCn1 to interrupt on rising edge, set EIMSKn to 1)

#define LCD_ADDR 0x27
#define RTC_ADDR 0x68

// State definitions.
typedef enum {
    DISABLED, 
    IDLE, 
    RUNNING,
    ERROR
} program_state_t;

program_state_t state = DISABLED;

/* Global definitions. */
pin_t red_led;
pin_t yellow_led;
pin_t green_led;
pin_t blue_led;

pin_t fan_motor;

stepper_t stepper;

FILE lcd;

void setup(){
    // Enable interrupts
    sei();

    // Initalize UART, 9600 baud
    uart_init(9600);

    // Initialize TWI for 2x16LCD
    twi_init();

    // Enable ADC
    adc_init();

    // TODO: what are we going to do to have the LCD
    //       update once per minute? are we just going to 
    //       poll the RTC? or are we setting up one of the timers?

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
    )
    
    lcd = lcd_twi_init(LCD_ADDR);

}

void log_message(char* message){
    char time_str[100]; // TODO: "memory safety"
    time_t time = get_time(RTC_ADDR);
    
    tsnprintf(time_str, 100, time);
    printf("[%s] %s\n", time_str, message);
    _delay_ms(1000);
}

int main(){
    // Contains the last ADC value for the vent direction potentiometer
    // that was used to change the vent direction.
    uint16_t last_pot_change_value;
    uint16_t water_level;
    dht11_t dht11_data;

    setup();

    for(;;){
        if(state == ERROR){
            // The only way to transition out of ERROR is to press the
            // reset button. There's nothing else special to do here.
            continue;
        }

        // Has the potentiometer changed by a meaningful amount?
        // If so, trigger a change on the stepper motor
        // TODO: write this

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
    
    // Disable the timer that's updating the LCD screen
    
    // Turn on the LED
    // TODO: what's the best way to "turn off" all the other LEDs? another function?

    // Set global program state to DISABLED
}


// Exact time stamp (using real time clock) should record transition times
// Water level should be continuously monitored and state changed to error if level is too low
// GREEN LED should be ON
void transition_to_idle(){
    // Record transition time to UART
    
    // Enable the timer that's updating the LCD screen
    
    // Turn on the LED

    // Turn off the fan motor

    // Set global program state to IDLE
    
}

// Fan motor should be on
// System should transition to IDLE as soon as temperature drops below threshold
// System should transition to ERROR state if water becomes too low
// BLUE LED should be turned on (all other LEDs turned off)
void transition_to_running(){
    // Record transition time to UART
    
    // Turn on the fan motor
    
    // Turn on the LED

    // Set global program state to IDLE
}

// Motor should be off and not start regardless of temperature
// A reset button should trigger a change to the IDLE stage if the water level is above the threshold
// Error message should be displayed on LCD
// RED LED should be turned on (all other LEDs turned off)
void transition_to_error(){
    // Record transition time to UART
    
    // Write message to LCD
    
    // Turn off the fan motor

    // Turn on red LED

    // Set global program state to ERROR
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
        transition_to_error();
    }
}

// TODO: may be necessary to write an additional timer ISR if we really need
//       the LCD to only update once every minute
//       since this ISR shouldn't be running all the time, we'd also
//       need an additional function to disable said timer