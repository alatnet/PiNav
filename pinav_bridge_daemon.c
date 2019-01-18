#define _DEFAULT_SOURCE

//needed for umask
#include <sys/types.h>
#include <sys/stat.h>

//system error numbers 
#include <errno.h>

//file control options
#include <fcntl.h>

//standard symbolic constants and types
#include <unistd.h>

//standard lib
#include <stdlib.h> //standard library definitions
#include <stdio.h> //standard buffered input/output
#include <stdint.h> //int types

//c string manipulation
#include <string.h>

//for sched_yield
#include <sched.h>

//system log
#include <syslog.h>

//joystick input
#include <linux/input.h>
#include <linux/joystick.h>

//for basename function
#include <libgen.h>

//#include <sys/ioctl.h>
//#include <sys/time.h>

typedef struct {
	uint8_t X : 1;
	uint8_t O : 1;
	uint8_t L1 : 1;
	uint8_t L2 : 1;
	uint8_t L3 : 1;
	uint8_t PS : 1;

	uint8_t DPAD : 4;

	int16_t JX;
	int16_t JY;

	int16_t Trigger;
} PSNavReport;

enum DPAD {
	CENTER = 0x08,
	UP = 0x00,
	UP_RIGHT = 0x01,
	RIGHT = 0x02,
	DOWN_RIGHT = 0x03,
	DOWN = 0x04,
	DOWN_LEFT = 0x05,
	LEFT = 0x06,
	UP_LEFT = 0x07
};

struct axis_state {
	int16_t x, y;
};

//function prototypes
int read_event(int fd, struct js_event *event);
size_t get_axis_state(struct js_event *event, struct axis_state axes[3]);

