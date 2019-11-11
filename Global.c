/**
 \verbatim
 ******************************************************************************
 * Copyright (c) HansBeerman (NL), all rights reserved.
 ******************************************************************************
 * This source code and any compilation or derivative thereof is the sole
 * property of HansBeerman (NL)and is provided pursuant to a
 * Software Licence Agreement. This code is the proprietary information of
 * HansBeerman (NL)and is confidential in nature. Its use and
 * dissemination by any other party other than HansBeerman (NL)
 * is strictly limited by the confidential information provisions of the
 * Agreement referenced above.
 ******************************************************************************
 \endverbatim
 ******************************************************************************
 *  Project     RawHIDLog
 * \file        Global.c
 * \brief       Contains global parameters and functions
 * \details		Concept
 * \author      Hans Beerman, (c) HansBeerman (NL)
 * \date        \$LastChangedDate:: 2019-11-09 #$
 * \version     \$Revision: 0.0.0.1 $
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


#include <sys/time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <pigpio.h>
#include <math.h>
#include <unistd.h>

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
#include "Global.h"
#include "Log.h"
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

long long UpdateSystemTimeWindow = CHECK_SYSTEM_TIME_WINDOW; // in minutes
long long SampleVoltageWindow = VOLTAGE_SAMPLE_WINDOW; // in secs
long long ReportVoltageWindow = VOLTAGE_REPORT_WINDOW; // in secs
long long ShowVoltageInTermWindow = VOLTAGE_SHOW_IN_TERM_WINDOW; // in secs
double VoltageMinimumLevel = MINIMUM_VOLTAGE_LEVEL; // in V
bool ValidDateTime = false;

/*
 ******************************************************************************
 * Routine prototypes
 ******************************************************************************
 */

long long current_Timestamp();

void remove_Chars(char *s, char c);

void string_Replace(char *s, char chr_to_repl, char repl_chr);

bool file_Exists (char * filename);

void load_params();
/*
 ******************************************************************************
 * Routine implementation
 ******************************************************************************
 */

/**
 ******************************************************************************
 * \brief		  current_timestamp() returns current time stamp in ms
 * \details
 * \param[in]   - none
 * \param[out]  - none
 * \return      - long long: time stamp in ms
 * \pre         -
 * \post        -
 * \attention   -
 ******************************************************************************
 */


long long current_Timestamp()
{
/*
	struct timeval te;
    gettimeofday(&te, NULL);

    long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000;

    return milliseconds;
*/

	struct timespec tp;
	long long milliseconds;

	clock_gettime(CLOCK_MONOTONIC, &tp);

	milliseconds = tp.tv_sec * 1000 + tp.tv_nsec / 1000000;

	return milliseconds;
}

/**
 ******************************************************************************
 * \brief		  remove_Chars() removes all chars c from a string
 * \details
 * \param[in]   - char *s:	string of which chars will be removed
 * \param[in]	- char c:	char which will be removed from string *s
 * \param[out]  - char *s:	updated string with removed chars
 * \return      - none
 * \pre         -
 * \post        -
 * \attention   -
 ******************************************************************************
 */

void remove_Chars(char *s, char c)
{
    int writer = 0;
    int reader = 0;

    while (s[reader])
    {
        if (s[reader] != c)
        {
            s[writer++] = s[reader];
        }

        reader++;
    }

    s[writer] = 0;
}

/**
 ******************************************************************************
 * \brief		  string_Replace replaces all occurrences of a char with
 * 				  another char
 * \details
 * \param[in]   - char *s:				string in which chars will be replaced
 * \param[in]   - char chr_to_replace:	char to be replaced
 * \param[in]   - char repl_char:		replace char
 * \param[out]  - char *s:				string in which chars will be replaced
 * \return      - none
 * \pre         -
 * \post        -
 * \attention   -
 ******************************************************************************
 */

void string_Replace(char *s, char chr_to_repl, char repl_chr)
{
     int i = 0;

     while(s[i] != '\0')
     {
           if(s[i] == chr_to_repl)
           {
               s[i] = repl_chr;
           }
           i++;
     }
}

/**
 ******************************************************************************
 * \brief		  file_Exists checks if file exists
 * \details
 * \param[in]   - char * filename
 * \param[out]  - none
 * \return      - bool: true if file exist, false if not
 * \pre         -
 * \post        -
 * \attention   -
 ******************************************************************************
 */

