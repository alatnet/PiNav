# Pi Nav
Converts your Raspberry Pi Zero W into a device that lets windows based computers use Playstation Navigation Controllers natively.

## How to install
There are two ways to install:
* Flashable image
* Installation Script  

Recomended MicroSD Card Size: 2GB

### Flashable
Follow the same instructions as you would flash a normal Raspbian image.

### Script
Current Distro as of time of writing: [Stretch](https://www.raspberrypi.org/downloads/raspbian/)  
1. Flash official Raspbian Light image (you dont need a gui unless you are doing extra stuff.)
2. While microsd card is still on the computer, open up the boot partition and extract the zip file to the root of it or create a pinav folder and download the github files.
3. Remove the microsd card from the computer and insert it into the Raspberry Pi Zero W. (Note, have your pi connected to a monitor with a keyboard, we will not be connecting to the computer at this time)
4. Power on the Raspberry Pi Zero W and wait until you get a prompt.
5. IMPORTANT: Set your password.  I cannot stress this enough so that your Pi doesnt become a vector of attack on your computer.
6. Type in the following:
> echo "dtoverlay=dwc2" > sudo tee -a /boot/config.txt  
echo "dwc2" | sudo tee -a /etc/modules  
echo "libcomposite" | sudo tee -a /etc/modules  

7. Afterwards, type in the following:
> sudo chmod +x ./install_pinav.sh  
sudo ./install_pinav.sh  

*if this does not work, you can open up the install_pinav.sh file with nano or cat and execute anything that doesnt have a # symbol manually.*  
8. Reboot your device by unplugging and replugging the Pi.  
*if you want ssh over the serial connection, type this:*
> sudo systemctl enable getty@ttyGS0.service  

*Note that the connection on a computer is via a com port and the baud rate is 115200.*

9. To pair a PS Nav Controller:
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

10. Disconnect the Raspberry Pi Zero W from the monitor and keyboard and connect it to a computer via usb cable.
11. Once it has installed all drivers, press the PS button on the PS Nav controller, you can verify that it is working via the game controller settings.