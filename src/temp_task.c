#include "main_task.h"
#include "temp_task.h"
#include "temperature_sensor.h"

char *proj2 = "/tmp/proj1";

int SOCKET=0;


int process_temp_data(float *data)
{
	// int status;

	// status = i2c_open();

 //    if (status != 0) {
 //        printf("Failed to open I2C Bus\n");
 //        return -1;
 //    }

	if (get_sensortemp(data) == -1) {
		printf("Failed to fetch temperature data\n");
		return -1;
	}

	// if (i2c_close() != 0) {
	// 	printf("Failed to close I2C Bus\n");
	// 	return -1;
	// }

	return 0;
}

/* Temperature Timer Handler */
void temp_timer_handler(void)
{
	char buffer[50];
	char socket_buffer[50];
	float data = 0;

	pthread_mutex_lock(&lock);
	
	printf("TEMPERATURE TIMER HANDLER\n");

	process_temp_data(&data);
	
	int fd = open(proj2,O_WRONLY);

	sprintf(buffer,"TEMP THREAD DATA\tTID:%ld\ttemp = %f\n",syscall(SYS_gettid), data);

	mq_send(logger_queue,buffer,50,0);

	write(fd,"T",1);

	close(fd);
	
	pthread_mutex_unlock(&lock);

	sprintf(socket_buffer,"Temperature (C): %f\n",data);
	if((SOCKET == 1) || (SOCKET == 2) || (SOCKET == 3))
	{
		mq_send(socket_queue,socket_buffer,50,0);
		SOCKET=0;
	}


	
}


void *temp_thread_handler()
{

	char temp_buffer[50];
	char temp_info[]="Taking Temperature Reading......\n";

	struct sigevent temp_sev;
	struct timespec temp_mainTimeSpec;
	struct itimerspec temp_trigger;

	memset(&temp_sev,0,sizeof(struct sigevent));
	memset(&temp_trigger,0,sizeof(struct itimerspec));

	/* 
     * Set the notification method as SIGEV_THREAD:
     *
     * Upon timer expiration, `sigev_notify_function` (thread_handler()),
     * will be invoked as if it were the start function of a new thread.
     *
     */
    temp_sev.sigev_notify = SIGEV_THREAD;
    temp_sev.sigev_notify_function = &temp_timer_handler;
	temp_sev.sigev_value.sival_ptr = &temp_info;

	 /*
    * Create the timer. In this example, CLOCK_REALTIME is used as the
    * clock, meaning that we're using a system-wide real-time clock for 
    * this timer.
    */
	timer_create(CLOCK_REALTIME, &temp_sev, &temp_timerid);

	/* Timer expiration will occur withing 2 seconds after being armed
     * by timer_settime(). Then the interval timer will takeover 
     */
    temp_trigger.it_value.tv_sec = 2;

	/* Uncomment the following line to set the interval timer and
	 * and see the threadhandler() execute periodically.
	 */
    temp_trigger.it_interval.tv_sec = 2;

    timer_settime(temp_timerid,0,&temp_trigger,NULL);

    int status;

    status = i2c_open();

    if (status != 0) {
        printf("Failed to open I2C Bus\n");
        //return -1;
    }

}