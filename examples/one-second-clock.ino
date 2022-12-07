int counts = 0;

ISR(TIMER1_OVF_vect){
  // Stop timer (disable, set lowest three bits to 000)
  TCCR1B &= 0xF8;

  Serial.print("Tick ");
  Serial.println(counts);

  counts++;

  // Wait for a second
  //TCNT1 = 65536-15625;
  TCNT1 = 65536-15625;
  // Start timer
  TCCR1B |= 0b00000101;
}

void setup() {
  TIMSK1 = 0b00000001; // Enable timer interrupts
  TCCR1B = 0xFD; // Max prescaler, all other "normal" options
  TCNT1 = 65536-15625; // Exact number of ticks for one second interrupts
  

  Serial.begin(9600);
}

void loop() {

}
