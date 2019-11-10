/*
 * Global.h
 *
 *  Created on: Oct 26, 2019
 *      Author: Hans Beerman
 */

#ifndef GLOBAL_H_
#define GLOBAL_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef
		enum { false, true } bool;

#define CHECK_SYSTEM_TIME_WINDOW 480 // in minutes
#define VOLTAGE_SAMPLE_WINDOW 5 // in secs
#define VOLTAGE_REPORT_WINDOW 300 // in secs
#define VOLTAGE_SHOW_IN_TERM_WINDOW 5 // in secs
#define MINIMUM_VOLTAGE_LEVEL 4.0 // in V

#define MAX_TIME_BETWEEN_TESTOPROEPEN 330// in secs (default 5 min 30 s)

#define CONFIGDIR "/mnt/usb/var/config/"
#define CONFIGFILENAME "config.dat"

extern long long UpdateSystemTimeWindow; // in minutes
extern long long SampleVoltageWindow; // in secs
extern long long ReportVoltageWindow; // in secs
extern long long ShowVoltageInTermWindow; // in secs
extern double VoltageMinimumLevel; // in V
extern bool ValidDateTime;

extern long long current_Timestamp();

extern void remove_Chars(char *s, char c);

extern void string_Replace(char *s, char chr_to_repl, char repl_chr);

extern bool file_Exists (char * filename);

extern void load_params();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* GLOBAL_H_ */
