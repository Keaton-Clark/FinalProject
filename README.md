# CPE 301 Final Project

- **Team name**: Exactly One Gallon of Milk
- **Members:** Keaton Clark (Keaton-Clark), Chris Zinser (ThatGuyChrisZ), Jann Arellano (Arellano-Jann), Lloyd Gonzales (lgactna)

## (approximate) timeline
- Dec 3: Modular logic completed; individual sensors working on breadboards
- Dec 7: Integration of all components, either on a "barebones" breadboard with the stuff in the Arduino kit or the final thing we're trying to do
- Dec 10: Completion of other deliverables
- Dec 13: project due date

## TODO
### Physical integrations (wiring and physical locations)
- [x] Water level sensor
- [x] LCD screen (to display air temp and humidity)
- [x] Stepper motor
- [x] On/off button
- [x] Vent direction, using either buttons or a potentiometer
- [x] Temperature + humidity sensor (DHT11)
- [x] Red, green, yellow, and blue indicating states
	- Can be substituted for one RGB LED

### Modular logic
- [x] Monitoring water level using either interrupts or ADC sampling
	- ADC library is not allowed
- [x] Vent direction control using the stepper motor, with either buttons or a potentiometer
	- Stepper motor libraries are allowed
	- **Implementation:** Potentiometer used
- [x] LCD display reflecting humidity and temperature (with or without integration with the DHT11)
	- LCD libraries are allowed
- [x] Use of DHT11 to get temperature and humidity readings
	- Libraries are allowed
- [x] Turning the fan blade on and off
- [x] State management (to manage the red/green/yellow/blue LEDs)
	- Doesn't (necessarily) need integration with other sensors at this stage
	- [x] On/off button (to transition between the **disabled** and **idle** states)
	- [x] "Error" reset button (to transition out of the **error** state into the **idle** state)

### Integrated logic
- [x] Event reporting, over USB (serial), indicating:
	- any changes to the stepper motor position for the vent
	- the time of state transitions as described in below
	- **Implementation:** Uses `printf()` calls in transition functions and stepper motor if statement body, may change
- [x] System should always respond to changes in vent control (whether from buttons or a potentiometer)
- [x] Humidity and temperature should always be reflected on the LCD screen, at least/exactly once per minute
- [x] Implementation of the state machine:
	- [x] The "stop" button should transition to the **disabled** state if in **idle** or **running**, but _not_ when **error**
	- [x] The **disabled** state, where:
		- [x] the yellow LED should be on
		- [x] temperature/water mointoring should be off
		- [x] the "start" button should transition to the **idle** state
	- [x] The **idle** state, where:
		- [x] the water level should be monitored and transition to **error** if the water level is too low
		- [x] the green LED should be on
		- [x] an automatic transition to **running** occurs if the temperature is above a certain threshold
	- [x] The **error** state, where:
		- [x] the motor is disabled
		- [x] the "reset" button should transition to the **idle** state if the water level is above the threshold
		- [x] the red LED should be turned on
	- [x] The **running** state, where:
		- [x] the fan should be on
		- [x] an automatic transition to **idle** if the temperature drops below a certain threshold
		- [x] an automatic transition to **error** if the water drops below a certain threshold
		- [x] the blue LED should be turned on

### Other deliverables
- [x] Complete schematic
	- We're just using Fritzing
- [x] Links to all specification sheets for components used
	- [x] DHT11 (temp/humidity)
	- [x] 2x16 LED
	- [x] Water sensor
	- [x] Stepper motor
	- [x] ULN2003 stepper motor driver board
	- [x] RTC
	- [x] DC motor
- [x] Pictures of final system
- [x] Video of system in operation
- [x] Design overview and constraints
	- [x] Operating temperatures
	- [x] Power requirements
	- [x] Exported Fritzing schematic image
	- [x] ... some others I can't think of
- [x] Rebuild code with dwm-style state management

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
