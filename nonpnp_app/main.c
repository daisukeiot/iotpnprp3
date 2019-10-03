#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include <iothub.h>
#include <iothub_message.h>
#include <iothub_client.h>
#include <iothub_client_version.h>
#include <iothub_device_client_ll.h>
#include <azure_prov_client/prov_device_ll_client.h>
#include <azure_prov_client/prov_security_factory.h>
#include <azure_c_shared_utility/xlogging.h>
#include <iothubtransportmqtt.h>
#include <azure_prov_client/prov_transport_mqtt_client.h>
#include <azure_c_shared_utility/threadapi.h>

#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/crt_abstractions.h>
// #include <jsondecoder.h>
#include <pthread.h>
#include "./config.h"
#include "grove_bme280.h"

static const char* global_prov_uri = "global.azure-devices-provisioning.net";

//
// From IoT Central's Device Connection
//
static const char* id_scope = "[ID Scope]";
static const char* saskey = "[Shared Access Signature (SAS)]";
static const char* registration_id = "[Registration ID]";

static int interval = INTERVAL;

MU_DEFINE_ENUM_STRINGS(PROV_DEVICE_RESULT, PROV_DEVICE_RESULT_VALUE);
MU_DEFINE_ENUM_STRINGS(PROV_DEVICE_REG_STATUS, PROV_DEVICE_REG_STATUS_VALUES);

/*****************************************************************************************************
* D2C Message
*****************************************************************************************************/ 

int getTelemetry(int fd, char *payload)
{
    float temperature;

    temperature = read_temperature(fd);

    snprintf(payload,
             BUFFER_SIZE,
             "{ \"deviceId\": \"%s\", \"temperature\": %5.2f}",
             registration_id,
             temperature);

    return temperature > TEMPERATURE_ALERT ? 1 : 0;
}

static void sendCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void *userContextCallback)
{
    if (IOTHUB_CLIENT_CONFIRMATION_OK == result)
    {
        LogInfo("Message sent successfully to Azure IoT Hub");
    }
    else
    {
        LogError("Failed to send message to Azure IoT Hub");
    }
}

static void sendMessage(IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle, char *buffer, int temperatureAlert)
{
    IOTHUB_MESSAGE_HANDLE messageHandle = IoTHubMessage_CreateFromByteArray(buffer, strlen(buffer));
    if (messageHandle == NULL)
    {
        LogError("Unable to create a new IoTHubMessage");
    }
    else
    {
        MAP_HANDLE properties = IoTHubMessage_Properties(messageHandle);
        Map_Add(properties, "temperatureAlert", (temperatureAlert > 0) ? "true" : "false");
        LogInfo("Sending message: %s", buffer);
        if (IoTHubDeviceClient_LL_SendEventAsync(iotHubClientHandle, messageHandle, sendCallback, NULL) != IOTHUB_CLIENT_OK)
        {
            LogError("Failed to send message to Azure IoT Hub");
        }

        IoTHubMessage_Destroy(messageHandle);
    }
}

/*****************************************************************************************************
* Callbacks for IoT Hub
*****************************************************************************************************/ 
static void iothub_connection_status(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void* user_context)
{

    LogInfo("iothub_connection_status Result %d Reason %d", result, reason);

    if (user_context == NULL)
    {
        LogError("iothub_connection_status user_context is NULL");
    }
    else
    {
        IOTHUB_CLIENT_CONTEXT* iothub_ctx = (IOTHUB_CLIENT_CONTEXT*)user_context;
        if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED)
        {
            iothub_ctx->connected = 1;
        }
        else
        {
            iothub_ctx->connected = 0;
            iothub_ctx->stop_loop = 1;
        }
    }
}

/*****************************************************************************************************
* Callbacks for DPS
*****************************************************************************************************/ 
static void dps_registration_status_callback(PROV_DEVICE_REG_STATUS reg_status, void* user_context)
{
    LogInfo("Provisioning Status: %s", MU_ENUM_TO_STRING(PROV_DEVICE_REG_STATUS, reg_status));
}

static void dps_register_device_callback(PROV_DEVICE_RESULT register_result, const char* iothub_uri, const char* device_id, void* user_context)
{
    if (user_context == NULL)
    {
        LogInfo("user_context is NULL");
    }
    else
    {
        DPS_CLIENT_CONTEXT* user_ctx = (DPS_CLIENT_CONTEXT*)user_context;
        if (register_result == PROV_DEVICE_RESULT_OK)
        {
            LogInfo("Registration Information received from service: %s!", iothub_uri);
            (void)mallocAndStrcpy_s(&user_ctx->iothub_uri, iothub_uri);
            (void)mallocAndStrcpy_s(&user_ctx->device_id, device_id);
            LogInfo("IoT Hub   %s", user_ctx->iothub_uri);
            LogInfo("Device Id %s", user_ctx->device_id);

            user_ctx->registration_complete = 1;
        }
        else
        {
            LogError("Failure encountered on registration %s", MU_ENUM_TO_STRING(PROV_DEVICE_RESULT, register_result) );
            user_ctx->registration_complete = 0;
        }
    }
}

