/*
 * Log.h
 *
 *  Created on: 2019-01-29
 *      Author: Hans Beerman
 */

#ifndef LOG_H_
#define LOG_H_

#include "Global.h"

#define LOGFILE "RawHIDLog"
#define ERRORLOGFILE "RawHIDErrorLog"
#define LOGDIR "/mnt/usb/logfiles/"
#define ERRORLOGDIR "/mnt/usb/errorlogs/"

extern bool output_to_terminal;
extern bool USBStickError;
extern char USBStickErrorMessage[256];


extern bool USBStick_Check(void);

extern void log_message(char *amessage, bool disableDateTime);

extern void log_error_message(char *amessage, bool disableDateTime);

extern void debug_log(char *amessage);

#endif /* LOG_H_ */
