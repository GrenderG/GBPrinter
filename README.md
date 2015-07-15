# Game Boy Printer Arduino Library

This arduino library and go client make it possible to connect a [Game Boy Printer](https://en.wikipedia.org/wiki/Game_Boy_Printer) to any computer and print an image of your choice.

[![Firmware demo](http://img.youtube.com/vi/HBTeIwrXzhc/0.jpg)](https://youtu.be/HBTeIwrXzhc)

## Installation and setup

### Arduino
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


### Go Client
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

    ./gbprint -save out.jpg doc/test1.jpg  # Image as sent to the printer
    ./gbprint -save out.jpg -no-rotate doc/test1.jpg  # Don't rotate
    ./gbprint -save out.jpg -no-resize doc/test1.jpg  # Don't resize
    ./gbprint -save out.jpg -no-resize -no-rotate doc/test1.jpg
    ./gbprint -save out.jpg -dither AVERAGE doc/test1.jpg  # Use avg dithering instead of Floyd Steinberg

As expected, you can invoke gbprint with `--help` to get the full list of available options.

Here is a chart with some test images. As you can see, depending on the input, it might be more
interesting to use different dithering algorithms.
![Chart with some test image results. Individual images can be found on the /doc directory](doc/test_chart.jpg)


## Previous work and references
This could not have existed without the awesome reverse engineering work done by
Furrtek, which was the first (to my knowledge) to uncover the GBPrinter protocol
and inner hardware details.

The work from milesburton and davedarko has also been very helpful to translate
the firmware published by Furrtek (originally for an ATtiny85) to the Arduino.

The main addition of this fork is the work that has been done on the interface
Arduino - PC, which now allows to easily transform and transfer an arbitrary
image to the printer. Also, the compilation of sources and information available
on this README should be quite useful to anybody trying to figure out how the
this works.

A list of references I've used throughout the project:

- [Furrtek GBPrinter hardware and package format details (French)](http://furrtek.free.fr/index.php?p=crea&a=gbprinter)
- [Furrtek making of a Game Boy Link cable to USB](http://furrtek.free.fr/index.php?p=crea&a=gbpcable&i=2)
- [Davedarko GitHub repo](http://github.com/davedarko/GBPrinter)
- [Davedarko GBPrinter posts (German)](http://davedarko.com/blog.php?tag=gameboy%20printer)
- [Miles Burton wiki](http://milesburton.com/Gameboy_Printer_with_Arduino)


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
