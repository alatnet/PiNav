#include <errno.h>
#include <stdio.h>
#include <sys/sysmacros.h>
#include <linux/usb/ch9.h>
#include <usbg/usbg.h>
#include <usbg/function/hid.h>
#include <usbg/function/net.h>
#include <inttypes.h>
#include <stdbool.h>

//#include <syslog.h>

#include <ini.h>

#include "pinav hid.h"

#ifndef DEBUG
#define DEBUG 0
#endif

uint64_t getSerial(void);
bool checkError(int err, char* str);
void debugPrint(char * str);

static int iniHandler(void* user, const char* section, const char* name, const char* value);

//enum XBOX_EMU {
//	NONE = 0,
//	XB360,
//	XBONE
//};

typedef struct {
	int numControllers;
	//int xboxemu;
	bool net;
	bool serial;
} pinavINI;

int main(int argc, char *argv[]) {
	debugPrint("Starting up.\n");

	debugPrint("Reading INI.\n");

	pinavINI config;

	if (ini_parse("/boot/pinav.ini", iniHandler, &config) < 0) {
		debugPrint("Could not open INI file.");
		config.numControllers = 2;
		//config.xboxemu = 0;
		config.net = false;
		config.serial = false;
	}

	usbg_state *s;
	usbg_gadget *g;
	usbg_config *c;
	usbg_function *f_acm, *f_rndis/*, *f_js0, *f_js1*/;

	usbg_function **f_js;

	//allocate hid functions
	if (config.numControllers > 0) f_js = malloc(sizeof(usbg_function*) * config.numControllers);

	int ret = -EINVAL;
	int usbg_ret;

	debugPrint("Initializing structs.\n");
	struct usbg_gadget_attrs g_attrs = {
		.bcdUSB = 0x0200,
		.bDeviceClass = USB_CLASS_PER_INTERFACE,
		.bDeviceSubClass = 0x00,
		.bDeviceProtocol = 0x00,
		.bMaxPacketSize0 = 64, /* Max allowed ep0 packet size */
		.idVendor = 0x1d6b,
		.idProduct = 0x0104,
		.bcdDevice = 0x0001, /* Verson of device */
	};

	debugPrint("Getting Serial.\n");
	char serial[20];
	sprintf(serial, "%016" PRIx64, (uint64_t)getSerial());

	struct usbg_gadget_strs g_strs = {
		.manufacturer = "Pimoroni Ltd.",
		.product = "PiNav",
		.serial = serial
	};

	struct usbg_gadget_os_descs g_os_desc = {
		.use = true,
		.b_vendor_code = 0xCD,
		.qw_sign = "MSFT100"
	};

	char usbconfig[30];
	strcpy(usbconfig, "CDC ");
	char jsConfigNum[8];
	sprintf(jsConfigNum, "%ixHID", config.numControllers);
	strcat(usbconfig, jsConfigNum);
	if (config.serial) strcat(usbconfig, "+ACM");
	if (config.net) strcat(usbconfig, "+RNDIS");

	struct usbg_config_strs c_strs = {
		.configuration = usbconfig //"CDC ACM+RNDIS+2xHID"
	};

	struct usbg_function_os_desc f_os_desc = {
		.compatible_id = "RNDIS",
		.sub_compatible_id = "5162001"
	};

	struct usbg_f_hid_attrs f_js_attrs = {
		.protocol = 0,
		.report_desc = {
			.desc = PiNav_ReportDescriptor,
			.len = sizeof(PiNav_ReportDescriptor),
		},
		.report_length = 8,
		.subclass = 0,
	};

	debugPrint("Beginning gadget creation.\n");

	//Initialize
	debugPrint("Init.\n");
	usbg_ret = usbg_init("/sys/kernel/config", &s);
	if (checkError(usbg_ret, "Error on USB gadget init\n")) goto out1;

	//Create the gadget
	debugPrint("Create.\n");
	usbg_ret = usbg_create_gadget(s, "g1", &g_attrs, &g_strs, &g);
	if (checkError(usbg_ret, "Error on create gadget\n")) goto out2;

	if (config.serial) {
		//Serial
		debugPrint("Serial\n");
		usbg_ret = usbg_create_function(g, USBG_F_ACM, "usb0", NULL, &f_acm);
		if (checkError(usbg_ret, "Error creating ACM function\n")) goto out2;
	}

	if (config.net) {
		//Network
		debugPrint("Network\n");
		usbg_ret = usbg_create_function(g, USBG_F_RNDIS, "usb0", NULL, &f_rndis);
		if (checkError(usbg_ret, "Error creating RNDIS function\n")) goto out2;

		debugPrint("-os desc\n");
		usbg_ret = usbg_set_interf_os_desc(f_rndis, "rndis", &f_os_desc);
		if (checkError(usbg_ret, "Error setting function OS desc\n")) goto out2;

		debugPrint("-to net func\n");
		usbg_f_net * netfunc = usbg_to_net_function(f_rndis);

		{
			//convert serial into mac addresses
			//remove first two digits and add 12 to beginning
			char host[20];
			sprintf(host, "12:%c%c:%c%c:%c%c:%c%c:%c%c", serial[6], serial[7], serial[8], serial[9], serial[10], serial[11], serial[12], serial[13], serial[14], serial[15]);

			//remove first two digits and add 02 to beginning
			char dev[20];
			sprintf(dev, "02:%c%c:%c%c:%c%c:%c%c:%c%c", serial[6], serial[7], serial[8], serial[9], serial[10], serial[11], serial[12], serial[13], serial[14], serial[15]);

			debugPrint("-host addr\n");
			usbg_ret = usbg_f_net_set_host_addr(netfunc, ether_aton(host));
			if (checkError(usbg_ret, "Error setting RNDIS function host addr\n")) goto out2;

			debugPrint("-dev addr\n");
			usbg_ret = usbg_f_net_set_dev_addr(netfunc, ether_aton(dev));
			if (checkError(usbg_ret, "Error setting RNDIS function dev addr\n")) goto out2;
		}
	}

	for (unsigned int i = 0; i < config.numControllers; i++) {
		char debugMsg[15];
		sprintf(debugMsg, "Joystick %i\n", i);
		debugPrint(debugMsg);

		char jsNum[5];
		sprintf(jsNum, "js%i", i);
		usbg_ret = usbg_create_function(g, USBG_F_HID, jsNum, &f_js_attrs, &f_js[i]);

		char errMsg[35];
		sprintf(errMsg, "Error creating HID JS%i function\n", i);
		if (checkError(usbg_ret, errMsg)) goto out2;
	}

	//Configure the gadget
	debugPrint("Config\n");
	usbg_ret = usbg_create_config(g, 1, "PiNav", NULL, &c_strs, &c);
	if (checkError(usbg_ret, "Error creating config\n")) goto out2;

	if (config.net) {
		//Setup the rndis device only first
		debugPrint("RNDIS First\n");
		usbg_ret = usbg_add_config_function(c, "rndis.usb0", f_rndis);
		if (checkError(usbg_ret, "Error adding rndis.usb0\n")) goto out2;
	}

	//Set the os descriptions and configurations
	debugPrint("OS Desc\n");
	usbg_ret = usbg_set_gadget_os_descs(g, &g_os_desc);
	if (checkError(usbg_ret, "Error setting gadget OS desc\n")) goto out2;

	usbg_ret = usbg_set_os_desc_config(g, c);
	if (checkError(usbg_ret, "Error setting gadget OS desc config\n")) goto out2;

	if (config.net) {
		//enable the gadget
		debugPrint("Enable.\n");
		usbg_ret = usbg_enable_gadget(g, DEFAULT_UDC);
		if (checkError(usbg_ret, "Error enabling gadget\n")) goto out2;

		//give it time to install
		debugPrint("Sleep.\n");
		sleep(5);

		//yank it back
		debugPrint("Disable.\n");
		usbg_ret = usbg_disable_gadget(g);
		if (checkError(usbg_ret, "Error disabling gadget\n")) goto out2;
	}

	//sneek in all the extra goodies
	if (config.net) debugPrint("Other Funcs.\n");
	else debugPrint("Functions.\n");

	if (config.serial) {
		usbg_ret = usbg_add_config_function(c, "acm.usb0", f_acm);
		if (checkError(usbg_ret, "Error adding acm.usb0\n")) goto out2;
	}

	for (unsigned int i = 0; i < config.numControllers; i++) {
		char jsNum[10];
		sprintf(jsNum, "hid.js%i", i);
		usbg_ret = usbg_add_config_function(c, jsNum, f_js[i]);

		char errMsg[30];
		sprintf(errMsg, "Error adding %s\n", jsNum);
		if (checkError(usbg_ret, errMsg)) goto out2;
	}

	//Reset bDeviceClass to 0x00
	//This is essential to make it work in Windows 10
	//Basically forces it to use device information
	//in the descriptors versus assuming a particular class.
	if (config.net) {
		debugPrint("Set Class.\n");
		usbg_ret = usbg_set_gadget_attr(g, USBG_B_DEVICE_CLASS, 0x00);
		if (checkError(usbg_ret, "Error setting device class\n")) goto out2;
	}

	//Re-attach the gadget
	if (config.net) debugPrint("Enable\n");
	else debugPrint("Re-enable\n");
	usbg_ret = usbg_enable_gadget(g, DEFAULT_UDC);
	if (checkError(usbg_ret, "Error enabling gadget\n")) goto out2;

	if (config.net) {
		debugPrint("Setting ip.\n");

		system("ifconfig usb0 up 10.0.99.1");
	}

	ret = 0;

out2:
	debugPrint("Cleaning up.\n");
	usbg_cleanup(s);

	free(f_js);

out1:
	debugPrint("Done.\n");
	return ret;
}

