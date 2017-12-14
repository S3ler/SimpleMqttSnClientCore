cmake_minimum_required(VERSION 3.7)
project(SimpleMqttSnClient)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory(arduino-linux-abstraction)
include_directories(arduino-linux-abstraction/src)
set(SOURCE_FILES
        MqttSnMessageHandler.h
        MqttSnMessageHandler.cpp
        SocketInterface.h
        global_defines.h
        mqttsn_messages.h
        MqttSnMessageHandler.cpp MqttSnMessageHandler.h SimpleMqttSnClient.cpp SimpleMqttSnClient.h
        LoggerInterface.h
        )


add_library(lib-SimpleMqttSnClient ${SOURCE_FILES})
target_link_libraries(lib-SimpleMqttSnClient arduino-linux-abstraction)

