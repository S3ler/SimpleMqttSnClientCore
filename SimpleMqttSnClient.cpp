//
// Created by bele on 11.12.17.
//


#include "SimpleMqttSnClient.h"

bool SimpleMqttSnClient::begin() {
    if (mqttSnMessageHandler == nullptr) {
        return false;
    }
    if (socketInterface == nullptr) {
        return false;
    }
    if (loggerInterface == nullptr) {
        return false;
    }
    if (!loggerInterface->begin()) {
        return false;
    }

    awaited_type = MQTTSN_PINGREQ;
    memset(&pingreq_result, 0x0, sizeof(pingreq_result));
    awaited_advertisment = nullptr;
    memset(&received_advertisments, 0x0, sizeof(received_advertisments));
    memset(&received_gwinfos, 0x0, sizeof(received_gwinfos));



    mqttSnMessageHandler->setSocket(socketInterface);
    mqttSnMessageHandler->setLogger(loggerInterface);
    mqttSnMessageHandler->setSimpleMqttSnClient(this);


    return mqttSnMessageHandler->begin();
}

void SimpleMqttSnClient::setSocketInterface(SocketInterface *socketInterface) {
    this->socketInterface = socketInterface;
}
void SimpleMqttSnClient::setLoggerInterface(LoggerInterface *loggerInterface) {
    this->loggerInterface = loggerInterface;
}

uint64_t SimpleMqttSnClient::ping_gateway(device_address *gateway) {
    return ping_gateway(gateway, 10000);
}

uint64_t SimpleMqttSnClient::ping_gateway(device_address *gateway, uint64_t timeout) {
    awaited_type = MQTTSN_PINGRESP;
    memset(&pingreq_result, 0x0, sizeof(pingreq_info));

    if (mqttSnMessageHandler->send_PingReq(gateway)) {
        int64_t start = millis();
        while ((millis() - start) < timeout) {
            if (!loop()) {
                break;
            }
            if (awaited_type != MQTTSN_PINGRESP) {
                return (uint64_t) (pingreq_result.received_timestamp - start);
            }
        }
    }

    if (awaited_type == MQTTSN_PINGRESP) {
        awaited_type = MQTTSN_PINGREQ;
    }
    return 0;
}

uint64_t SimpleMqttSnClient::get_advertisment_timestamp(device_address *gateway) {
    for (uint8_t i = 0; i < advertisment_info_buffer_size; i++) {
        advertise_info *info = &received_advertisments[i];
        if (memcmp(gateway, &info->address, sizeof(device_address)) == 0) {
            return info->received_timestamp;
        }
    }
    return 0;
}

device_address *SimpleMqttSnClient::await_advertise() {
    return await_advertise(10000);
}

device_address *SimpleMqttSnClient::await_advertise(uint64_t timeout) {
    awaited_type = MQTTSN_ADVERTISE;
    awaited_advertisment = nullptr;

    int64_t start = millis();
    while ((millis() - start) < timeout) {
        if (!loop()) {
            break;
        }
        if (awaited_type != MQTTSN_ADVERTISE) {
            // advertise received
            return &awaited_advertisment->address;
        }
    }
    if (awaited_type == MQTTSN_ADVERTISE) {
        awaited_type = MQTTSN_PINGREQ;
    }
    return &awaited_advertisment->address;
}

device_address *SimpleMqttSnClient::search_gateway() {
    return search_gateway(10000);
}

device_address *SimpleMqttSnClient::search_gateway(uint64_t searchtime) {
    awaited_type = MQTTSN_GWINFO;
    memset(&received_gwinfos, 0x0, sizeof(received_gwinfos));

    if (mqttSnMessageHandler->send_SearchGW()) {
        int64_t start = millis();
        while ((millis() - start) < searchtime) {
            if (!loop()) {
                break;
            }
        }
    }

    if (awaited_type != MQTTSN_GWINFO) {
        // find the one wit the highest RSSI and not empty
        int16_t max_rssi = INT16_MIN;
        gwinfo_info *result = nullptr;
        for (uint8_t i = 0; i < gwinfo_info_buffer_size; i++) {
            gwinfo_info *info = &received_gwinfos[i];
            if(info->gw_id != 0){
                if (info->rssi > max_rssi) {
                    result = info;
                    max_rssi = result->rssi;
                }
            }
        }
        return &result->address;
    }
    if (awaited_type == MQTTSN_GWINFO) {
        awaited_type = MQTTSN_PINGREQ;
    }
    return nullptr;
}

bool SimpleMqttSnClient::publish_m1(device_address *gateway, uint16_t predefined_topicId, bool retain, uint8_t *data,
                                    uint16_t data_length) {
    return mqttSnMessageHandler->send_Publish(gateway, predefined_topicId, retain, data, data_length);
}

bool SimpleMqttSnClient::loop() {
    // handle Advertisments only if we look for them
    // handle GatewayInfo only if we look for them
    return mqttSnMessageHandler->loop();
}

void SimpleMqttSnClient::setMqttSnMessageHandler(MqttSnMessageHandler *mqttSnMessageHandler) {
    this->mqttSnMessageHandler = mqttSnMessageHandler;
}

