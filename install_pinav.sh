#!/bin/bash

#setup usb gadget functionality
#echo "dtoverlay=dwc2" | sudo tee -a /boot/config.txt
#echo "dwc2" | sudo tee -a /etc/modules
#echo "libcomposite" | sudo tee -a /etc/modules

#enable console on serial
#systemctl enable getty@ttyGS0.service

#copy libusbgx libraries
cp libusbgx/libusbgx.a /usr/local/lib
cp libusbgx/libusbgx.la /usr/local/lib
cp libusbgx/libusbgx.lai /usr/local/lib
cp libusbgx/libusbgx.so /usr/local/lib
cp libusbgx/libusbgx.so.2 /usr/local/lib
cp libusbgx/libusbgx.so.2.0.0 /usr/local/lib
ldconfig

#copy executables
cp bin/navpair /usr/bin
cp bin/sixpair /usr/bin
cp bin/pinav_usb /usr/bin
cp bin/pinav_bridge_daemon /usr/bin

#make the executables... well... executable
chmod +x /usr/bin/navpair
chmod +x /usr/bin/sixpair
chmod +x /usr/bin/pinav_usb
chmod +x /usr/bin/pinav_bridge_daemon

#copy misc files
cp misc/pinav.ini /boot
cp misc/pinav_usb.service /etc/systemd/system
cp misc/pinav_bridge_daemon@.service /etc/systemd/system
cp misc/99-pinav_bridge_daemon.rules /etc/udev/rules.d

#enable pinav service
systemctl enable pinav_usb