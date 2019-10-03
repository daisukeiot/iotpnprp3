#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required(VERSION 2.8.11)

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
  set (CMAKE_C_FLAGS "--std=c99 -g ${CMAKE_C_FLAGS}")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_PROV_MODULE")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_PROV_MODULE")

include_directories("/usr/local/include/azureiot"
                    "/usr/local/include/azureiot/inc/")

file(GLOB nonpnp-app_h_files
    "*.h"
)

file(GLOB nonpnp-app_c_files
    "*.c"
)

add_executable(nonpnp-app ${nonpnp-app_c_files} ${nonpnp-app_h_files})

target_link_libraries(nonpnp-app
                      parson
                      wiringPi
                      serializer
                      iothub_client_mqtt_transport
                      iothub_client
                      umqtt
                      aziotsharedutil
                      prov_mqtt_transport
                      umqtt
                      ssl
                      crypto
                      curl
                      pthread
                      m
                      crypto
                      prov_device_ll_client
                      prov_auth_client
                      hsm_security_client
                      msr_riot
                      utpm
                      dl
                      )

