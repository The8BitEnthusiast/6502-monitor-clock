# Ben Eater 6502 Monitor Augmented with Clock Generator

![Splash](https://github.com/The8BitEnthusiast/6502-monitor-clock/blob/main/images/overview.png?raw=true)

## Overview

This is an extended version of Ben Eater's [arduino based 6502 monitor](https://eater.net/downloads/6502-monitor.ino). It incorporates clock generation with the same features as his [clock module](https://eater.net/8bit/clock), i.e. with variable clock frequency and manual stepping. It also adds the ability to generate X number of pulses, which can be handy for Ben's VGA project, and to define breakpoints.

## Build Instructions

The only pre-requisite for compiling the sketch is to install this [Command Parser Library](https://www.arduinolibraries.info/libraries/command-parser) through the Arduino IDE's library manager. It is then normal verify/compile/upload procedure beyond that point.

## Quick Start

Connect the arduino to the 6502 CPU just like you would with Ben's monitor sketch. The only difference is the clock output, which is on pin 13 so that you can see the clock blinking on the arduino's built-in LED. Connect pin 13 to the clock input of the CPU, making sure you disconnect any other clock source (e.g. oscillator) to avoid conflict.

Set up the Arduino IDE's serial terminal for 115,200 baud speed. After uploading the sketch, you will be greeted with usage instructions, which can be brought back to the screen at any time with the 'h' command.

By default, the clock and monitor are off. The default clock frequency is set to 1 hz.

### Enable the Monitor and Start the Clock

Enter the following command in the serial terminal to launch the monitor and clock:

```
m <enter>
c <enter>
```

If you wish to pause the clock, enter 'c' again, or just hit "enter" if 'c' was the last command.

### Change the Frequency of the Clock

You can change the frequency of the clock while it is running or stopped with the following command, which takes the frequency as its only parameter:

```
f 1 <enter>
```

This sets the frequency to 1 hz. Adjust to your liking!

### Stop the Clock

You can stop the clock by re-entering the 'c' command.

### Step the Clock

The 's' command is used to step the clock. You must supply the number of cycles desired. As a hint, since the 'enter' key repeats the last command, you can single step the clock repeatly using this sequence:

```
s 1 <enter>
<enter>
<enter>
...
```

### Define a Breakpoint

The 'b' command defines a breakpoint. The only parameter is the address at which the clock will be stopped. The format of the address is HEX.

```
b ffc0 <enter>
c <enter>
```

This last command sequence sets a breakpoint at address "FFC0" and toggles the clock (will run if it was stopped). The clock will keep going until the address hits "FFC0"