bool file_Exists (char * filename)
{
   struct stat buf;

   if (stat(filename, &buf) == 0)
   {
	   return true;
   }
   else
   {
	   return false;
   }
}

void read_params_file()
{
	char current_filename[512];
	char param_filename[512];
	char current_line[255];
	int i;
	int j;
	unsigned char param_name[255];
	unsigned char param_value[255];
	bool get_param_name;
	bool param_name_ok;
	bool param_value_ok;
	char logmes[512];
	int line_nr = 0;
	struct stat USBSDA1stat;

	if (stat("/dev/sda1", &USBSDA1stat) < 0)
	{
		if (output_to_terminal == true)
		{
			printf("Error: No USB stick found, please insert!\n");
		}
		return;
	}

	sprintf(current_filename, "%s%s", CONFIGDIR, CONFIGFILENAME);

	log_message("***** Read_params_file! *****", !ValidDateTime);
	log_error_message("Read_params_file!", !ValidDateTime);

	if (file_Exists(current_filename))
	{
		FILE *fp = fopen(current_filename, "r");
		if (fp != NULL)
		{
			fseek(fp, 0, SEEK_END);
			if (ferror(fp) == 0)
			{
				int flen = ftell(fp);
				if (flen > 0)
				{
					line_nr = 0;
					fseek(fp, SEEK_SET, 0);
					while (fgets(current_line, 512, fp) != NULL)
					{
						line_nr++;
						i = 0;
						get_param_name = true;
						param_name_ok = false;
						param_value_ok = false;
						while ((current_line[i] != '\n') && (i < 255))
						{
							if (get_param_name == true)
							{
								if (current_line[i] != '=')
								{
									param_name[i] = current_line[i];
									param_name_ok = true;
								}
								else
								{
									param_name[i] = 0x00;
									get_param_name = false;
									j = 0;
								}
							}
							else
							{
								param_value[j] = current_line[i];
								param_value_ok = true;
								j++;
							}
							i++;
						}
						param_value[j] = 0x00;
						if ((param_name_ok == true) && (param_value_ok == true))
						{
							sprintf(param_filename, "/tmp/%s.dat", param_name);
							FILE *paramfilep = fopen(param_filename, "w");
							if (paramfilep != NULL)
							{
								fprintf(paramfilep, "%s", param_value);
//								sprintf(logmes, "Temp paramfile: %s created", param_filename);
//								log_message(logmes, !ValidDateTime);
								fclose(paramfilep);
							}
							else
							{
								sprintf(logmes, "Unable to create temp file: %s", param_filename);
								log_error_message(logmes, !ValidDateTime);
							}
						}
						else
						{
							if (param_name_ok == false)
							{
								log_error_message("Error: param_name_ok = false", !ValidDateTime);
							}
							if (param_value_ok == false)
							{
								log_error_message("Error: param_value_ok = false", !ValidDateTime);
							}
						}
					}
//					sprintf(logmes, "%d lines read from: %s", line_nr, current_filename);
//					log_message(logmes, !ValidDateTime);
				}
				else
				{
					sprintf(logmes, "Paramfile: %s seems empty", current_filename);
					log_error_message(logmes, !ValidDateTime);
				}
			}
			else
			{
				sprintf(logmes, "Unable to read from param file: %s", current_filename);
				log_error_message(logmes, !ValidDateTime);
			}
			fclose(fp);
		}
		else
		{
			sprintf(logmes, "Unable to open param file: %s", current_filename);
			log_error_message(logmes, !ValidDateTime);
		}
	}
	else
	{
		sprintf(logmes, "Param file: %s not found", current_filename);
		log_error_message(logmes, !ValidDateTime);
	}
}

double read_double_param(char * param_name, double default_value)
{
	double result;
	char paramfilename[512];
	char logmes[512];

	result = default_value;

	sprintf(paramfilename, "/tmp/%s.dat", param_name);
	if (file_Exists(paramfilename))
	{
		FILE *paramfilep = fopen(paramfilename, "r");
		if (paramfilep != NULL)
		{
			fscanf(paramfilep, "%lf", &result);
			fclose(paramfilep);
			unlink(paramfilename);
		}
		else
		{
			sprintf(logmes, "Unable to open temp paramfile: %s", paramfilename);
			log_error_message(logmes, !ValidDateTime);
		}
	}
	else
	{
		sprintf(logmes, "Temp paramfile: %s does not exists", paramfilename);
		log_error_message(logmes, !ValidDateTime);
	}
	return result;
}

