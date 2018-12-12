#!/bin/bash

#disable console on serial
#systemctl disable getty@ttyGS0.service

#disable and remove the pinav usb gadget service
systemctl disable pinav_usb
rm /etc/systemd/system/pinav_usb.service
rm /usr/bin/pinav_usb

#remove the bridge daemon
rm /boot/pinav/99-pinav_bridge_daemon.rules
rm /boot/pinav/pinav_bridge_daemon@.service
rm /usr/bin/pinav_bridge_daemon

#remove the navpair program
rm /usr/bin/navpair