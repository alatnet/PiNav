# Pi Nav
Converts your Raspberry Pi Zero W into a device that lets windows based computers use Playstation Navigation Controllers natively.

## Source branch
This branch is for the source files of PiNav.

### Required libraries
1. libusb
2. libusbgx

### Required Tools
1. autoconf (libusbgx)
2. libtool (libusbgx)
3. libconfig-dev (libusbgx)

## How to compile
1. sudo apt-get install libusb-dev autoconf libtool libconfig-dev
2. Compile libusbgx.
3. make