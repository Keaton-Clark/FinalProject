# CPE 301 Final Project

- **Team name**: (we need one)
- **Members:** Keaton Clark, Chris Zinser, Jann Arellano, Lloyd Gonzales

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

## Developing with WSL
(see https://learn.microsoft.com/en-us/windows/wsl/connect-usb)

Prerequisites:
- WSL with Linux kernel 5.10.60.1 or later
- Ubuntu 20.04

On Ubuntu, install the AVR development packages:
```bash
sudo apt-get install gcc-avr binutils-avr gdb-avr avr-libc avrdude
```

Then, download the latest release of usbipd-win using the .msi and restart:
https://github.com/dorssel/usbipd-win/releases

After restarting, start WSL and run
```bash
sudo apt install linux-tools-5.4.0-77-generic hwdata
sudo update-alternatives --install /usr/local/bin/usbip usbip /usr/lib/linux-tools/5.4.0-77-generic/usbip 20
```

**With WSL running** and the Arduino plugged in, in an (administrator) Windows command prompt (or PowerShell window), do
```bash
usbipd wsl list
```

Pick the bus ID that corresponds to the Arduino, then run
```bash
usbipd wsl attach --busid <busid>
```

which will prompt for a password to run a sudo command. Then, in Ubuntu, to check that it's actually worked as well as what dev it's been assigned to:
```
dmesg | grep tty
```

Modify the makefile if needed (which shouldn't be necessary). You can run `usbipd wsl detach --busid <busid>` to return the Arduino to Windows, or just physically disconnect the Arduino.

Finally, if you're using VS Code, it's a good idea to use WSL as your remote environment and set the compiler path to avr-gcc **for the current workspace** so it stops complaining about every missing library:
```json
{
    "C_Cpp.default.compilerPath": "/usr/bin/avr-gcc"
}
```