uint64_t getSerial(void) {
	static uint64_t serial = 0;

	FILE *filp;
	char buf[512];
	char term;

	filp = fopen("/proc/cpuinfo", "r");

	if (filp != NULL) {
		while (fgets(buf, sizeof(buf), filp) != NULL) {
			if (!strncasecmp("serial\t\t:", buf, 9)) {
				sscanf(buf + 9, "%Lx", &serial);
			}
		}

		fclose(filp);
	}
	return serial;
}

bool checkError(int err, char* str) {
	if (err != USBG_SUCCESS && DEBUG) {
		fprintf(stderr, str);
		fprintf(stderr, "Error: %s : %s\n", usbg_error_name(err), usbg_strerror(err));
		return true;
	}
	return false;
}

void debugPrint(char * str) {
	if (DEBUG) printf("%s", str);
}

static int iniHandler(void* user, const char* section, const char* name, const char* value) {
	pinavINI * config = (pinavINI*)user;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
#define MATCHBOOL(v) strcmp("true", v) == 0 || strcmp("True", value) == 0 || strcmp("TRUE", value) == 0 || strcmp("1", value) == 0

	if (MATCH("pinav", "numControllers")) {
		/*if (strcmp("auto", value) == 0) config->numControllers = -1;
		else {*/
		config->numControllers = atoi(value);
		if (config->numControllers <= 0) config->numControllers = 2;
		/*}*/
	} else if (MATCH("pinav.net", "enabled"))
		if (MATCHBOOL(value)) config->net = true;
		else config->net = false;
	else if (MATCH("pinav.serial", "enabled"))
		if (MATCHBOOL(value)) config->serial = true;
		else config->serial = false;
	/*else if (MATCH("pinav", "xboxemu"))
	if (strcmp("none", value) == 0) config->xboxemu = NONE;
	else if (strcmp("xb360", value) == 0) config->xboxemu = XB360;
	else if (strcmp("xb1", value) == 0) config->xboxemu = XBONE;
	else config->xboxemu = NONE;*/
	else return 0;

	return 1;
}
