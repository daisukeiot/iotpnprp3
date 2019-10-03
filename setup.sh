#!/bin/bash

sh -c '
sudo apt-get update
sudo apt-get purge -y libssl-dev
sudo apt-get install -y libssl1.0-dev git cmake build-essential curl libcurl4-openssl-dev uuid-dev

if [ ! -d "$PWD/azure-iot-sdk-c" ]; then
    git clone --recursive https://github.com/azure/azure-iot-sdk-c.git --recursive -b public-preview
fi

if [ -d "$PWD/azure-iot-sdk-c" ]; then
    cd $PWD/azure-iot-sdk-c/build_all/linux
    ./build.sh --no-make --provisioning --no-amqp --no-http
    cd ../../cmake/iotsdk_linux
    make
    sudo make install
    sudo cp $PWD/provisioning_client/deps/libmsr_riot.a /usr/local/lib
    sudo cp $PWD/provisioning_client/deps/utpm/libutpm.a /usr/local/lib
    echo "************************************************"
    echo "Azure Device SDK built and installed"
    echo "Continue to the next hands on lab"
    echo "************************************************"
else
    echo "SDK does not exist"
fi
'
