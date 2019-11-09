/*
 ============================================================================
 Name        : Test-001.c
 Author      : Hans Beerman
 Version     : 1.0.0.0
 Copyright   : (c) 2019 Hans Beerman
 Description : Test I2C connection to Wemos device
 ============================================================================
 */

#define _BSD_SOURCE
#define _XOPEN_SOURCE
#include <sys/time.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <pigpio.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include "Global.h"
#include "Log.h"
#include "ABE_ADCPi.h"
#include <features.h>

#define CHECK_SYSTEM_TIME_WINDOW 480 // in minutes
#define VOLTAGE_SAMPLE_WINDOW 5 // in secs
#define VOLTAGE_REPORT_WINDOW 300 // in secs
#define VOLTAGE_SHOW_IN_TERM_WINDOW 5 // in secs
#define MINIMUM_VOLTAGE_LEVEL 4.0 // in V

bool program_is_running = true;

long long current_Timestamp()
{
    struct timeval te;
    gettimeofday(&te, NULL);

    long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000;

    return milliseconds;
}

void setsystemdatetime(char * newdatetime)
{
	char clockcommand[50];
	char funcresultline[255];
	FILE *fp;
	struct tm result;
	struct tm *currtime;
	char buffer[50];

	time_t newtime;
	time_t current_system_time;

	if (strcmp(newdatetime, "0000-00-00 00:00:00") != 0)
	{
		current_system_time = time(NULL);
		currtime = gmtime(&current_system_time);

		if (strptime(newdatetime, "%Y-%m-%d %H:%M:%S", &result) != NULL)
		{
			result.tm_isdst = currtime->tm_isdst;
			newtime = mktime(&result) - 3600;
			if (((current_system_time - newtime) > 1) || ((current_system_time - newtime) < 0))
			{
				strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", localtime(&newtime));
				sprintf(clockcommand, "date -u -s \"%s\"", buffer);
				fp = popen(clockcommand, "r");
				if (fp != NULL)
				{
					fgets(funcresultline, sizeof(funcresultline), fp);
					pclose(fp);
				}
			}
		}
	}

}


