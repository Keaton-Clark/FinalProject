// Write a stepper motor function, it's super easy. I think one pin is for vcc, one for ground, one is direction and the last you just pulse for every tick you want to move
// https://components101.com/motors/28byj-48-stepper-motor
// https://create.arduino.cc/projecthub/debanshudas23/getting-started-with-stepper-motor-28byj-48-3de8c9
// https://github.com/arduino-libraries/Stepper
// https://www.arduino.cc/reference/en/libraries/stepper/
// #include <Stepper.h> // https://docs.arduino.cc/learn/electronics/stepper-motors#unipolar-stepper-circuit-and-schematic

#define stepsPerRevolution 200;

Stepper init_motor(pin_t *pins){
    Stepper myStepper(stepsPerRevolution, pins[0], pins[1], pins[2], pins[3]);
    myStepper.setSpeed(15);
    return myStepper;
}

// Stepper myStepper(stepsPerRevolution, 8, 9, 10, 11);
// myStepper.setSpeed(15);
void clockwise_turn(motor_t motor){
    myStepper.step(stepsPerRevolution);
}

void counter_clockwise_turn(motor_t motor){
    myStepper.step(-stepsPerRevolution);
}







// basically the stuff below is register level or smth
// // hahahahahah not sure if any of this is correct bc i cant test itttttt :)))
// motor_t init_motor(pin_t *pins){
//     for (int i = 0; i < 4; ++i){ 
//         pin_mode(pins[i], OUTPUT)
//     }
//     return (motor_t){
//         .pins = pins
//     };
// }

// // this function turns the motor once
// // there must be a brilliant way to do this with bitshifting but i have no idea how
// void one_step_turn(motor_t motor){
//     write(motor, 1, 0, 0, 0); // does this need to be write(&motor,1,0,0,0) ?
//     _delay_ms(5);
//     write(motor, 1, 1, 0, 0);
//     _delay_ms(5);
//     write(motor, 0, 1, 0, 0);
//     _delay_ms(5);
//     write(motor, 0, 1, 1, 0);
//     _delay_ms(5);
//     write(motor, 0, 0, 1, 0);
//     _delay_ms(5);
//     write(motor, 0, 0, 1, 1);
//     _delay_ms(5);
//     write(motor, 0, 0, 0, 1);
//     _delay_ms(5);
//     write(motor, 1, 0, 0, 1);
//     _delay_ms(5);
// }
// // this function updates all the pins of the motor at once to the parameter given
// void write(motor_t motor, int a,int b,int c,int d){
//     write_pin(motor.pins[0],a);
//     write_pin(motor.pins[1],b);
//     write_pin(motor.pins[2],c);
//     write_pin(motor.pins[3],d);
// }
// // basic stepper motor functionality. fix so that it doesn't use pinmode etc.
// // wait jk we can use pinmodes ig
// // #define A 2
// // #define B 3
// // #define C 4
// // #define D 5
 
// // #define NUMBER_OF_STEPS_PER_REV 512

// // void write(int a,int b,int c,int d){
// // digitalWrite(A,a);
// // digitalWrite(B,b);
// // digitalWrite(C,c);
// // digitalWrite(D,d);
// // }

// // void setup(){
// // pinMode(A,OUTPUT);
// // pinMode(B,OUTPUT);
// // pinMode(C,OUTPUT);
// // pinMode(D,OUTPUT);
// // }

// // this turns the motor one whole turn by following the stepper motor schematics
// // void onestep(){
// // write(1,0,0,0);
// // delay(5);
// // write(1,1,0,0);
// // delay(5);
// // write(0,1,0,0);
// // delay(5);
// // write(0,1,1,0);
// // delay(5);
// // write(0,0,1,0);
// // delay(5);
// // write(0,0,1,1);
// // delay(5);
// // write(0,0,0,1);
// // delay(5);
// // write(1,0,0,1);
// // delay(5);
// // }


// // this turns the motor NUMBER_OF_STEPS_PER_REV times
// // void loop(){
// // int i;
// // i=0;
// // while(i<NUMBER_OF_STEPS_PER_REV){
// // onestep();
// // i++;
// // }
// // }