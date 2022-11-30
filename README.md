# CPE 301 Final Project

**Team name**: (we need one)
**Members:** Keaton Clark, Chris Zinser, Jann Arellano, Lloyd Gonzales

## (approximate) timeline
- Dec 3: Modular logic completed; individual sensors working on breadboards
- Dec 7: Integration of all components, either on a "barebones" breadboard with the stuff in the Arduino kit or the final thing we're trying to do
- Dec 10: Completion of other deliverables
- Dec 13: project due date

## TODO
### Physical integrations (wiring and physical locations)
- [ ] Water level sensor
- [ ] LCD screen (to display air temp and humidity)
- [ ] Stepper motor
- [ ] On/off button
- [ ] Vent direction, using either buttons or a potentiometer
- [ ] Temperature + humidity sensor (DHT11)
- [ ] Red, green, yellow, and blue indicating states

### Modular logic
- [ ] Monitoring water level using either interrupts or ADC sampling
	- ADC library is not allowed
- [ ] Vent direction control using the stepper motor, with either buttons or a potentiometer
	- Stepper motor libraries are allowed
- [ ] LCD display reflecting humidity and temperature (with or without integration with the DHT11)
	- LCD libraries are allowed
- [ ] Use of DHT11 to get temperature and humidity readings
	- Libraries are allowed
- [ ] Turning the fan blade on and off
- [ ] State management (to manage the red/green/yellow/blue LEDs)
	- Doesn't (necessarily) need integration with other sensors at this stage
	- [ ] On/off button (to transition between the **disabled** and **idle** states)
	- [ ] "Error" reset button (to transition out of the **error** state into the **idle** state)

### Integrated logic
- [ ] Event reporting, over USB (serial), indicating:
	- any changes to the stepper motor position for the vent
	- the time of state transitions as described in below
- [ ] System should always respond to changes in vent control (whether from buttons or a potentiometer)
- [ ] Humidity and temperature should always be reflected on the LCD screen, at least once per minute
- [ ] Implementation of the state machine:
	- [ ] The "stop" button should always transition to the **disabled** state
	- [ ] The **disabled** state, where:
		- [ ] the yellow LED should be on
		- [ ] temperature/water mointoring should be off
		- [ ] the "start" button should transition to the **idle** state
	- [ ] The **idle** state, where:
		- [ ] the water level should be monitored and transition to **error** if the water level is too low
		- [ ] the green LED should be on
		- [ ] an automatic transition to **running** occurs if the temperature is above a certain threshold
	- [ ] The **error** state, where:
		- [ ] the motor is disabled
		- [ ] the "reset" button should transition to the **idle** state if the water level is above the threshold
		- [ ] the red LED should be turned on
	- [ ] The **running** state, where:
		- [ ] the fan should be on
		- [ ] an automatic transition to **idle** if the temperature drops below a certain threshold
		- [ ] an automatic transition to **error** if the water drops below a certain threshold
		- [ ] the blue LED should be turned on

### Other deliverables
- [ ] Complete schematic
- [ ] Links to all specification sheets for components used
- [ ] Pictures of final system
- [ ] Video of system in operation
- [ ] Design overview and constraints
	- [ ] Operating temperatures
	- [ ] Power requirements
	- [ ] ... some others I can't think of