int main(int argc, char *argv[])
{
    DPS_CLIENT_CONTEXT dps_ctx;
    IOTHUB_CLIENT_CONTEXT iothub_ctx;

    memset(&dps_ctx, 0, sizeof(DPS_CLIENT_CONTEXT));
    dps_ctx.registration_complete = FALSE;
    dps_ctx.sleep_time = 10;

    memset(&iothub_ctx, 0, sizeof(IOTHUB_CLIENT_CONTEXT));

    IOTHUB_DEVICE_CLIENT_LL_HANDLE device_ll_handle;

    LogInfo("Provisioning API Version: %s", Prov_Device_LL_GetVersionString());
    LogInfo("Iothub API Version: %s", IoTHubClient_GetVersionString());

    /***************************************************************************************************
     * Provision the device using DPS
    ***************************************************************************************************/
    PROV_DEVICE_LL_HANDLE hDps;

    if (IoTHub_Init() != 0)
    {
        LogError("Failed to initialize the platform.");
        return 1;
    }

    prov_dev_security_init(SECURE_DEVICE_TYPE_SYMMETRIC_KEY);
    prov_dev_set_symmetric_key_info(registration_id, saskey);

    // Create handle for DPS
    if ((hDps = Prov_Device_LL_Create(global_prov_uri, id_scope, Prov_Device_MQTT_Protocol)) == NULL)
    {
        LogError("failed calling Prov_Device_LL_Create");
    }
    else
    {
        bool traceOn = false;
        Prov_Device_LL_SetOption(hDps, PROV_OPTION_LOG_TRACE, &traceOn);

        // Start Device Registration
        if (Prov_Device_LL_Register_Device(hDps, dps_register_device_callback, &dps_ctx, dps_registration_status_callback, &dps_ctx) != PROV_DEVICE_RESULT_OK)
        {
            LogError("failed calling Prov_Device_LL_Register_Device");
        }
        else
        {
            do
            {
                Prov_Device_LL_DoWork(hDps);
                ThreadAPI_Sleep(dps_ctx.sleep_time);
            } while (dps_ctx.registration_complete == 0);
        }
        Prov_Device_LL_Destroy(hDps);
    }

    /***************************************************************************************************
     * Connect to IoT Hub
    ***************************************************************************************************/
    if (dps_ctx.registration_complete != 1)
    {
        LogInfo("registration failed!");
    }
    else
    {
        if ((device_ll_handle = IoTHubDeviceClient_LL_CreateFromDeviceAuth(dps_ctx.iothub_uri, dps_ctx.device_id, MQTT_Protocol) ) == NULL)
        {
            LogError("failed create IoTHub client from connection string %s!", dps_ctx.iothub_uri);
        }
        else
        {
            (void)IoTHubDeviceClient_LL_SetConnectionStatusCallback(device_ll_handle, iothub_connection_status, &iothub_ctx);
            // (void)IoTHubClient_LL_SetMessageCallback(device_ll_handle, receiveMessageCallback, NULL);
            // (void)IoTHubClient_LL_SetDeviceMethodCallback(device_ll_handle, deviceMethodCallback, NULL);
            // (void)IoTHubClient_LL_SetDeviceTwinCallback(device_ll_handle, twinCallback, NULL);
            (void)IoTHubClient_LL_SetOption(device_ll_handle, "product_info", "IoT_PnP_HOL");
        }
    }

    /***************************************************************************************************
     * Send Telemetry
    ***************************************************************************************************/

    if (dps_ctx.registration_complete == 1)
    {
        int sensor = initialize_sensor();

        while (true)
        {
            if (iothub_ctx.connected == 1)
            {
                char buffer[BUFFER_SIZE];
                int result = getTelemetry(sensor, buffer);
                if (result != -1)
                {
                    LogInfo("%s", buffer);
                    sendMessage(device_ll_handle, buffer, result);
                }
                else
                {
                    LogError("Failed to read message");
                }
            }
            IoTHubClient_LL_DoWork(device_ll_handle);
            ThreadAPI_Sleep(interval);
        }

        IoTHubClient_LL_Destroy(device_ll_handle);
    }

    free(dps_ctx.iothub_uri);
    free(dps_ctx.device_id);
    prov_dev_security_deinit();

    // Free all the sdk subsystem
    IoTHub_Deinit();

    return 0;
}
