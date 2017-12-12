//
// Created by bele on 11.12.17.
//

#include "MqttSnMessageHandler.h"


bool MqttSnMessageHandler::begin() {
    if(socketInterface == nullptr){
        return false;
    }
    if(logger == nullptr){
        return false;
    }
    if(!logger->begin()){
        return false;
    }
    return this->socketInterface->begin();
}

void MqttSnMessageHandler::setSocket(SocketInterface *socketInterface) {
    this->socketInterface = socketInterface;
}

void MqttSnMessageHandler::setLogger(LoggerInterface *logger) {
    this->logger = logger;
}

void MqttSnMessageHandler::setSimpleMqttSnClient(SimpleMqttSnClient *simpleMqttSnClient) {
    this->simpleMqttSnClient = simpleMqttSnClient;
}

void MqttSnMessageHandler::receiveData(device_address *address, uint8_t *bytes) {
    message_header *header = (message_header *) bytes;
    if (header->length < 2) {
        return;
    }
    switch (header->type) {

        case MQTTSN_ADVERTISE:
            parse_advertise(address, bytes);
            break;
        case MQTTSN_PINGREQ:
            parse_pingreq(address, bytes);
            break;
        case MQTTSN_PINGRESP:
            parse_pingresp(address, bytes);
            break;
        case MQTTSN_GWINFO:
            parse_gwinfo(address, bytes, 0);
            break;
        default:
            break;
    }
}

bool MqttSnMessageHandler::loop() {
    return socketInterface->loop();
}

bool MqttSnMessageHandler::send_PingReq(device_address *destination) {
    msg_pingreq msg;
    msg.init_msg_pingreq(&msg, nullptr);
    return socketInterface->send(destination, (uint8_t *) &msg, msg.length);
}

bool MqttSnMessageHandler::send_SearchGW() {
    msg_searchgw msg = msg_searchgw(UINT8_MAX);
    return socketInterface->send(socketInterface->getBroadcastAddress(), (uint8_t *) &msg, msg.length);
}

bool MqttSnMessageHandler::send_Publish(device_address *destination, uint16_t predefined_topicId, bool retain,
                                        uint8_t *payload,
                                        uint16_t payload_length) {
    if (payload_length > 255) {
        return false;
    }
    msg_publish msg = msg_publish(false, -1, retain, false, predefined_topicId,
                                  0x01, payload, (uint8_t) payload_length);
    return socketInterface->send(destination, (uint8_t *) &msg, msg.length);
}

void MqttSnMessageHandler::parse_advertise(device_address *source, uint8_t *bytes) {
    msg_advertise *msg = (msg_advertise *) bytes;
    if (msg->type == MQTTSN_ADVERTISE && msg->length == 5) {
        handle_advertise(source, msg->gw_id, msg->duration, socketInterface->getLastMsgRssi());
    }
}

void MqttSnMessageHandler::handle_advertise(device_address *source, uint8_t gw_id, uint16_t duration, int16_t rssi) {
    simpleMqttSnClient->advertise_received(source, gw_id, duration, rssi);
}

void MqttSnMessageHandler::parse_pingreq(device_address *source, uint8_t *bytes) {
    msg_pingreq *msg = (msg_pingreq *) bytes;
    if(msg->type == MQTTSN_PINGREQ && msg->length != 2){
        send_Pingresp(source);
    }
}

void MqttSnMessageHandler::send_Pingresp(device_address *destination) {
    message_header msg;
    msg.to_pingresp();
    socketInterface->send(destination, msg, msg.length);
}

void MqttSnMessageHandler::parse_pingresp(device_address *source, uint8_t *bytes) {
    message_header *msg = (message_header *) bytes;
    if (msg->type == MQTTSN_PINGRESP && msg->length == 2) {
        handle_pingresp(source, socketInterface->getLastMsgRssi());
    }
}

void MqttSnMessageHandler::parse_gwinfo(device_address *source, uint8_t *bytes, int16_t rssi) {
    msg_gwinfo *msg = (msg_gwinfo *) bytes;
    if (msg->type == MQTTSN_GWINFO && msg->length == 3) {
        handle_gwinfo(source, msg->gw_id, rssi);
    } else if (msg->type == MQTTSN_GWINFO && msg->length > 3 && msg->length == 3 + sizeof(device_address)) {
        device_address *gw_address = (device_address *) msg->gw_address;
        handle_gwinfo(source, msg->gw_id, gw_address, rssi);
    }
}

void MqttSnMessageHandler::handle_gwinfo(device_address *source, uint8_t gw_id, int16_t rssi) {
    simpleMqttSnClient->gwinfo_received(source, gw_id, rssi);
}

void
MqttSnMessageHandler::handle_gwinfo(device_address *source, uint8_t gw_id, device_address *gw_address, int16_t rssi) {
    simpleMqttSnClient->gwinfo_received(source, gw_id, gw_address, rssi);
}

void MqttSnMessageHandler::handle_pingresp(device_address *source, int16_t rssi) {
    simpleMqttSnClient->pingresp_received(source, rssi);
}


