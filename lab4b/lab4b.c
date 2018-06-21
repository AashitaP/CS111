#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <time.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include "mraa/aio.h"
#include "mraa/gpio.h"
#include <mraa.h>
#include <aio.h>
#include "fcntl.h"
#include <unistd.h>
#include <poll.h>



const float B = 4275;               // B value of the thermistor
const float R0 = 100000;            // R0 = 100k
int logFlag = 0;
mraa_aio_context sensor;
char scale = 'F';
mraa_gpio_context button;
int stop = 0;
int period = 1;
char* fileName;
FILE* logFile = NULL;

float CtoF(float value)
{
    float farenheit = value*9/5 + 32;
    return farenheit;
}

float FtoC(float value) //not required
{
    float celsius = (value - 32) / (9/5);
    return celsius;
}

float calculateTemp(int value, char unit)
{
    float tempValue = 1023.0/value - 1.0;
    tempValue = R0*tempValue;
    float temperature = 1.0/(log(tempValue/R0)/B + 1/298.15)-273.15;
    if(unit == 'C')
    {
        return temperature;
    }
    else if(unit == 'F')
    {
        return CtoF(temperature);
    }

    return temperature;
}

void shutdown()
{
    time_t rawtime;
    time(&rawtime);
    struct tm* timeLocal;
    timeLocal = localtime(&rawtime);
    fprintf(stdout, "%d:%d:%d SHUTDOWN\n", timeLocal -> tm_hour,timeLocal -> tm_min, timeLocal -> tm_sec);
    if(logFlag)
    {
       /* FILE* logFile = fopen(fileName, "a+");
        if (logFile == NULL){
            fprintf(stderr, "Error opening log file. Errno: %d, error message: %s \n", errno, strerror(errno));
            exit(1);
        }  */
        fprintf(logFile,"OFF\n");
        fprintf(logFile, "%d:%d:%d SHUTDOWN\n", timeLocal -> tm_hour,timeLocal -> tm_min, timeLocal -> tm_sec);
       // fclose(logFile);
    }
    mraa_gpio_close(button);
    exit(0);
}

void processCommands(char* str)
{
    char* cptr;
    char* logPtr;
    if(strstr(str, "OFF") != NULL)
    {
        shutdown();
    } 
    else if(strstr(str, "SCALE=F") != NULL)
    {
        scale = 'F';
        if(logFlag)
        {
          /*  FILE* logFile = fopen(fileName, "a+");
            if (logFile == NULL){
                fprintf(stderr, "Error opening log file. Errno: %d, error message: %s \n", errno, strerror(errno));
                exit(1);
            } */
            fprintf(logFile,"SCALE=F\n");
          //  fclose(logFile);
        }
    }
    else if(strstr(str, "SCALE=C") != NULL)
    {
        scale = 'C';
        if(logFlag)
        {
           /* FILE* logFile = fopen(fileName, "a+");
            if (logFile == NULL){
                fprintf(stderr, "Error opening log file. Errno: %d, error message: %s \n", errno, strerror(errno));
                exit(1);
            }   */         
            fprintf(logFile,"SCALE=C\n");
           // fclose(logFile);
        }
    }
    else if(strstr(str, "STOP") != NULL)
    {
        stop = 1;
        if(logFlag)
        {
           /* FILE* logFile = fopen(fileName, "a+");
            if (logFile == NULL){
                fprintf(stderr, "Error opening log file. Errno: %d, error message: %s \n", errno, strerror(errno));
                exit(1);
            }  */           
            fprintf(logFile,"STOP\n");
           // fclose(logFile);
        }
    }
    else if(strstr(str, "START") != NULL)
    {
        stop = 0;
        if(logFlag)
        {
           /* FILE* logFile = fopen(fileName, "a+");
            if (logFile == NULL){
                fprintf(stderr, "Error opening log file. Errno: %d, error message: %s \n", errno, strerror(errno));
                exit(1);
            } */
            fprintf(logFile,"START\n");
           // fclose(logFile);
        }
    }
    else if((cptr = strstr(str, "PERIOD")) != NULL)
    {
        while(*cptr != '=')
        {
            cptr++;
        }
        cptr++; //past '='
        period = atoi(cptr);
        if(logFlag)
        {
           /* FILE* logFile = fopen(fileName, "a+");
            if (logFile == NULL){
                fprintf(stderr, "Error opening log file. Errno: %d, error message: %s \n", errno, strerror(errno));
                exit(1);
            }   */       
            fprintf(logFile, "PERIOD=%d\n", period);
           // fclose(logFile);
        }
    }
    else if((logPtr = strstr(str, "LOG ")) != NULL)
    {
        if(logFlag)
        {
            /*FILE* logFile = fopen(fileName, "a+");
            if (logFile == NULL){
                fprintf(stderr, "Error opening log file. Errno: %d, error message: %s \n", errno, strerror(errno));
                exit(1);
            } */
            fprintf(logFile, "LOG ");
            while(*logPtr != 'G')
            {
                logPtr++;
            }
            logPtr+=2; //past G and
            while(*logPtr != '\n')
            {
                fprintf(logFile, "%c", *logPtr);
                logPtr++;
            }
            fprintf(logFile, "\n");
           // fclose(logFile);
        }
    }

}

