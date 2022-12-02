#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include "io.h"

// define these when we have them
#define RED_LED_PIN = -1
#define YELLOW_LED_PIN = -1
#define GREEN_LED_PIN = -1
#define BLUE_LED_PIN = -1

#define START_BUTTON_PIN = -1
#define STOP_BUTTON_PIN = -1
#define RESET_BUTTON_PIN = -1

/*
The pin change mask register (PCMSKn) dictates which pins
are used for pin interrupts. If the corresponding bit in the
PCICR (pin change interrupt control register) is set, in addition
to enabling the relevant pin number in PCMSKn, then you can use the
- PCINT0_vect 
- PCINT1_vect 
- PCINT2_vect 

vectors as applicable. Obviously, since any PCMSK only contains eight 
bits: 
- PCINT0_vect covers the PCINT7...0 bits in PCMSK0
- PCINT1_vect covers the PCINT15...8 bits in PCMSK1
- PCINT2_vect covers the PCINT23...16 bits in PCMSK2

If any of the corresponding enabled pins change, that will lead to the corresponding
ISR being "called".

The ATMega2560 is notable in that only a few of its digital and analog pins
can be used for interrupts. Each pin's corresponding bit can be found 
in the diagram on:
https://community.element14.com/products/arduino/w/documents/2969/arduino-mega-2560-rev3-pinout-atmega2560-pin-mapping-eagle-files-schematics-and-more

See 
https://arduino.stackexchange.com/questions/79690/assign-an-interrupt-to-any-pin-of-the-atmega328-microcontroller
for more.

I don't think there are macros that translate digital pins to the corresponding 
PCINTn macro directly, unfortunately.
*/

typedef enum program_states{
    DISABLED,
    IDLE,
    ERROR,
    RUNNING,
} program_states_t;

volatile program_states_t program_state = DISABLED;

pin_t red_led_pin = new_pin(RED_LED_PIN);
pin_t yellow_led_pin = new_pin(YELLOW_LED_PIN);
pin_t green_led_pin = new_pin(GREEN_LED_PIN);
pin_t blue_led_pin = new_pin(BLUE_LED_PIN);

pin_t start_button_pin = new_pin(START_BUTTON_PIN);
pin_t stop_button_pin = new_pin(STOP_BUTTON_PIN);
pin_t reset_button_pin = new_pin(RESET_BUTTON_PIN);

// enable interrupts
sei();

// Depending on how we actually want to wire this up and write the code,
// we may or may not need to write multiple ISRs for the buttons. But
// the only requirement is that an ISR is used for the "start" button.
/**
 * @brief ISR for the start button, transitioning from DISABLED to IDLE.
 */
ISR(...){
    if(program_state == DISABLED){
        // Transition to idle state.
        program_state = IDLE;
        toggle_pin(yellow_led_pin);
        toggle_pin(green_led_pin);
    }else{
        // Do nothing.
    }
}

// If we really want to convert these to interrupts as well, we can
// TODO: other state transitions

void transition_to_error(){
    // Turn off motor

    // Display error message on LCD

    // Turn on RED LED

    // Log state transition to UART
}

int main(){
    while(true){
        // if temperature below threshold

        //
    }
}