int main(int argc, char **argv)
{
	bool output_to_terminal = false;
	int ReadRawDevice;
	int CharsRead;
	char Buffer[1025];
	char LogStr[4096];
	char tmpstr[255];
	char DateTimeStr[255];
	int i;
	int j;
	int k;
	bool ChangeSystemTime = true;
	bool SystemDateTimeUpdated = false;
	struct stat USBHIDstat;
	bool USBHidConnected = false;
	long long Next_System_Time_Check = 0;
	long long Next_Voltage_Report_Time = 0;
	long long Next_Voltage_Show_In_Term_Time = 0;

	long long next_voltage_sample;
	double min_voltage_level = 100000.0;
	double max_voltage_level = 0;
	double current_voltage_level = 0.0;
	bool skip_First_Message = true;
	bool skip_Second_Message = true;

	bool VoltageLevelError = false;

	if (argc > 1)
	{
		if ((strcmp(argv[1], "-log") == 0) || (strcmp(argv[1], "/log") == 0) || (strcmp(argv[1], "--log") == 0))
		{
			output_to_terminal = true;
			printf("Log to terminal = on\n");
		}
	}

	log_message("***** Init USB connection *****", true);
	log_error_message("Init USB connection", true);
	ReadRawDevice = -1;
	while (ReadRawDevice < 0)
	{
		ReadRawDevice = open("/dev/hidraw0", O_RDONLY | O_NONBLOCK);
		if (ReadRawDevice < 0)
		{
			if (output_to_terminal == true)
			{
				printf("Error opening USB device\n");
			}
			usleep(1000000);
		}
	}
	USBHidConnected = true;
	log_message("***** Program Started *****", true);
	log_error_message("Program Started", true);
	if (output_to_terminal == true)
	{
		printf("Program started\n\n");
	}

	next_voltage_sample = current_Timestamp() + VOLTAGE_SAMPLE_WINDOW * 1000;
	Next_Voltage_Report_Time = current_Timestamp() + VOLTAGE_REPORT_WINDOW * 1000;
	Next_Voltage_Show_In_Term_Time = current_Timestamp() + VOLTAGE_SHOW_IN_TERM_WINDOW * 1000;
	while (program_is_running == true)
	{
		if (stat("/dev/hidraw0", &USBHIDstat) < 0)
		{
			log_message("***** Error: USB connection lost! *****", false);
			log_error_message("Error: USB connection lost!", false);
			if (output_to_terminal == true)
			{
				printf("Error: USB connection lost!\n");
			}
			if (USBHidConnected == true)
			{
				if (output_to_terminal == true)
				{
					printf("Closing USB connection\n");
				}
				close(ReadRawDevice);
				USBHidConnected = false;
			}
			if (output_to_terminal == true)
			{
				printf("Waiting for USB connection!\n");
			}
			while (stat("/dev/hidraw0", &USBHIDstat) < 0)
			{
				usleep(100000);
			}
			if (output_to_terminal == true)
			{
				printf("USB connection available again, trying to connect!\n");
			}
			ReadRawDevice = -1;
			while (ReadRawDevice < 0)
			{
				ReadRawDevice = open("/dev/hidraw0", O_RDONLY | O_NONBLOCK);
				if (ReadRawDevice < 0)
				{
					if (output_to_terminal == true)
					{
						printf("Error opening USB device\n");
					}
					usleep(1000000);
				}
			}
			log_message("***** Solved: USB connected *****", false);
			log_error_message("Solved: USB connected", false);
			if (output_to_terminal == true)
			{
				printf("Solved: USB connected\n");
			}
		}
		LogStr[0] = 0x00;
		CharsRead = 0;
		CharsRead = read(ReadRawDevice, Buffer, 1024);
		if (((skip_First_Message == true) || (skip_Second_Message == true)) && (CharsRead > 0))
		{
			CharsRead = 0;
			if (skip_First_Message == false)
			{
				skip_Second_Message = false;
			}
			skip_First_Message = false;
		}
		if (CharsRead > 0)
		{
			Buffer[CharsRead] = 0x00;

			j = 0;
			for (i = 0; i < CharsRead; i++)
			{
				if (((Buffer[i] > 0) && (Buffer[i] < 32)) || (Buffer[i] >= 127))
				{
					sprintf(tmpstr, "[0x%02X]", Buffer[i]);
					for (k = 0; k < 6; k++)
					{
						LogStr[j] = tmpstr[k];
						j++;
					}
				}
				else
				{
					LogStr[j] = Buffer[i];
					j++;
				}
				LogStr[j] = 0x00;
			}

			if (ChangeSystemTime == true)
			{

				// find date and time in logmessage
				if (strlen(LogStr) > 0)
				{
					char *pfound = strstr(LogStr, "TESTOPROEP HOOFDSYSTEEM");
					if (pfound != NULL)
					{
						char *pfound = strstr(LogStr, ")");
						if (pfound != NULL)
						{
							j = 0;
							DateTimeStr[j] = '2';
							j++;
							DateTimeStr[j] = '0';
							j++;
							DateTimeStr[j] = pfound[7];
							j++;
							DateTimeStr[j] = pfound[8];
							j++;
							DateTimeStr[j] = '-';
							j++;
							for (i = 1; i < 6; i++)
							{
								DateTimeStr[j] = pfound[i];
								j++;
							}
							for (i = 9; i < 18; i++)
							{
								DateTimeStr[j] = pfound[i];
								j++;
							}
							DateTimeStr[j] = 0x00;
						}
						if (output_to_terminal == true)
						{
							printf("Current Date/Time = %s\n", DateTimeStr);
						}
						setsystemdatetime(DateTimeStr);
						if (output_to_terminal == true)
						{
							printf("System date/time set.\n");
						}
						SystemDateTimeUpdated = true;
						Next_System_Time_Check = current_Timestamp() + (CHECK_SYSTEM_TIME_WINDOW * 60000);
						ChangeSystemTime = false;
					}
				}
			}

			if (output_to_terminal == true)
			{
				printf("%s\n", LogStr);
			}
			if (SystemDateTimeUpdated == true)
			{
				log_message(LogStr, false);
			}
			else
			{
				log_message(LogStr, true);
			}
		}
		if (ChangeSystemTime == false)
		{
			if (current_Timestamp() >= Next_System_Time_Check)
			{
				ChangeSystemTime = true;
			}
		}
		if (current_Timestamp() >= next_voltage_sample)
		{
			next_voltage_sample = current_Timestamp() + VOLTAGE_SAMPLE_WINDOW * 1000;
			// the correction factor 1.279762 is needed because a 4k7 resistor is placed in the input lead (the ADC input internally has a 10K and a 16K8 in series
			current_voltage_level = 1.279762 * read_voltage(0x68, 1, 18, 1, 1); // read from adc chip 1, channel 1, 18 bit, pga gain set to 1 and continuous conversion mode

			if ((current_voltage_level < min_voltage_level) && (min_voltage_level < 100))
			{
				min_voltage_level = current_voltage_level;
			}
			else
			{
				if ((current_voltage_level < min_voltage_level) && (current_voltage_level > 3))
				{
					min_voltage_level = current_voltage_level;
				}
			}
			if (current_voltage_level > max_voltage_level)
			{
				max_voltage_level = current_voltage_level;
			}
			if (current_voltage_level < MINIMUM_VOLTAGE_LEVEL)
			{
				if (VoltageLevelError == false)
				{
					sprintf(LogStr, "***** Error: Pager voltage = %G V (this is below the limit of %G V) *****", current_voltage_level, MINIMUM_VOLTAGE_LEVEL);
					log_message(LogStr, false);
					sprintf(LogStr, "Error: Pager voltage = %G V (this is below the limit of %G V)", current_voltage_level, MINIMUM_VOLTAGE_LEVEL);
					log_error_message(LogStr, false);
					if (output_to_terminal == true)
					{
						printf("%s\n", LogStr);
					}
					VoltageLevelError = true;
				}
			}
			else
			{
				if (VoltageLevelError == true)
				{
					sprintf(LogStr, "***** Solved: Pager voltage = %G V (this is at or above the limit of %G V) *****", current_voltage_level, MINIMUM_VOLTAGE_LEVEL);
					log_message(LogStr, false);
					sprintf(LogStr, "Solved: Pager voltage = %G V (this is at or above the limit of %G V)", current_voltage_level, MINIMUM_VOLTAGE_LEVEL);
					log_error_message(LogStr, false);
					if (output_to_terminal == true)
					{
						printf("%s\n", LogStr);
					}
					VoltageLevelError = false;
				}
			}
		}

		if (current_Timestamp() > Next_Voltage_Report_Time)
		{
			Next_Voltage_Report_Time = current_Timestamp() + VOLTAGE_REPORT_WINDOW * 1000;
			sprintf(LogStr, "***** Current voltage level pager: %G V *****", current_voltage_level);
			log_message(LogStr, false);
			if (output_to_terminal == true)
			{
				printf("%s\n", LogStr);
			}
			if (min_voltage_level < 100)
			{
				sprintf(LogStr, "***** Minimum voltage level pager: %G V *****", min_voltage_level);
			}
			else
			{
				sprintf(LogStr, "***** Minimum voltage level detection waiting for first positive value measured *****");
			}
			log_message(LogStr, false);
			if (output_to_terminal == true)
			{
				printf("%s\n", LogStr);
			}
			sprintf(LogStr, "***** Maximum voltage level pager: %G V *****", max_voltage_level);
			log_message(LogStr, false);
			if (output_to_terminal == true)
			{
				printf("%s\n", LogStr);
			}
		}

		if (current_Timestamp() > Next_Voltage_Show_In_Term_Time)
		{
			Next_Voltage_Show_In_Term_Time = current_Timestamp() + VOLTAGE_SHOW_IN_TERM_WINDOW * 1000;
			if (output_to_terminal == true)
			{
				printf("------------\n");
				printf("Current voltage level pager: %G V\n", current_voltage_level);
				if (min_voltage_level < 100)
				{
					printf("Minimum voltage level pager: %G V\n", min_voltage_level);
				}
				else
				{
					printf("Minimum voltage level detection waiting for first positive value measured\n");
				}
				printf("Maximum voltage level pager: %G V\n", max_voltage_level);
				printf("------------\n");
			}
		}
	}

	close(ReadRawDevice);

	return EXIT_SUCCESS;
}