int main(int argc, char ** argv)
{
	int c;
	//int threadNumber = 0;

	static struct option long_options[] = {
		{"period", required_argument, 0, 'p'}, 
		{"scale", required_argument, 0, 's'},
		{"log", required_argument, 0, 'l'},
		{0, 0, 0, 0}
	};

	while((c = getopt_long(argc, argv, "sd", long_options, NULL)) != -1) {
		switch(c) {
			case 'p':
				period = atoi(optarg);
			break;
			case 's':
				scale = optarg[0];
                if(scale != 'F' && scale != 'C')
                {
                    fprintf(stderr, "Scale shouldbe either 'F' or 'C' ");
                    exit(1);
                }
			break;	
			case 'l':
                logFlag = 1;
				fileName = optarg;
                logFile = fopen(fileName, "w");
                if (logFile == NULL){
                    fprintf(stderr, "Error creating log file. Errno: %d, error message: %s \n", errno, strerror(errno));
                    exit(1);
                }
			break;
            default:
             fprintf(stderr, "Usage: [--period=#] [--scale=[FC]] [--log=filename]] \n");
			 exit(1);
        }
    }

    button = mraa_gpio_init(60); //initialize MRAA pin 
    if (button == NULL) { 
        fprintf(stderr, "Failed to initialize button \n");
        mraa_deinit();
        exit(1);
    }

    //configure interface to be an input pin
    mraa_gpio_dir(button, MRAA_GPIO_IN);

    sensor = mraa_aio_init(1); 
    if (sensor == NULL) { 
        fprintf(stderr, "Failed to initialize sensor \n");
        mraa_deinit();
        exit(1);
    }

    struct pollfd fds[1];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN | POLLHUP | POLLERR;

    while(1)
    {
        //reading time
        time_t rawtime;
        time(&rawtime);
        struct tm* timeLocal;
        timeLocal = localtime(&rawtime);


        //read temperature

        int tempSensor = mraa_aio_read(sensor);
        float tempValue = calculateTemp(tempSensor, scale);

        if(stop == 0)
        {
            fprintf(stdout, "%d:%d:%d %0.1f\n", timeLocal -> tm_hour,timeLocal -> tm_min, timeLocal -> tm_sec, tempValue);
            if(logFlag)
            {
               /* FILE* logFile = fopen(fileName, "a+");
                if (logFile == NULL){
                    fprintf(stderr, "Error opening log file. Errno: %d, error message: %s \n", errno, strerror(errno));
                    exit(1);
                } */
                fprintf(logFile, "%d:%d:%d %0.1f\n", timeLocal -> tm_hour,timeLocal -> tm_min, timeLocal -> tm_sec, tempValue);
               // fclose(logFile);
            }
        }
        if(mraa_gpio_read(button)) //if button is pressed, shut down program
        {
            shutdown();
        }

        time_t start;
        time_t end;
        time(&start);
        time(&end);
        while(difftime(end, start) < period)
        {
            if(mraa_gpio_read(button)) //if button is pressed, shut down program
            {
                shutdown();
            }

            int ret = poll(fds, 1, 0);
		    if(ret == -1) {
			    fprintf(stderr, "polling error. errno: %d, message: %s \n", errno, strerror(errno));
			    exit(1);
		    }

		    if(fds[0].revents & POLLIN) //reading from input writing to server
		    {
			    char buffer[128];
			    memset(buffer, '0', 128);
			    int nBytes = read(STDIN_FILENO, buffer, 128); 
			    if(nBytes < 0)
			    {
				    fprintf(stderr, "error with read function. errno: %d, message: %s \n", errno, strerror(errno));
				    exit(1);
			    }

                char tempCommand[20] = "";
                int index = 0;
                int i;
                for(i = 0;i<nBytes;i++)
                {
                    if(buffer[i]!='\n')
                    {
                        tempCommand[index]=buffer[i];
                        index++;
                    }
                    else
                    {
                        char command[20];
                        strcpy(command,tempCommand);
                        processCommands(command);
                        memset(tempCommand,0,20);
                        index = 0;
                    }
                }
            }


            time(&end);
        }

    }



}


