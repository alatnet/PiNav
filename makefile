.PHONY : all

#Generic compiling information
##C Compiler
CC=gcc
CFLAGS=-std=gnu11
CFLAGS_BUILD=-std=gnu11

##C++ Compiler
CXX=g++
CXXFLAGS=-std=gnu++1z
CXXFLAGS_BUILD=-std=gnu++1z

##Linker
LBITS=$(shell getconf LONG_BIT)
LDFLAGS=-Ur

#compile all the programs
all : pinav_usb pinav_bridge_daemon navpair sixpair

#compile the programs
navpair : navpair.c
	$(CC) $(CFLAGS_BUILD) navpair.c -o navpair -lusb -Ofast

sixpair : sixpair.c
	$(CC) $(CFLAGS_BUILD) sixpair.c -o sixpair -lusb -Ofast

pinav_bridge_daemon : pinav_bridge_daemon.c
	$(CXX) $(CXXFLAGS_BUILD) pinav_bridge_daemon.c -o pinav_bridge_daemon -Ofast

pinav_usb : pinav_usb.c
	$(CC) $(CFLAGS_BUILD) pinav_usb.c submodules/inih/ini.c -o pinav_usb -lusbgx -Isubmodules/inih -Ofast -Isubmodules/libusbgx/include -Lsubmodules/libusbgx/src/.libs 

pinav_usb_debug : pinav_usb.c
	$(CC) $(CFLAGS_BUILD) pinav_usb.c submodules/inih/ini.c -o pinav_usb -lusbgx -Isubmodules/inih -Isubmodules/libusbgx/include -Lsubmodules/libusbgx/src/.libs -DDEBUG=1 -g

#install the programs
install : install-libusbgx
	#copy navpair
	cp navpair /usr/bin
	chmod +x /usr/bin/navpair
	#copy sixpair
	cp sixpair /usr/bin
	chmod +x /usr/bin/sixpair
	#copy pinav_usb
	cp pinav_usb /usr/bin
	chmod +x /usr/bin/pinav_usb
	#copy and enable pinav_usb service
	cp misc/pinav_usb.service /etc/systemd/system
	systemctl enable pinav_usb
	#copy pinav_bridge daemon and services
	cp pinav_bridge_daemon /usr/bin
	chmod +x /usr/bin/pinav_bridge_daemon
	cp misc/pinav_bridge_daemon@.service /etc/systemd/system
	cp misc/99-pinav_bridge_daemon.rules /etc/udev/rules.d

#uninstall the programs
uninstall : uninstall-libusbgx
	systemctl disable pinav_usb
	rm /usr/bin/navpair /usr/bin/sixpair /usr/bin/pinav_bridge_daemon /usr/bin/pinav_usb /etc/systemd/system/pinav_usb.service /etc/systemd/system/pinav_bridge_daemon@.service /etc/udev/rules.d/99-pinav_bridge_daemon.rules
	
#install libusbgx libraries
install-libusbgx:
	cp submodules/libusbgx/src/.libs/libusbgx.a /usr/local/lib
	cp submodules/libusbgx/src/.libs/libusbgx.la /usr/local/lib
	cp submodules/libusbgx/src/.libs/libusbgx.lai /usr/local/lib
	cp submodules/libusbgx/src/.libs/libusbgx.so /usr/local/lib
	cp submodules/libusbgx/src/.libs/libusbgx.so.2 /usr/local/lib
	cp submodules/libusbgx/src/.libs/libusbgx.so.2.0.0 /usr/local/lib
	ldconfig

#uninstall libusbgx libraries
uninstall-libusbgx :
	rm /usr/local/lib/libusbgx.a /usr/local/lib/libusbgx.la /usr/local/lib/libusbgx.lai /usr/local/lib/libusbgx.so /usr/local/lib/libusbgx.so.2 /usr/local/lib/libusbgx.so.2.0.0
	ldconfig

install_navpair :
	#copy navpair
	cp navpair /usr/bin
	chmod +x /usr/bin/navpair
install_sixpair :
	#copy sixpair
	cp sixpair /usr/bin
	chmod +x /usr/bin/sixpair
install_pinav_usb :
	#copy pinav_usb
	cp pinav_usb /usr/bin
	chmod +x /usr/bin/pinav_usb
	#copy and enable pinav_usb service
	cp misc/pinav_usb.service /etc/systemd/system
	systemctl enable pinav_usb
install_bridge :
	#copy pinav_bridge daemon and services
	cp pinav_bridge_daemon /usr/bin
	chmod +x /usr/bin/pinav_bridge_daemon
	cp misc/pinav_bridge_daemon@.service /etc/systemd/system
	cp misc/99-pinav_bridge_daemon.rules /etc/udev/rules.d

uninstall_navpair :
	rm /usr/bin/navpair
uninstall_sixpair :
	rm /usr/bin/sixpair
uninstall_pinav_usb :
	systemctl disable pinav_usb
	rm /usr/bin/pinav_usb /etc/systemd/system/pinav_usb.service
uninstall_bridge :
	rm /usr/bin/pinav_bridge_daemon /etc/systemd/system/pinav_bridge_daemon@.service /etc/udev/rules.d/99-pinav_bridge_daemon.rules

#clean up the files
clean :
	rm navpair sixpair pinav_bridge_daemon pinav_usb