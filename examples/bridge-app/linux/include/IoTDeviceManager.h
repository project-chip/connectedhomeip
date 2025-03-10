#ifndef IOT_DEVICE_MANAGER_H
#define IOT_DEVICE_MANAGER_H

#include <iostream>
#include <fstream>
#include <jsoncpp/json/json.h>
#include <exception>

struct IoTDevice {
    std::string id;
    std::string product_name;
    int product_id;
    std::string vendor_name;
    int vendor_id;
    int endpoint;
    int type_id;
    float hardware_version;
    float software_version;
    std::string hardware_version_string;
    std::string software_version_string;
    std::string serial_number;
    std::string unique_id;
    Json::Value state;
    Json::Value toJson() const;
    static IoTDevice fromJson(const Json::Value &json);
};

class IoTDeviceManager {
    private:
        std::string file_path;
        Json::Value devices;
        std::string new_id = "";
        
    public:
        IoTDeviceManager(const std::string &file_path);
        void addDevice(IoTDevice &device);
        bool removeDevice(const std::string &id);
        bool updateDeviceStateById(const std::string &id, const Json::Value &state);
        bool updateDeviceStateByEndpoint(const int &endpoint, const Json::Value &state);
        void saveDevices() const;
        IoTDevice loadDeviceById(const std::string &id) const;
        IoTDevice loadDeviceByEndpoint(const int &endpoint) const;
        void printAllDevices() const;
        Json::Value getDeviceStateByEndpoint(const int endpoint) const;
        Json::Value getDeviceState(const std::string &id) const;
        bool isEndpointAvailable(int endpoint) const;
        Json::Value& getDevices();
        std::string getNewId() const { return new_id; }
        void setNewId(const std::string &id) { new_id = id; }
        void clearDevices(void);
        // void IoTDeviceManager::setDeviceState(const std::string& deviceId, const std::string& state) ;
};

#endif
