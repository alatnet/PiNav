# Pi Nav
Converts your Raspberry Pi Zero W into a device that lets windows based computers use Playstation Navigation Controllers natively.

## How to install
There are two ways to install:
* Flashable image
* Installation Script  

Recomended MicroSD Card Size: 2GB

### Flashable
Follow the same instructions as you would flash a normal Raspbian image.  
1. IMPORTANT: Set your password.  I cannot stress this enough so that your Pi doesnt become a vector of attack on your computer.
2. To pair a PS Nav Controller:
* Connect a nav controller to the Pi via usb.  
> sudo navpair  

* Disconnect nav controller.  

>sudo bluetoothctl  
[bluetooth]# discoverable on  
[bluetooth]# agent on  
[bluetooth]# default-agent  

* Press PS button on nav controller.  
> [bluetooth]# connect \<psnav mac\>  
[bluetooth]# trust \<psnav mac\>  
[bluetooth]# quit  

3. Disconnect the Raspberry Pi Zero W from the monitor and keyboard and connect it to a computer via usb cable.
4. Once it has installed all drivers, press the PS button on the PS Nav controller, you can verify that it is working via the game controller settings.

### Script
Current Distro as of time of writing: [Stretch](https://www.raspberrypi.org/downloads/raspbian/)  
You are going to need a UART adapter to connect to the Pi Zero W.  
Pinout for uart connection is as followed when the HDMI and USB connections are pointed down starting from the top left:  
- Pin 1: 5V
- Pin 2: NC
- Pin 3: GND
- Pin 4: TX
- Pin 5: RX  

Note: The baud rate is 115200.

#
1. Flash official Raspbian Light image (you dont need a gui unless you are doing extra stuff.)
2. While microsd card is still on the computer, open up the boot partition and extract the zip file to the root of it or create a pinav folder and download the github files.
4. Edit the config.txt file and add \"enable_uart=1\" and \"dtoverlay=dwc2\" to the end of the file (use something other than notepad as it doesnt handle unix formated new lines).
5. Remove the microsd card from the computer and insert it into the Raspberry Pi Zero W.
6. Connect a UART adapter to the Pi Zero W.
4. Power on the Raspberry Pi Zero W via USB connection to your computer and wait until you get a prompt via the UART adapter.
5. IMPORTANT: Set your password.  I cannot stress this enough so that your Pi doesnt become a vector of attack on your computer.
6. Type in the following:
> echo "dwc2" | sudo tee -a /etc/modules  
echo "libcomposite" | sudo tee -a /etc/modules  

7. Reboot the Pi Zero W.
8. Go into the bin folder within the pinav folder on the boot partition.
9. Type in the following:
> sudo chmod +x ./enableNet.sh
sudo ./enableNet.sh

10. Share your internet connection with the Pi Zero W via the new RNDIS connection.
11. Type in the following when you verified that the Pi Zero W has a network connection:
> sudo apt-get install libconfig9

12. Afterwards, type in the following:
> sudo chmod +x ./install_pinav.sh  
sudo ./install_pinav.sh  

*If this does not work, you can open up the install_pinav.sh file with nano or cat and execute anything that doesnt have a # symbol manually.*  

*If you want ssh over the usb serial connection, type this:*
> sudo systemctl enable getty@ttyGS0.service  

*Note that the connection on a computer is via a com port and the baud rate is 115200.*  

13. Reboot the Pi Zero W.  
14. If you wish, you can disable uart by removing the \"enable_uart=1\" from the config.txt file on the boot partition.  
You wouldnt really need it anymore as PiNav has the ability to enable a serial connection via USB.  

### Connecting a PS Nav Controller
1. To pair a PS Nav Controller:
* Connect a nav controller to the Pi via usb.  
> sudo navpair  

* Disconnect nav controller.  

>sudo bluetoothctl  
[bluetooth]# discoverable on  
[bluetooth]# agent on  
[bluetooth]# default-agent  

* Press PS button on nav controller.  
> [bluetooth]# connect \<psnav mac\>  
[bluetooth]# trust \<psnav mac\>  
[bluetooth]# quit  

2. Disconnect the Raspberry Pi Zero W from the monitor and keyboard and connect it to a computer via usb cable.
3. Once it has installed all drivers, press the PS button on the PS Nav controller, you can verify that it is working via the game controller settings.

# Configuration
PiNav allows for easy configuration via an ini file on the root of the boot partition.  
The file's name is \"pinav.ini\".  