int read_int_param(char * param_name, int default_value)
{
	int result;
	char paramfilename[512];
	char logmes[512];

	result = default_value;

	sprintf(paramfilename, "/tmp/%s.dat", param_name);
	if (file_Exists(paramfilename))
	{
		FILE *paramfilep = fopen(paramfilename, "r");
		if (paramfilep != NULL)
		{
			fscanf(paramfilep, "%d", &result);
			fclose(paramfilep);
			unlink(paramfilename);
		}
		else
		{
			sprintf(logmes, "Unable to open temp paramfile: %s", paramfilename);
			log_error_message(logmes, !ValidDateTime);
		}
	}
	else
	{
		sprintf(logmes, "Temp paramfile: %s does not exists", paramfilename);
		log_error_message(logmes, !ValidDateTime);
	}
	return result;
}

long long read_long_long_param(char * param_name, long long default_value)
{
	long long result;
	char paramfilename[512];
	char logmes[512];

	result = default_value;

	sprintf(paramfilename, "/tmp/%s.dat", param_name);
	if (file_Exists(paramfilename))
	{
		FILE *paramfilep = fopen(paramfilename, "r");
		if (paramfilep != NULL)
		{
			fscanf(paramfilep, "%lld", &result);
			fclose(paramfilep);
			unlink(paramfilename);
		}
		else
		{
			sprintf(logmes, "Unable to open temp paramfile: %s", paramfilename);
			log_error_message(logmes, !ValidDateTime);
		}
	}
	else
	{
		sprintf(logmes, "Temp paramfile: %s does not exists", paramfilename);
		log_error_message(logmes, !ValidDateTime);
	}
	return result;
}

bool read_bool_param(char * param_name, bool default_value)
{
	int result;
	char paramfilename[512];
	char logmes[512];

	result = default_value;

	sprintf(paramfilename, "/tmp/%s.dat", param_name);
	if (file_Exists(paramfilename))
	{
		FILE *paramfilep = fopen(paramfilename, "r");
		if (paramfilep != NULL)
		{
			fscanf(paramfilep, "%d", &result);
			fclose(paramfilep);
			unlink(paramfilename);
		}
		else
		{
			sprintf(logmes, "Unable to open temp paramfile: %s", paramfilename);
			log_error_message(logmes, !ValidDateTime);
		}
	}
	else
	{
		sprintf(logmes, "Temp paramfile: %s does not exists", paramfilename);
		log_error_message(logmes, !ValidDateTime);
	}
	return result;
}


void load_params()
{
	read_params_file();

	UpdateSystemTimeWindow = read_long_long_param("UpdateSystemTimeWindow", CHECK_SYSTEM_TIME_WINDOW); // in minutes
	if (UpdateSystemTimeWindow < 60)
	{
		UpdateSystemTimeWindow = 60;
	}
	if (UpdateSystemTimeWindow > 1440)
	{
		UpdateSystemTimeWindow = 1440;
	}
	SampleVoltageWindow = read_long_long_param("SampleVoltageWindow", VOLTAGE_SAMPLE_WINDOW); // in secs
	if (SampleVoltageWindow < 2)
	{
		SampleVoltageWindow = 2;
	}
	if (SampleVoltageWindow > 60)
	{
		SampleVoltageWindow = 60;
	}
	ReportVoltageWindow = read_long_long_param("ReportVoltageWindow", VOLTAGE_REPORT_WINDOW); // in secs
	if (ReportVoltageWindow < 10)
	{
		ReportVoltageWindow = 10;
	}
	if (ReportVoltageWindow > 1200)
	{
		ReportVoltageWindow = 1200;
	}
	ShowVoltageInTermWindow = read_long_long_param("ShowVoltageInTermWindow", VOLTAGE_SHOW_IN_TERM_WINDOW); // in secs
	if (ShowVoltageInTermWindow < 1)
	{
		ShowVoltageInTermWindow = 1;
	}
	if (ShowVoltageInTermWindow > 1200)
	{
		ShowVoltageInTermWindow = 1200;
	}
	VoltageMinimumLevel = read_double_param("double VoltageMinimumLevel", MINIMUM_VOLTAGE_LEVEL); // in V
	if (VoltageMinimumLevel < 2.0)
	{
		VoltageMinimumLevel = 2.0;
	}
	if (VoltageMinimumLevel > 5.0)
	{
		VoltageMinimumLevel = 5.0;
	}
}


/* EOF */
/* Template version:    4.0         */
/* Template status:     RELEASED    */
