#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <mosquittopp.h>
#include "IoTDeviceManager.h"
#include <iostream>
#include <sstream>
#include <exception>

class MQTTHandler : public mosqpp::mosquittopp {
private:
    IoTDeviceManager &deviceManager;
    int reconnect_attempts = 0;

    std::string broker;
    int port;
    int keepalive;
    std::string client_id;
    std::string topic_subscribe;
    std::string topic_response;
    std::string username;
    std::string password;
    
    void sendResponse(const std::string &cmd, const std::string &rqi, int code);
    void loadConfigFromFile(const std::string &filename);
public:
    MQTTHandler(IoTDeviceManager &manager, const std::string &configFile);
    void on_connect(int rc) override;
    void on_message(const struct mosquitto_message *message) override;
    void reconnect();
    void handleAddDevice(const Json::Value &root);
    // void handleSyncDevice(const Json::Value &root);
    void handleDeleteDevice(const Json::Value &root);
    void handleLightControl(const Json::Value &root);
    void handleTouchSwitchControl(const Json::Value &root);
    void handleSensorValue(const Json::Value &root);
    void sendControl(const std::string& deviceId, uint8_t *onoff, uint8_t *dim, uint8_t *ctt, uint8_t *h, uint8_t *l, uint8_t *s);

};

#endif
