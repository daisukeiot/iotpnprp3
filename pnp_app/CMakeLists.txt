#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

#this is CMakeLists.txt for pnp-app

cmake_minimum_required(VERSION 2.8.11)

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
  set (CMAKE_C_FLAGS "--std=c99 -g ${CMAKE_C_FLAGS}")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_PROV_MODULE")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_PROV_MODULE")

include_directories("/usr/local/include/azureiot"
                    "/usr/local/include/azureiot/inc/")

file(GLOB pnp-app_h_files
    "*.h"
    "./utilities/*.h"
)

file(GLOB pnp-app_c_files
    "*.c"
    "./utilities/*.c"
)

include_directories(.)
include_directories(./utilities)

add_executable(pnp-app ${pnp-app_c_files} ${pnp-app_h_files})

target_link_libraries(pnp-app wiringPi
                          serializer
                          iothub_client_mqtt_transport
                          iothub_client
                          digitaltwin_client
                          umqtt
                          aziotsharedutil
                          prov_mqtt_transport
                          umqtt
                          ssl
                          crypto
                          curl
                          pthread
                          m
                          prov_device_ll_client
                          prov_auth_client
                          hsm_security_client
                          msr_riot
                          utpm
                          dl)