# Game Boy Printer Arduino Library

This arduino library and go client make it possible to connect a [Game Boy Printer](https://en.wikipedia.org/wiki/Game_Boy_Printer) to any computer and print an image of your choice.

*A video should go in here*

## Installation and setup

To upload the firmware to your arduino device open `arduino/src/src.ino` with
the Arduino IDE and follow the usual upload process. No extra steps are needed.

The Game Boy Printer GameLink cable needs to be connected to the Arduino as
follows:

- Serial Clock > 8
- Serial In > 9
- Serial Out > 10
- Ground > Ground

The GBPrinter port pinout (female) is the following:

     _____
    /5 3 1\
    |6 4 2|
     -----

     1. VCC (5V)
     2. Serial Out
     3. Serial In
     4. Serial Data
     5. Serial Clock
     6. Ground

TODO: Expand on the connection, explain here that you don't need any extra resistors
and post images of my setup and explain why the wormlight might be a good idea
for anyone that wants to do this a little bit seriously.

There is a test suite for the firmware, that mocks the Arduino Library. To build
it you will need `scons`.

    git submodule update --init --recursive  # This will download the mock libs
    cd arduino
    scons  # This builds and runs the tests

TODO: The test suite is a bit broken at the moment, but it was quite useful to
test concepts and make sure that things like state transitions or the circle
buffer worked properly.

Building the client in go (>=1.3) is simple:

    go get github.com/nfnt/resize
    go get github.com/tarm/serial
    go build gbprint.go

As an example, let's assume that the Arduino is listening to serial port `/dev/tty.usb1`
and we want to print `awesome_cat.jpg`:

    ./gbprint -serial /dev/tty.usb1 awesome_cat.jpg

Wait for a bit and soon enough your Game Boy Printer will be onto it.

If you just want to test the client for a bit, see what sort of output you can
expect from your printer, there is also the option to output the resulting image
to a file (instead of printing):

    ./gbprint -save out.jpg awesome_cat.jpg  # Image as sent to the printer
    ./gbprint -save out.jpg -no-rotate awesome_cat.jpg  # Don't rotate
    ./gbprint -save out.jpg -no-resize awesome_cat.jpg  # Don't resize
    ./gbprint -save out.jpg -no-resize -no-rotate awesome_cat.jpg
    ./gbprint -save out.jpg -dither AVERAGE awesome_cat.jpg  # Use avg dithering instead of Floyd Steinberg

As expected, you can explore the client options by invoking it with `--help`.

TODO: Post some results of the commands mentioned in the examples above


## Previous work and references
This could not have existed without the awesome reverse engineering work done by
Furrtek, which was the first (to my knowledge) to uncover the GBPrinter protocol
and inner hardware details.

The work from millburton and davedarko has also been very helpful to translate
the firmware published by Furrtek (originally for an ATtiny85) to the Arduino.

The main addition of this fork is the work that has been done on the interface
Arduino - PC, which now allows to easily transform and transfer an arbitrary
image to the printer. Also, the compilation of sources and information available
on this README should be quite useful for anybody trying to figure out how the
protocol and hardware work.

A list of references I've used throughout the project, from most to least employed:

- [Furrtek making of a Game Boy Link cable to USB](http://furrtek.free.fr/index.php?p=crea&a=gbpcable&i=2)
- [Furrtek GBPrinter Hardware and package format details](http://furrtek.free.fr/index.php?p=crea&a=gbprinter)

TODO: I should put some millburton stuff or something else, but those 2 articles are the main source.


## Firmware

### Communication Protocol

### States

### Error codes

### Connection & Cable details


## Client

### Communication Protocol

### Image processing

### Image serialization
This may be more appropriate to add under firmware?

### Options
Talk a bit about the options supported and img formats
