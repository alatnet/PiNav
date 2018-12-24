#!/bin/bash

#Parts of code used are from https://gist.github.com/Gadgetoid/c52ee2e04f1cd1c0854c3e77360011e2
#Props to Gadgetoid for getting rndis working on windows with multiple composited usb devices.

if [ ! -d /sys/kernel/config/usb_gadget ]; then
    modprobe libcomposite
fi

if [ -d /sys/kernel/config/usb_gadget/g1 ]; then
    exit 0
fi

ID_VENDOR="0x1d6b"
ID_PRODUCT="0x0104"

SERIAL="$(grep Serial /proc/cpuinfo | sed 's/Serial\s*: 0000\(\w*\)/\1/')"
MAC="$(echo ${SERIAL} | sed 's/\(\w\w\)/:\1/g' | cut -b 2-)"
MAC_HOST="12$(echo ${MAC} | cut -b 3-)"
MAC_DEV="02$(echo ${MAC} | cut -b 3-)"

cd /sys/kernel/config/usb_gadget/

mkdir g1
cd g1

echo "0x0200" > bcdUSB
echo "0x02" > bDeviceClass
echo "0x00" > bDeviceSubClass
echo "0x0001" > bcdDevice
echo $ID_VENDOR > idVendor
echo $ID_PRODUCT > idProduct

# Windows extensions to force config

echo "1" > os_desc/use
echo "0xcd" > os_desc/b_vendor_code
echo "MSFT100" > os_desc/qw_sign

mkdir strings/0x409
echo "$SERIAL" > strings/0x409/serialnumber
echo "Pimoroni Ltd." > strings/0x409/manufacturer
echo "PiNav Network" > strings/0x409/product

# Config #1 for OSX / Linux

mkdir configs/c.1
mkdir configs/c.1/strings/0x409
echo "CDC RNDIS" > configs/c.1/strings/0x409/configuration

mkdir functions/rndis.usb0 # Flippin' Windows

#network
echo "RNDIS" > functions/rndis.usb0/os_desc/interface.rndis/compatible_id
echo "5162001" > functions/rndis.usb0/os_desc/interface.rndis/sub_compatible_id

echo $MAC_HOST > functions/rndis.usb0/host_addr
echo $MAC_DEV > functions/rndis.usb0/dev_addr

# Set up the rndis device only first

ln -s functions/rndis.usb0 configs/c.1 #network

# Tell Windows to use config #2

ln -s configs/c.1 os_desc

# Show Windows the RNDIS device with
# bDeviceClass 0x02
# bDeviceSubClass 0x02

ls /sys/class/udc > UDC

# Give it time to install

sleep 5

# Yank it back

echo "" > UDC

# Reset bDeviceClass to 0x00
# This is essential to make it work in Windows 10
# Basically forces it to use device information
# in the descriptors versus assuming a particular class.

echo "0x00" > bDeviceClass

# Re-attach the gadget

ls /sys/class/udc > UDC

# BOOM!

ifconfig usb0 up 10.0.99.1
