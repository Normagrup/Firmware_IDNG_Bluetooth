#ifndef TIME_FUNCTIONS_H
#define TIME_FUNCTIONS_H

#define SLEEP_DALI_TIME_MS              150
#define SLEEP_CONFIG_TIME_MS            1000
#define POLLING_TIMER_MS                2000
#define TEST_TIMER_MS                   60000
#define ADD_DEVICE_TIMER_MS             14000
#define CONFIRM_ADD_DEVICE_TIMER_MS     2000
#define NEW_ITERATION_TIMER_MS          2000
#define GROUP_FRAME_TIMER_MS            2000
#define COM_TIMER_MS                    2000
#define WEBSERVER_SEND_TIME_MS          50

void delay(uint16_t deadTimeMs);

#endif // TIME_FUNCTIONS_H
