// Write a stepper motor function, it's super easy. I think one pin is for vcc, one for ground, one is direction and the last you just pulse for every tick you want to move
// https://components101.com/motors/28byj-48-stepper-motor
// https://create.arduino.cc/projecthub/debanshudas23/getting-started-with-stepper-motor-28byj-48-3de8c9
// https://github.com/arduino-libraries/Stepper

// basic stepper motor functionality. fix so that it doesn't use pinmode etc.
// wait jk we can use pinmodes ig
// #define A 2
// #define B 3
// #define C 4
// #define D 5
 
// #define NUMBER_OF_STEPS_PER_REV 512

// hahahahahah not sure if any of this is correct bc i cant test itttttt :)))
motor_t init_motor(pin_t *pins){
    for (int i = 0; i < 5; ++i){
        pin_mode(pins[i], OUTPUT)
    }
    return (motor_t){
        .pins = pins
    };
}

// something feels wrong here so i'm gonna stop.
void one_step_turn(motor_t motor){
    for (int i = 0; i < 4; ++i){
        // write_pin(motor.pins[i],)
    } 
}
void write(int a,int b,int c,int d){
write_pin(A,a);
write_pin(B,b);
write_pin(C,c);
write_pin(D,d);
}

// void setup(){
// pinMode(A,OUTPUT);
// pinMode(B,OUTPUT);
// pinMode(C,OUTPUT);
// pinMode(D,OUTPUT);
// }
void onestep(){
write(1,0,0,0);
delay(5);
write(1,1,0,0);
delay(5);
write(0,1,0,0);
delay(5);
write(0,1,1,0);
delay(5);
write(0,0,1,0);
delay(5);
write(0,0,1,1);
delay(5);
write(0,0,0,1);
delay(5);
write(1,0,0,1);
delay(5);
}



void loop(){
int i;
i=0;
while(i<NUMBER_OF_STEPS_PER_REV){
onestep();
i++;
}
}