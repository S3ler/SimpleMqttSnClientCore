cmake_minimum_required(VERSION 3.7)
project(SimpleMqttSnClient)

set(CMAKE_CXX_STANDARD 11)


set(SOURCE_FILES
        SocketInterface.h
        global_defines.h
        mqttsn_messages.h
        MqttSnMessageHandler.cpp MqttSnMessageHandler.h SimpleMqttSnClient.cpp SimpleMqttSnClient.h)


add_library(lib-SimpleMqttSnClient ${SOURCE_FILES})

