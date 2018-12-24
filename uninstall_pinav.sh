#!/bin/bash

#disable console on serial
#systemctl disable getty@ttyGS0.service

#disable the pinav service
systemctl disable pinav_usb

#remove the files
rm /usr/bin/navpair /usr/bin/sixpair /usr/bin/pinav_usb /usr/bin/pinav_bridge_daemon /etc/systemd/system/pinav_usb.service /etc/systemd/system/pinav_bridge_daemon@.service /etc/udev/rules.d/99-pinav_bridge_daemon.rules /usr/local/lib/libusbgx.a /usr/local/lib/libusbgx.la /usr/local/lib/libusbgx.lai /usr/local/lib/libusbgx.so /usr/local/lib/libusbgx.so.2 /usr/local/lib/libusbgx.so.2.0.0
ldconfig