#!/bin/bash

#setup usb gadget functionality
#echo "dtoverlay=dwc2" | sudo tee -a /boot/config.txt
#echo "dwc2" | sudo tee -a /etc/modules
#echo "libcomposite" | sudo tee -a /etc/modules

#enable console on serial
#systemctl enable getty@ttyGS0.service

#copy and enable the pinav usb gadget service
cp /boot/pinav/pinav_usb /usr/bin/pinav_usb
chmod +x /usr/bin/pinav_usb
cp /boot/pinav/pinav_usb.service /etc/systemd/system/pinav_usb.service
systemctl enable pinav_usb

#copy the bridge daemon
cp /boot/pinav/99-pinav_bridge_daemon.rules /etc/udev/rules.d
cp /boot/pinav/pinav_bridge_daemon@.service /etc/systemd/system
cp /boot/pinav/pinav_bridge_daemon /usr/bin
chmod +x /usr/bin/pinav_bridge_daemon

#copy the navpair program
cp /boot/pinav/navpair /usr/bin
chmod +x /usr/bin/navpair