#ifndef GLOBAL_DEF_H
#define GLOBAL_DEF_H

// Log events
#define LOG_DEVICE_ADDED                      0x05
#define LOG_DEVICE_REMOVED                    0x06
#define LOG_ALL_DEVICE_REMOVED                0x07

// Communication Failures
#define LOG_COMMUNICATION_FAILURE             0x01
#define LOG_COMMUNICATION_RECOVERED           0x10

// Battery Failures
#define LOG_BATTERY_FAILURE                   0x02
#define LOG_BATTERY_RECOVERED                 0x20

// Lamp Failures
#define LOG_LAMP_FAILURE                      0x03
#define LOG_LAMP_RECOVERED                    0x30

// Autonomy (Duration) Failures
#define LOG_DURATION_FAILURE                  0x04
#define LOG_DURATION_RECOVERED                0x40

// Test Requests
#define LOG_TEST_REQUESTED_FUNCTIONAL         0x08
#define LOG_TEST_REQUESTED_DURATION           0x09
#define LOG_TEST_STOPPED                      0x0A

// Generic Test Results
#define LOG_TEST_COMPLETED_FUNCTIONAL         0x0B
#define LOG_TEST_COMPLETED_DURATION           0x0C

// Specific FT Results
#define LOG_TEST_FT_OK                        0xB1
#define LOG_TEST_FT_FAIL                      0xB2

// Specific DT Results
#define LOG_TEST_DT_OK                        0xC1
#define LOG_TEST_DT_FAIL                      0xC2


#endif // GLOBAL_DEF_H
