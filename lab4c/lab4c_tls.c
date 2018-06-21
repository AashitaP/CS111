//NAME: Aashita Patwari
//EMAIL: harshupatwari@gmail.com


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
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <openssl/ssl.h>



const float B = 4275;               // B value of the thermistor
const float R0 = 100000;            // R0 = 100k
int logFlag = 0;
mraa_aio_context sensor;
char scale = 'F';
mraa_gpio_context button;
int stop = 0;
int period = 1;
char* fileName;
char* hostName;
FILE* logFile = NULL;
char* id;
int portNumber = 0;
int idFlag = 0;
int portFlag = 0;
int hostFlag = 0;
int sockfd;
SSL *ssl;

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

void shutdownProg()
{
    time_t rawtime;
    time(&rawtime);
    struct tm* timeLocal;
    timeLocal = localtime(&rawtime);
    fprintf(stdout, "%.2d:%.2d:%.2d SHUTDOWN\n", timeLocal -> tm_hour,timeLocal -> tm_min, timeLocal -> tm_sec);
    if(logFlag)
    {
        fprintf(logFile,"OFF\n");
        fprintf(logFile, "%.2d:%.2d:%.2d SHUTDOWN\n", timeLocal -> tm_hour,timeLocal -> tm_min, timeLocal -> tm_sec);
    }
    mraa_gpio_close(button);
    SSL_shutdown(ssl);
    close(sockfd);
    exit(0);
}

void processCommands(char* str)
{
    char* cptr;
    char* logPtr;
    if(strstr(str, "OFF") != NULL)
    {
        shutdownProg();
    } 
   else if(strstr(str, "SCALE=F") != NULL)
    {
        scale = 'F';
        if(logFlag)
        {
            fprintf(logFile,"SCALE=F\n");
        }
    }
    else if(strstr(str, "SCALE=C") != NULL)
    {
        scale = 'C';
        if(logFlag)
        {
            fprintf(logFile,"SCALE=C\n");
        }
    }
    else if(strstr(str, "STOP") != NULL)
    {
        stop = 1;
        if(logFlag)
        {      
            fprintf(logFile,"STOP\n");
        }
    }
    else if(strstr(str, "START") != NULL)
    {
        stop = 0;
        if(logFlag)
        {
            fprintf(logFile,"START\n");
        }
    }
    else if((cptr = strstr(str, "PERIOD")) != NULL)
    {
        while(*cptr != '=')
        {
            cptr++;
        }
        cptr++; //past '='
        char periodBuf[10];
        int i = 0;
        while(*cptr != 0)
        {
            periodBuf[i] = *cptr;
            i++;
            cptr++;
        }
        period = atoi(periodBuf); //if not period = atoi(cptr);
        if(logFlag)
        {     
            fprintf(logFile, "PERIOD=%d\n", period);
           // fclose(logFile);
        }
    }
    else if((logPtr = strstr(str, "LOG ")) != NULL)
    {
        if(logFlag)
        {
            fprintf(logFile, "LOG ");
            while(*logPtr != 'G')
            {
                logPtr++;
            }
            logPtr+=2; //past G and space
            while(*logPtr != '\n')
            {
                fprintf(logFile, "%c", *logPtr);
                logPtr++;
            }
            fprintf(logFile, "\n");
        }
    }

}

int main(int argc, char ** argv)
{
	int c;
	//int threadNumber = 0;

	static struct option long_options[] = {
		//{"period", required_argument, 0, 'p'}, 
		//{"scale", required_argument, 0, 's'},
		{"log", required_argument, 0, 'l'},
        {"id", required_argument, 0, 'i'},
        {"host", required_argument, 0, 'h'},
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
            case 'i':
                id = optarg;
                if(strlen(id) != 9)
                {
                    fprintf(stderr, "Id had to be 9 numbers\n");
                    exit(1);
                }
                idFlag = 1;
            break;
            case 'h':
                hostFlag = 1;
                hostName = optarg;
            break;
            default:
             fprintf(stderr, "Usage: --log=filename --id=# --host=hostname port number \n");
			 exit(1);
        }
    }

    if(optind < argc) //not end of args
    {
        portNumber = atoi(argv[optind]);
        if(portNumber > 0) 
        portFlag = 1;
        else
        {
            fprintf(stderr, "Invalid port number");
            exit(1);
        }
    }
    if(!portFlag || !idFlag || !logFlag || !hostFlag) //mandatory args
    {
        fprintf(stderr, "Usage: --log=filename --id=# --host=hostname port number \n");
		exit(1);
    }


  /*  button = mraa_gpio_init(60); //initialize MRAA pin 
    if (button == NULL) { 
        fprintf(stderr, "Failed to initialize button \n");
        mraa_deinit();
        exit(1);
    }

    //configure interface to be an input pin
    mraa_gpio_dir(button, MRAA_GPIO_IN); */

    sensor = mraa_aio_init(1); 
    if (sensor == NULL) { 
        fprintf(stderr, "Failed to initialize sensor \n");
        mraa_deinit();
        exit(1);
    }

