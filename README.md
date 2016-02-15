# gp_bracket

Bracket capture sample program with libgphoto2.

## Description

This is a sample program to show how to deal with libgphoto2 APIs.
The program takes 3 sequential bracket pictures with different shutter speed 3 times each 20 second.

NOTE that this program may need modification depend on your camera, lens and version of libgphoto2.

For more details of gphoto2 and libgphoto2, refer to.

[http://gphoto.sourceforge.net/](http://gphoto.sourceforge.net/)

## Requirement

- Linux PC or Linux board. (I checked with Beagle Bone Green + pre installed Debian)

- libgphoto2 (I checked with version 2.5.7)

- gcc (I checked with (Debian 4.6.3-14) 4.6.3)

- DSLR supported by libgphoto2. (I checked with EOS 5D2 & 5D3)

## Usage

- Connect between Linux and your camera with USB cable.
- Power on your camera.
- lsusb to see your camera is surely connected to the computer.
- Open command prompt.

- ./bracket

- Then capture sequence will start.

## Build

- Modify Makefile according to your libgphoto2 inludes and libs environment.

- make

## Author

delphinus1024

## License

[MIT](https://raw.githubusercontent.com/delphinus1024/opencv30hdr/master/LICENSE.txt)
