//
// Created by bele on 11.12.17.
//

#ifndef RASPBERRYPISIMPLEMQTTSNCLIENT_MQTTSNMESSAGEHANDLER_H
#define RASPBERRYPISIMPLEMQTTSNCLIENT_MQTTSNMESSAGEHANDLER_H


#include "SocketInterface.h"
#include "mqttsn_messages.h"
#include "SimpleMqttSnClient.h"

class SocketInterface;
class SimpleMqttSnClient;

class MqttSnMessageHandler {
public:

    bool begin();

    void setLogger(LoggerInterface *logger);
    void setSocket(SocketInterface *socketInterface);

    void setSimpleMqttSnClient(SimpleMqttSnClient *simpleMqttSnClient);
    void receiveData(device_address *address, uint8_t *bytes);

    bool loop();


    bool send_PingReq(device_address *destination);

    bool send_SearchGW();

    bool send_Publish(device_address *destination, uint16_t predefined_topicId, bool retain, uint8_t *payload, uint16_t payload_length);

    void send_Pingresp(device_address *destination);

    void parse_advertise(device_address *source, uint8_t *bytes);

    void parse_pingreq(device_address *source, uint8_t *bytes);

    void parse_pingresp(device_address *source, uint8_t *bytes);

    void parse_gwinfo(device_address *source, uint8_t *bytes, int16_t rssi);

    void handle_advertise(device_address *source, uint8_t gw_id, uint16_t duration, int16_t rssi);

    void handle_gwinfo(device_address *source, uint8_t gw_id, int16_t rssi);

    void handle_gwinfo(device_address *source, uint8_t gw_id, device_address *gw_address, int16_t rssi);

    void handle_pingresp(device_address *source, int16_t rssi);

    SocketInterface *socketInterface;

    SimpleMqttSnClient *simpleMqttSnClient;



    LoggerInterface *logger;
};


#endif //RASPBERRYPISIMPLEMQTTSNCLIENT_MQTTSNMESSAGEHANDLER_H