//set up socket   (underlying connection)
	sockfd = 0;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "Error creating socket. Error code: %d, error message: %s \n", errno, strerror(errno));
		exit(1);
	}

    server = gethostbyname(hostName);
	 if (server == NULL) {
        fprintf(stderr,"ERROR, no such host. Error code: %d, error message: %s \n", errno, strerror(errno));
        exit(1);
    }

    memset(&serv_addr, '0', sizeof(serv_addr)); 


	serv_addr.sin_family = AF_INET;
	memcpy((char *) &serv_addr.sin_addr.s_addr, (char *) server -> h_addr, server -> h_length);
	serv_addr.sin_port = htons(portNumber);
	if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		fprintf(stderr, "Connection failed. Error code: %d, error message: %s \n", errno, strerror(errno));
		exit(1);
	}

    SSL_library_init(); //always returns 1 according to man page
    OpenSSL_add_all_algorithms();
    const SSL_METHOD *method = SSLv23_client_method();
    SSL_CTX *ctx;
    //try to create new context
    if((ctx = SSL_CTX_new(method)) == NULL)
    {
        fprintf(stderr, "Unable to create new SSL context structure \n");
        exit(1);
    }

    ssl = SSL_new(ctx);

    SSL_set_fd(ssl, sockfd); //attach SSL session to socket descriptor
    if(SSL_connect(ssl) != 1)
    {
        fprintf(stderr, "Unable to build session \n");
        exit(1);
    }

//send ID after setting up connection
    fprintf(logFile, "ID=%s\n", id);
    char idBuf[13];
    sprintf(idBuf, "ID=%s\n", id);
    if(SSL_write(ssl, idBuf, 13) <= 0)
    {
        fprintf(stderr, "SSL write operation unsucccessful\n");
        exit(2);
    }

    //read from socket

    if(close(STDIN_FILENO)<0)
    {
		fprintf(stderr, "Error closing. Error code: %d, error message: %s \n", errno, strerror(errno));
		exit(1);
    }
    dup2(sockfd, STDIN_FILENO); 

    struct pollfd fds[1];
    fds[0].fd = sockfd;
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

        if(stop == 0) //wrie to sockfd not stdout
        {
            char buffer[100];
            int buffCount = sprintf(buffer, "%.2d:%.2d:%.2d %0.1f\n",  timeLocal -> tm_hour,timeLocal -> tm_min, timeLocal -> tm_sec, tempValue);
            //dprintf(sockfd, "%d:%d:%d %0.1f\n", timeLocal -> tm_hour,timeLocal -> tm_min, timeLocal -> tm_sec, tempValue);
            if(logFlag)
            {
                fprintf(logFile, "%.2d:%.2d:%.2d %0.1f\n", timeLocal -> tm_hour,timeLocal -> tm_min, timeLocal -> tm_sec, tempValue);
            }

            if(SSL_write(ssl, buffer,buffCount) < 0)
            {
                fprintf(stderr, "SSL write operation unsucccessful\n");
                exit(2);
            }
        }
/*      if(mraa_gpio_read(button)) //if button is pressed, shut down program
        {
            shutdown();
        } */
        time_t start;
        time_t end;
        time(&start);
        time(&end);
        while(difftime(end, start) < period)
        {
          /*  if(mraa_gpio_read(button)) //if button is pressed, shut down program
            {
                shutdown();
            }
 */
            int ret = poll(fds, 1, 0);
		    if(ret == -1) {
			    fprintf(stderr, "polling error. errno: %d, message: %s \n", errno, strerror(errno));
			    exit(1);
		    }

		    if(fds[0].revents & POLLIN) //reading from input writing to server
		    {
			    char buffer[128];
			    memset(buffer, '0', 128);
			    int nBytes = SSL_read(ssl, buffer, 128); 
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


