/*
* IoT Hub Raspberry Pi C - Microsoft Sample Code - Copyright (c) 2017 - Licensed MIT
*/

#ifndef CONFIG_H_
#define CONFIG_H_

#define INTERVAL 1000
#define SIMULATED_DATA 0
#define BUFFER_SIZE 256
#define TEMPERATURE_ALERT 30

#define LED_PIN 7

#define CREDENTIAL_PATH "~/.iot-hub"

#include <stdbool.h>

typedef struct DPS_CLIENT_CONTEXT_TAG
{
    unsigned int sleep_time;
    char* iothub_uri;
    char* access_key_name;
    char* device_key;
    char* device_id;
    bool registration_complete;
} DPS_CLIENT_CONTEXT;

typedef struct IOTHUB_CLIENT_CONTEXT_TAG
{
    int connected;
    int stop_loop;
} IOTHUB_CLIENT_CONTEXT;

#endif  // CONFIG_H_
