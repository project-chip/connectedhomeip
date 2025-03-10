#include "MQTTHandler.h"
#include <jsoncpp/json/json.h> 
#include <iostream>
#include <thread>
#include <chrono>


MQTTHandler::MQTTHandler(IoTDeviceManager &manager, const std::string &configFile) 
    : mosquittopp(), deviceManager(manager) {
    
    // Load cấu hình từ file JSON
    loadConfigFromFile(configFile);
    
    // Đặt ID cho client MQTT
    this->reinitialise(client_id.c_str(), true);

    // Đặt username và password
    username_pw_set(username.c_str(), password.c_str());
    // Kết nối MQTT
    connect(broker.c_str(), port, keepalive);
}

void MQTTHandler::loadConfigFromFile(const std::string &filename) {
   
    std::ifstream file(filename);
    if (!file.is_open()) {
        // throw std::runtime_error("Could not open MQTT config file.");
    }

    Json::Value config;
    file >> config;

    broker = config["broker"].asString();
    port = config["port"].asInt();
    keepalive = config["keepalive"].asInt();
    client_id = config["client_id"].asString();
    topic_subscribe = config["topic_subscribe"].asString();
    topic_response = config["topic_response"].asString();
    username = config["username"].asString();
    password = config["password"].asString();
    std::cout << "Loaded MQTT config: " << broker << ":" << port << std::endl;

}

void MQTTHandler::reconnect() {
    while (reconnect_attempts < 5) {
        std::cerr << "Attempting to reconnect (" << reconnect_attempts + 1 << "/5)..." << std::endl;
        int ret = connect(broker.c_str(), port, keepalive);
        if (ret == MOSQ_ERR_SUCCESS) {
            std::cout << "Reconnected successfully!" << std::endl;
            return;
        }
        reconnect_attempts++;
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    std::cerr << "Failed to reconnect after 5 attempts. Exiting..." << std::endl;
}


void MQTTHandler::on_connect(int rc) {
    if (rc == 0) {
        std::cout << "Connected to MQTT broker!" << std::endl;
        subscribe(NULL, topic_subscribe.c_str());
    } else {
        std::cerr << "Failed to connect, return code: " << rc << std::endl;
    }
}

void MQTTHandler::sendResponse(const std::string &cmd, const std::string &rqi, int code) {

    Json::Value response;
    response["cmd"] = cmd;
    response["rqi"] = rqi;
    response["data"]["code"] = code;

    Json::StreamWriterBuilder writer;
    std::string responseStr = Json::writeString(writer, response);

    publish(NULL, topic_response.c_str(), (unsigned int)responseStr.length(), responseStr.c_str());
 
}

void MQTTHandler::handleAddDevice(const Json::Value &root) {
    IoTDevice newDevice = IoTDevice::fromJson(root["data"]);
    sendResponse(root["cmd"].asString(), root["rqi"].asString(), 0);
    if(deviceManager.getDevices().isMember(newDevice.id)) return;
    // IoTDevice newDevice = IoTDevice::fromJson(root["data"]);
    deviceManager.addDevice(newDevice);
    // sendResponse(root["cmd"].asString(), root["rqi"].asString(), 0);
}

// void MQTTHandler::handleSyncDevice(const Json::Value &root) {
//     IoTDevice newDevice = IoTDevice::fromJson(root["data"]);
//     sendResponse(root["cmd"].asString(), root["rqi"].asString(), 0);
//     if(deviceManager.getDevices().isMember(newDevice.id)) return;
//     deviceManager.addDevice(newDevice);
//     sendResponse(root["cmd"].asString(), root["rqi"].asString(), 0);
// }

void MQTTHandler::handleDeleteDevice(const Json::Value &root) {
    bool success = deviceManager.removeDevice(root["data"]["id"].asString());
    sendResponse(root["cmd"].asString(), root["rqi"].asString(), success ? 0 : 1);
}

void MQTTHandler::handleLightControl(const Json::Value &root) {

    std::string id = root["data"]["id"].asString();
    Json::Value state = root["data"]["data"];  // Lấy dữ liệu điều khiển từ "data.data"
    bool success = deviceManager.updateDeviceStateById(id, state);
    sendResponse(root["cmd"].asString(), root["rqi"].asString(), success ? 0 : 1);
 
}

void MQTTHandler::handleTouchSwitchControl(const Json::Value &root) {
    
    Json::Value state;
    state["onoff"] = root["data"]["onoff"];
    bool success = deviceManager.updateDeviceStateById(root["data"]["id"].asString(), state);
    sendResponse(root["cmd"].asString(), root["rqi"].asString(), success ? 0 : 1);

}

void MQTTHandler::handleSensorValue(const Json::Value &root) {
   
    Json::Value state;
    state["value"] = root["data"]["value"];
    bool success = deviceManager.updateDeviceStateById(root["data"]["id"].asString(), state);
    sendResponse(root["cmd"].asString(), root["rqi"].asString(), success ? 0 : 1);
 
}


void MQTTHandler::sendControl(const std::string& deviceId, uint8_t *onoff, uint8_t *dim, uint8_t *ctt, uint8_t *h, uint8_t *l, uint8_t *s) {
    Json::Value root;
    root["cmd"] = "controlDev";
    root["rqi"] = "12345678";

    Json::Value data;
    data["id"] = deviceId;

    Json::Value deviceData;
    if(onoff != nullptr) deviceData["onoff"] = Json::Value(*onoff);
    if(dim != nullptr) deviceData["dim"] = *dim;
    if(ctt != nullptr) deviceData["ctt"] = *ctt;
    if(h != nullptr) deviceData["h"] = *h;
    if(l != nullptr) deviceData["l"] = *l;
    if(s != nullptr) deviceData["s"] = *s;

    data["data"] = deviceData;
    root["data"] = data;

    Json::StreamWriterBuilder writer;
    std::string controlStr = Json::writeString(writer, root);

    publish(NULL, topic_response.c_str(), (unsigned int)controlStr.length(), controlStr.c_str());
}

void MQTTHandler::on_message(const struct mosquitto_message *message) {
   
    Json::Value root;
    std::string payload(static_cast<char*>(message->payload), message->payloadlen);
    Json::CharReaderBuilder reader;
    std::istringstream s(payload);
    std::string errs;

    std::cout << "Received message: " << payload << std::endl;

    if (Json::parseFromStream(reader, s, &root, &errs)) {
        std::string cmd = root["cmd"].asString();
        if (cmd == "add_device") handleAddDevice(root);
        else if (cmd == "del_device") handleDeleteDevice(root);
        else if (cmd == "controlDev") handleLightControl(root);
        else if (cmd == "touch_switch_ctrl") handleTouchSwitchControl(root);
        else if (cmd == "sensor_value") handleSensorValue(root);
        else if (cmd == "reset") deviceManager.clearDevices();
    } else {
        std::cerr << "Error parsing MQTT message: " << errs << std::endl;
    }

}