//main function
int main(int argc, char **argv) {
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//DAEMON STUFF
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	/* Our process ID and Session ID */
	pid_t pid, sid;

	/* Fork off the parent process */
	pid = fork();
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}
	/* If we got a good PID, then
	   we can exit the parent process. */
	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}

	/* Change the file mode mask */
	umask(0);

	/* Open any logs here */
	{
		char log[256];
		sprintf(log, "pinav_bridge_daemon on /%s", argv[1]);
		openlog(log, LOG_NOWAIT | LOG_PID, LOG_USER);

		sprintf(log, "Successfully started pinav_bridge_daemon on /%s", argv[1]);
		syslog(LOG_NOTICE, log);
	}

	/* Create a new SID for the child process */
	sid = setsid();
	if (sid < 0) {
		/* Log the failure */
		syslog(LOG_ERR, "Could not generate session ID for child process");
		exit(EXIT_FAILURE);
	}

	/* Change the current working directory */
	if ((chdir("/")) < 0) {
		/* Log the failure */
		syslog(LOG_ERR, "Could not change working directory to /");
		exit(EXIT_FAILURE);
	}

	/* Close out the standard file descriptors */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//PiNav Bridge Stuff
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	int hidout, js0in;
	struct js_event event;
	struct axis_state axes[3] = {0};
	size_t axis;

	{ //open the devices
		//get which joystick connected
		int joystickNum;
		sscanf(basename(argv[1]), "js%d", &joystickNum);

		//set the hidout and jsin path
		char hidoutPath[25];
		char jsinPath[25];
		sprintf(hidoutPath, "/dev/hidg%i", joystickNum);
		sprintf(jsinPath, "/%s", argv[1]);

		//open the hid device
		if ((hidout = open(hidoutPath, O_WRONLY)) < 0) {
			syslog(LOG_ERR, "Could not open %s", hidoutPath);
			exit(EXIT_FAILURE);
		}

		//open the joystick device
		if ((js0in = open(jsinPath, O_RDONLY)) < 0) {
			syslog(LOG_ERR, "Could not open %s", jsinPath);
			close(hidout);
			exit(EXIT_FAILURE);
		}
	}

	PSNavReport hidoutReport = {0};

	//zero out everything
	hidoutReport.Trigger = -32766;
	hidoutReport.DPAD = CENTER;
	write(hidout, &hidoutReport, sizeof(PSNavReport));

	struct DPAD_S {
		uint8_t left, right, up, down;
	} dpad_state = {0};

	//BRIDGE THE DATA!
	while (read_event(js0in, &event) == 0) { //read the joystick input
		//depending on the event type
		switch (event.type) {
		case JS_EVENT_BUTTON: //if it's a button assign the button to the correct button output
			switch (event.number) {
			case 4: //L1
				hidoutReport.L1 = event.value;
				break;
			case 6: //L2/Trigger
				hidoutReport.L2 = event.value;
				break;
			case 11: //L3
				hidoutReport.L3 = event.value;
				break;
			case 0: //X
				hidoutReport.X = event.value;
				break;
			case 1: //O
				hidoutReport.O = event.value;
				break;
			case 10: //PS
				hidoutReport.PS = event.value;
				break;
			case 13: //dpad, requires conversion
				dpad_state.up = event.value;
				break;
			case 14:
				dpad_state.down = event.value;
				break;
			case 15:
				dpad_state.left = event.value;
				break;
			case 16:
				dpad_state.right = event.value;
				break;
			}
			break;
		case JS_EVENT_AXIS: //if it's an axis, assign it to the correct axis output
			axis = get_axis_state(&event, axes);
			if (axis == 0) {
				hidoutReport.JX = axes[0].x;
				hidoutReport.JY = axes[0].y;
			} else if (axis == 1) {
				hidoutReport.Trigger = axes[1].x;
			}
			break;
		default:
			break;
		}

		//convert individual button presses of the dpad into a proper hat switch.
		if (dpad_state.up && dpad_state.left) {
			hidoutReport.DPAD = UP_LEFT;
		} else if (dpad_state.up && dpad_state.right) {
			hidoutReport.DPAD = UP_RIGHT;
		} else if (dpad_state.down && dpad_state.left) {
			hidoutReport.DPAD = DOWN_LEFT;
		} else if (dpad_state.down && dpad_state.right) {
			hidoutReport.DPAD = DOWN_RIGHT;
		} else if (dpad_state.up) {
			hidoutReport.DPAD = UP;
		} else if (dpad_state.down) {
			hidoutReport.DPAD = DOWN;
		} else if (dpad_state.left) {
			hidoutReport.DPAD = LEFT;
		} else if (dpad_state.right) {
			hidoutReport.DPAD = RIGHT;
		} else {
			hidoutReport.DPAD = CENTER;
		}

		//write the converted joystick output
		write(hidout, &hidoutReport, sizeof(PSNavReport));
		sched_yield();
	}

	//zero out everything
	hidoutReport.JX = 0;
	hidoutReport.JY = 0;
	hidoutReport.Trigger = -32766;
	hidoutReport.L1 = 0;
	hidoutReport.L2 = 0;
	hidoutReport.L3 = 0;
	hidoutReport.X = 0;
	hidoutReport.O = 0;
	hidoutReport.PS = 0;
	hidoutReport.DPAD = CENTER;
	write(hidout, &hidoutReport, sizeof(PSNavReport));

	close(hidout);
	close(js0in);
	closelog();

	exit(EXIT_SUCCESS);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
}

//utility functions
int read_event(int fd, struct js_event *event) {
	ssize_t bytes;

	bytes = read(fd, event, sizeof(*event));

	if (bytes == sizeof(*event))
		return 0;

	/* Error, could not read full event. */
	return -1;
}

size_t get_axis_state(struct js_event *event, struct axis_state axes[3]) {
	size_t axis = event->number / 2;

	if (axis < 3) {
		if (event->number % 2 == 0)
			axes[axis].x = event->value;
		else
			axes[axis].y = event->value;
	}

	return axis;
}

//notes
/*  Ps Nav Controller /dev/input/jsX input data from joystick class
buttons:
-4: L1
-6: trigger/L2
-11: L3
-0: X
-1: O
-13: dpad up
-14: dpad down
-15: dpad left
-16: dpad right
-10: ps button

Axis:
-0: X
-1: Y
-2: trigger/L2
*/