void
SimpleMqttSnClient::advertise_received(device_address *gateway, uint8_t gw_id, uint16_t duration, int16_t rssi) {
    uint64_t current_millis = millis();

    for (uint8_t i = 0; i < advertisment_info_buffer_size; i++) {
        advertise_info *info = &received_advertisments[i];
        if (memcmp(&info->address, gateway, sizeof(device_address)) == 0) {

            // update value
            info->received_timestamp = current_millis;
            info->duration = (uint64_t) duration * (uint64_t) 1000;
            info->rssi = rssi;

            if (awaited_type == MQTTSN_ADVERTISE) {
                awaited_advertisment = info;
                awaited_type = MQTTSN_PINGREQ;
            }

            return;
        }
    }

    for (uint8_t i = 0; i < advertisment_info_buffer_size; i++) {
        advertise_info *info = &received_advertisments[i];
        if (info->received_timestamp == 0) {

            // empty entry, insert
            info->received_timestamp = current_millis;
            memcpy(&info->address, gateway, sizeof(device_address));
            info->duration = (uint64_t) duration * (uint64_t) 1000;
            info->rssi = rssi;

            if (awaited_type == MQTTSN_ADVERTISE) {
                awaited_advertisment = info;
                awaited_type = MQTTSN_PINGREQ;
            }
            return;
        }
    }

    // awaited advertisment buffer full
    // find entry with minimum rssi
    uint8_t min_index = gwinfo_info_buffer_size;
    uint64_t min_received_timestamp = INT16_MAX;

    for (uint8_t i = 0; i < advertisment_info_buffer_size; i++) {
        advertise_info *info = &received_advertisments[i];
        if (min_received_timestamp < info->received_timestamp) {
            min_index = i;
            min_received_timestamp = info->received_timestamp;
        }
    }

    advertise_info *info = &received_advertisments[min_index];
    info->received_timestamp = current_millis;
    memcpy(&info->address, gateway, sizeof(device_address));
    info->duration = (uint64_t) duration * (uint64_t) 1000;
    info->rssi = rssi;

    if (awaited_type == MQTTSN_ADVERTISE) {
        awaited_advertisment = info;
        awaited_type = MQTTSN_PINGREQ;
    }

}

void SimpleMqttSnClient::gwinfo_received(device_address *gateway, uint8_t gw_id, int16_t rssi) {
    insert_into_received_gwinfos(gateway, gw_id, rssi);
    if(awaited_type == MQTTSN_GWINFO){
        awaited_type = MQTTSN_PINGREQ;
    }
}

void
SimpleMqttSnClient::gwinfo_received(device_address *source, uint8_t gw_id, device_address *gw_address, int16_t rssi) {
    if (gw_id == 0) {
        // we do not accept gw_id which are 0
        return;
    }

    if (memcmp(source, gw_address, sizeof(device_address)) == 0) {
        insert_into_received_gwinfos(source, gw_id, rssi);
    } else {
        // source and msg address do not match
        // we ignore these packets at the moment
    }
    if(awaited_type == MQTTSN_GWINFO){
        awaited_type = MQTTSN_PINGREQ;
    }
}

void SimpleMqttSnClient::insert_into_received_gwinfos(device_address *gateway, uint8_t gw_id, int16_t rssi) {

    uint64_t current_millis = millis();
    for (uint8_t i = 0; i < gwinfo_info_buffer_size; i++) {
        gwinfo_info *info = &received_gwinfos[i];
        if (memcmp(gateway, &info->address, sizeof(device_address)) == 0) {
            // update values
            info->received_timestamp = current_millis;
            info->gw_id = gw_id;
            info->rssi = rssi;
            return;
        }
    }

    for (uint8_t i = 0; i < gwinfo_info_buffer_size; i++) {
        gwinfo_info *info = &received_gwinfos[i];
        if (info->gw_id == 0) {
            // empty entry, insert
            info->received_timestamp = current_millis;
            memcpy(&info->address, gateway, sizeof(device_address));
            info->gw_id = gw_id;
            info->rssi = rssi;
            return;
        }
    }

    // gwinfos buffer full
    // find entry with minimum rssi
    uint8_t min_index = 0;
    int16_t min_rssi = INT16_MAX;
    for (uint8_t i = 0; i < gwinfo_info_buffer_size; i++) {
        gwinfo_info *info = &received_gwinfos[i];
        if (min_rssi < info->rssi) {
            min_rssi = info->rssi;
            min_index = i;
        }
    }

    gwinfo_info *info = &received_gwinfos[min_index];
    // overwrite entry with least rssi
    info->received_timestamp = current_millis;
    memcpy(&info->address, gateway, sizeof(device_address));
    info->gw_id = gw_id;
    info->rssi = rssi;
}

void SimpleMqttSnClient::pingresp_received(device_address *source, int16_t rssi) {
    if (awaited_type == MQTTSN_PINGRESP) {
        uint64_t current_millis = millis();
        pingreq_result.received_timestamp = current_millis;
        pingreq_result.rssi = rssi;
        awaited_type = MQTTSN_PINGREQ;
    }
}










