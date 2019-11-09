/**
 \verbatim
 ******************************************************************************
 * Copyright (c) HansBeerman B.V. (NL), all rights reserved.
 ******************************************************************************
 * This source code and any compilation or derivative thereof is the sole
 * property of HansBeerman B.V. (NL)and is provided pursuant to a
 * Software Licence  Agreement. This code is the proprietary information of
 * HansBeerman B.V. (NL)and is confidential in nature. Its use and
 * dissemination by any other party other than HansBeerman B.V. (NL)
 * is strictly limited by the confidential information provisions of the
 * Agreement referenced above.
 ******************************************************************************
 \endverbatim
 ******************************************************************************
 *  Project     RawHIDLog
 * \file        Log.c
 * \brief       To log text to a logfile
 * \details		Final
 * \author      Hans Beerman, HansBeerman B.V. (NL)
 * \date        \$LastChangedDate:: 2019-11-06 #$
 * \version     \$Revision:0.0.0.1 $
 * \attention   -
 ******************************************************************************
 */



 /*
 ******************************************************************************
 * Standard include files
 ******************************************************************************
 */
/* MISRA RULE 19.15 VIOLATION:  The following exclusion was added because of the
 *                              inclusion of multiple headers, which are the
 *                              same. This could result in redefinitions of
 *                              types causing out of boundary scenarios.
 *                              The violation may only be turned off for header
 *                              files of external units.
 *                              Since all header files are protected against
 *                              multiple inclusion, no redefinitions can occur.
 */
/*lint -save -e537 */
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <linux/types.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
/*lint -restore */

/*
 ******************************************************************************
 * Project include files
 ******************************************************************************
 */

/* MISRA RULE 19.15 VIOLATION:  The following exclusion was added because of the
 *                              inclusion of multiple headers, which are the
 *                              same. This could result in redefinitions of
 *                              types causing out of boundary scenarios.
 *                              The violation may only be turned off for header
 *                              files of external units.
 *                              Since all header files are protected against
 *                              multiple inclusion, no redefinitions can occur.
 */
/*lint -save -e537 */
#include "Log.h"
#include "Global.h"
/*lint -restore */

/*
 ******************************************************************************
 * Constants
 ******************************************************************************
 */

/*
 ******************************************************************************
 * Types
 ******************************************************************************
 */

/*
 ******************************************************************************
 * Macro defines
 ******************************************************************************
 */


/*
 ******************************************************************************
 * Variable declarations
 ******************************************************************************
 */
bool logfile_is_updated = false;
bool error_logfile_is_updated = false;

/*
 ******************************************************************************
 * Routine prototypes
 ******************************************************************************
 */

void log_message(char *amessage, bool disableDateTime);

void log_error_message(char *amessage, bool disableDateTime);

bool logging_available(void);

bool error_logging_available(void);

void debug_log(char *amessage);

/*
 ******************************************************************************
 * Routine implementation
 ******************************************************************************
 */


bool log_created = false;
bool error_log_created = false;

