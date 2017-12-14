//
// Created by bele on 11.12.17.
//

#ifndef RASPBERRYPISIMPLEMQTTSNCLIENT_SIMPLEMQTTSNCLIENT_H
#define RASPBERRYPISIMPLEMQTTSNCLIENT_SIMPLEMQTTSNCLIENT_H

#include <cstring>
#include <stdint-gcc.h>
#include "LoggerInterface.h"
#include "MqttSnMessageHandler.h"
#include "global_defines.h"
#include "SocketInterface.h"
#include "mqttsn_messages.h"
#include <arduino-linux-abstraction/src/Arduino.h>
#include <wiringPi.h>

#define advertisment_info_buffer_size 1
#define gwinfo_info_buffer_size 5

class SocketInterface;
class MqttSnMessageHandler;

struct pingreq_info {
    uint64_t received_timestamp = 0;
    int16_t rssi = 0;
};

struct advertise_info {
    uint64_t received_timestamp = 0;
    device_address address;
    uint64_t duration = 0;
    int16_t rssi = 0;
};

struct gwinfo_info {
    uint64_t received_timestamp = 0;
    device_address address;
    uint8_t gw_id = 0;
    int16_t rssi = 0;
};

class SimpleMqttSnClient {

public:
    /**
     * Initializes the SimpleMqttSnClient.
     * @return true if all components were initialized sucessfully else false.
     */
    bool begin();

    /**
     * Sets the SocketInterface used by the SimpleMqttSnClient.
     * @param socketInterface to set.
     */
    void setSocketInterface(SocketInterface *socketInterface);

    /**
    * Sets the LoggerInterface used by the SimpleMqttSnClient.
    * @param loggerInterface to set.
    */
    void setLoggerInterface(LoggerInterface *loggerInterface);

    /**
     * Sets the MqttSnMessageHandler to be used.
     * @param mqttSnMessageHandler to set.
     */
    void setMqttSnMessageHandler(MqttSnMessageHandler *mqttSnMessageHandler);
    /**
     * Check the Round Trip Time between sending a PingReq and receiving a PingResp.
     * Uses the default timeout of 10 seconds.
     * @param gateway is the device_addres to send to
     * @return the time in milliseconds until receiving the PingResp packet. Returns 0 on timeout.
     */
    uint64_t ping_gateway(device_address *gateway);

    /**
     * Check the Round Trip Time between sending a PingReq and receiving a PingResp.
     * Uses the given timeout.
     * @param gateway is the device_addres to send to
     * @param timeout value waiting for the PingResp packet.
     * @return the time in milliseconds until receiving the PingResp packet. Returns 0 on timeout.
     */
    uint64_t ping_gateway(device_address *gateway, uint64_t timeout);


    /**
     * Gives the millis when the last advertisment packet of the given gateway was received
     * @param gateway to get the value from.
     * @return the time in milliseconds. If the gateway is unknown returns 0.
     */
    uint64_t get_advertisment_timestamp(device_address *gateway);


    /**
     * Awaits a Advertisment and returns the send device_address.
     * Uses the default timeout of 10 seconds.
     * @return the source address of the Advertisment packet. Returns nullptr on timeout
     */
    device_address *await_advertise();

    /**
    * Awaits a Advertisment and returns the send device_address.
    * Uses the given timeout.
    * @param timeout value waiting for a Advertisment packet.
    * @return the source address of the Advertisment packet. Returns nullptr on timeout
    */
    device_address *await_advertise(uint64_t timeout);

    /**
     * Searches a Gateway and returns the Gateway with the highest RSSI.
     * Uses the default searchtime of 10 seconds and searches so long for the Gateway.
     * @return the device_address of the gateway with the highest RSSI received during the search. Returns nullptr when no Gateway was found.
     */
    device_address *search_gateway();

    /**
     * Searches a Gateway and returns the Gateway with the highest RSSI.
     * Uses the default searchtime of 10 seconds.
     * @param searchtime in milliseconds how long the SimpleMqttSnClient searches for Gateways.
     * @return the device_address of the gateway with the highest RSSI received during the search. Returns nullptr when no Gateway was found.
     */
    device_address *search_gateway(uint64_t searchtime);


    bool
    publish_m1(device_address *gateway, uint16_t predefined_topicId, bool retain, uint8_t *data, uint16_t data_length);

    /**
     * Loops through the SimpleMqttSnClient components.
     * @return true if everything is ok, false if an error occurs.
     */
    bool loop();

private:
    SocketInterface *socketInterface = nullptr;
    LoggerInterface *loggerInterface = nullptr;
    MqttSnMessageHandler* mqttSnMessageHandler = nullptr;

    message_type awaited_type = MQTTSN_PINGREQ;

    pingreq_info pingreq_result;

    advertise_info *awaited_advertisment = nullptr;

    advertise_info received_advertisments[advertisment_info_buffer_size];

    gwinfo_info received_gwinfos[gwinfo_info_buffer_size];

public:
    void advertise_received(device_address *gateway, uint8_t gw_id, uint16_t duration, int16_t rssi);

    void gwinfo_received(device_address *gateway, uint8_t gw_id, int16_t rssi);

    void gwinfo_received(device_address *source, uint8_t gw_id, device_address *gw_address, int16_t rssi);

    void insert_into_received_gwinfos(device_address *gateway, uint8_t gw_id, int16_t rssi);

    void pingresp_received(device_address *source, int16_t rssi);

};


#endif //RASPBERRYPISIMPLEMQTTSNCLIENT_SIMPLEMQTTSNCLIENT_H