/**
 ******************************************************************************
 * \brief		  log_message stores text messages in a local file
 * \details
 * \param[in]   - char *amessage = the message to be logged
 * \param[out]  - none
 * \return      - none
 * \pre         -
 * \post        -
 * \attention   - Logfiles of 31 days or more ago are deleted
 ******************************************************************************
 */

 void log_message(char *amessage, bool disableDateTime)
 {
	 FILE *file;
	 struct stat filestat;
	 int current_day;
	 int next_day;
	 char current_filename[512];
	 char next_filename[512];
	 time_t curtime;
	 struct tm *loctime;
	 char current_time_date[256];
	 char mesout[1024];
	 int i;

	 /* Get the current time. */
	 curtime = time (NULL);

	 /* Convert it to local time representation. */
	 loctime = localtime (&curtime);

	 current_day = loctime->tm_mday;

	 sprintf(current_time_date, "%4d-%2d-%2d%s%2d:%2d:%2d", loctime->tm_year + 1900, loctime->tm_mon + 1, loctime->tm_mday, "|||" , loctime->tm_hour, loctime->tm_min, loctime->tm_sec);

	 for (i = 0; i < strlen(current_time_date); i++)
	 {
		 if (current_time_date[i] == ' ')
		 {
			 current_time_date[i] = '0';
		 }
		 else
		 {
			 if (current_time_date[i] == '|')
			 {
				 current_time_date[i] = ' ';
			 }
		 }
	 }
	 next_day = current_day + 1;
	 if (next_day > 31)
	 {
		 next_day = 1;
	 }

	 sprintf(current_filename, "%s%s_%d.log", LOGDIR, LOGFILE, current_day);
	 sprintf(next_filename, "%s%s_%d.log", LOGDIR, LOGFILE, next_day);
	 if (stat(next_filename, &filestat) >= 0)
	 {
		 remove(next_filename);
	 }

	 if (stat(current_filename, &filestat) < 0)
	 {
		 log_created = false;
	 }
	 else
	 {
		 log_created = true;
	 }
	 if (log_created == false)
	 {
		 file = fopen(current_filename, "w");
		 log_created = true;
	 }
	 else
	 {
		 file = fopen(current_filename, "a");
	 }
	 if (file == NULL)
	 {
		 log_created = false;
		 return;
	 }
	 else
	 {
		 logfile_is_updated = true;
		 if (disableDateTime == true)
		 {
			 sprintf(mesout, "%c%c%c%s\n", 9, 9, 9, amessage);
		 }
		 else
		 {
			 sprintf(mesout, "%s%c%s\n", current_time_date, 9, amessage);
		 }

		 fputs(mesout, file);
		 fclose(file);
	 }
 }

 /**
  ******************************************************************************
  * \brief		  log_error_message stores text messages in a local file
  * \details
  * \param[in]   - char *amessage = the message to be logged
  * \param[out]  - none
  * \return      - none
  * \pre         -
  * \post        -
  * \attention   -
  ******************************************************************************
  */

  void log_error_message(char *amessage, bool disableDateTime)
  {
 	 FILE *file;
 	 struct stat filestat;
 	 char current_filename[512];
 	 time_t curtime;
 	 struct tm *loctime;
 	 char current_time_date[256];
 	 char mesout[1024];
 	 int i;

 	 /* Get the current time. */
 	 curtime = time (NULL);

 	 /* Convert it to local time representation. */
 	 loctime = localtime (&curtime);

 	 sprintf(current_time_date, "%4d-%2d-%2d%s%2d:%2d:%2d", loctime->tm_year + 1900, loctime->tm_mon + 1, loctime->tm_mday, "|||" , loctime->tm_hour, loctime->tm_min, loctime->tm_sec);

 	 for (i = 0; i < strlen(current_time_date); i++)
 	 {
 		 if (current_time_date[i] == ' ')
 		 {
 			 current_time_date[i] = '0';
 		 }
 		 else
 		 {
 			 if (current_time_date[i] == '|')
 			 {
 				 current_time_date[i] = ' ';
 			 }
 		 }
 	 }

 	 sprintf(current_filename, "%s%s.log", ERRORLOGDIR, ERRORLOGFILE);

 	 if (stat(current_filename, &filestat) < 0)
 	 {
 		 error_log_created = false;
 	 }
 	 else
 	 {
 		error_log_created = true;
 	 }
 	 if (log_created == false)
 	 {
 		 file = fopen(current_filename, "w");
 		 error_log_created = true;
 	 }
 	 else
 	 {
 		 file = fopen(current_filename, "a");
 	 }
 	 if (file == NULL)
 	 {
 		 error_log_created = false;
 		 return;
 	 }
 	 else
 	 {
 		 error_logfile_is_updated = true;
 		 if (disableDateTime == true)
 		 {
 			 sprintf(mesout, "%c%c%c%s\n", 9, 9, 9, amessage);
 		 }
 		 else
 		 {
 			 sprintf(mesout, "%s%c%s\n", current_time_date, 9, amessage);
 		 }

 		 fputs(mesout, file);
 		 fclose(file);
 	 }
  }


 /**
  ******************************************************************************
  * \brief		  logging_available returns true if logging is available
  * \details
  * \param[in]   - none
  * \param[out]  - bool true: logging available, false: no logging available
  * \return      - none
  * \pre         -
  * \post        -
  * \attention   -
  ******************************************************************************
  */
 bool logging_available(void)
 {
	 char current_filename[512];
	 int i;
	 struct stat filestat;

	 if (logfile_is_updated == false)
	 {
		 return false;
	 }
	 for (i = 0; i <= 30; i++)
	 {
		 sprintf(current_filename, "%s%s_%d.log", LOGDIR, LOGFILE, i);
		 if (stat(current_filename, &filestat) >= 0)
		 {
			 return true;
		 }
	 }

	 return false;
 }

 /**
  ******************************************************************************
  * \brief		  error_logging_available returns true if error logging is
  * 			  available
  * \details
  * \param[in]   - none
  * \param[out]  - bool true: error logging available, false: not available
  * \return      - none
  * \pre         -
  * \post        -
  * \attention   -
  ******************************************************************************
  */
 bool error_logging_available(void)
 {
	 char current_filename[512];
	 struct stat filestat;

	 if (error_logfile_is_updated == false)
	 {
		 return false;
	 }
	 sprintf(current_filename, "%s%s.log", ERRORLOGDIR, ERRORLOGFILE);
	 if (stat(current_filename, &filestat) >= 0)
	 {
		 return true;
	 }
	 return false;
 }



 /**
  ******************************************************************************
  * \brief		  debug_log stores text messages in the error log file
  * \details
  * \param[in]   - char *amessage = the message to be logged
  * \param[out]  - none
  * \return      - none
  * \pre         -
  * \post        -
  * \attention   -
  ******************************************************************************
  */

  void debug_log(char *amessage)
  {
	  char messout[4096];

	  sprintf(messout, "DEBUG function: %s", amessage);
	  log_message(messout, false);
  }


/* EOF */
 /* Template version:    4.0         */
 /* Template status:     RELEASED    */
