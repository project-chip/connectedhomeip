#include "IoTDeviceManager.h"
#include <filesystem>
#include <csignal>

Json::Value IoTDevice::toJson() const {
    Json::Value json;
    json["id"] = id;
    json["product_name"] = product_name;
    json["product_id"] = product_id;
    json["vendor_name"] = vendor_name;
    json["vendor_id"] = vendor_id;
    json["type_id"] = type_id;
    json["hardware_version"] = hardware_version;
    json["software_version"] = software_version;
    json["hardware_version_string"] = hardware_version_string;
    json["software_version_string"] = software_version_string;
    json["serial_number"] = serial_number;
    json["unique_id"] = unique_id;
    json["state"] = state;
    json["endpoint"] = endpoint;
    return json;
}

IoTDevice IoTDevice::fromJson(const Json::Value &json) {

    IoTDevice device;
    device.id = json["id"].asString();
    device.product_name = json["product_name"].asString();
    device.product_id = json["product_id"].asInt();
    device.vendor_name = json["vendor_name"].asString();
    device.vendor_id = json["vendor_id"].asInt();
    device.type_id = json["type_id"].asInt();
    device.hardware_version = json["hardware_version"].asFloat();
    device.software_version = json["software_version"].asFloat();
    device.hardware_version_string = json["hardware_version_string"].asString();
    device.software_version_string = json["software_version_string"].asString();
    device.serial_number = json["serial_number"].asString();
    device.unique_id = json["unique_id"].asString();
    device.state = json["state"];
    device.endpoint = json["endpoint"].asUInt();    
    return device;
}

IoTDeviceManager::IoTDeviceManager(const std::string &path) : file_path(path) {
    if (!std::filesystem::exists(file_path)) {
        std::cout << "File " << file_path << " not found. Creating new JSON file..." << std::endl;
        saveDevices();  // Ghi file JSON rỗng
    } else {
        std::ifstream file(file_path);
        if (file.good()) file >> devices;
    }
}

void IoTDeviceManager::addDevice(IoTDevice &device) {
    device.endpoint = devices.size() + 3;
    devices[device.id] = device.toJson();
    std::cout << "Device added: " << device.id << std::endl;
    setNewId(device.id);
    saveDevices();  
    raise(SIGUSR1); 
}

bool IoTDeviceManager::removeDevice(const std::string &id) {
    devices.removeMember(id);
    if(devices.isMember(id)) return false;
    saveDevices();
    return true;  
}

bool IoTDeviceManager::updateDeviceStateById(const std::string &id, const Json::Value &state) {
    if (!devices.isMember(id)) return false;

    for (const auto &key : state.getMemberNames()) {
        devices[id]["state"][key] = state[key];
    }
    saveDevices();
    return true; 
}

bool IoTDeviceManager::updateDeviceStateByEndpoint(const int &endpoint, const Json::Value &state){
    
    for (const auto &device : devices) {
        if (device["endpoint"] == endpoint) {
            for (const auto &key : state.getMemberNames()) {
                devices[device["id"].asString()]["state"][key] = state[key];
            }
            return true;
        }
    }
    saveDevices();
    return false; 
}

void IoTDeviceManager::saveDevices() const {  
    std::ofstream file(file_path);
    file << devices;   
}

IoTDevice IoTDeviceManager::loadDeviceById(const std::string &id) const { 
    if (!devices.isMember(id)) {
        std::cerr << "Device ID not found!" << std::endl;
        return IoTDevice();
    }
    return IoTDevice::fromJson(devices[id]); 
}

IoTDevice IoTDeviceManager::loadDeviceByEndpoint(const int &endpoint) const { 

    for (const auto &device : devices) {
        if (device["endpoint"] == endpoint) {
            return IoTDevice::fromJson(device); // Giả định trạng thái luôn là active
        }
    }

    return IoTDevice(); 
}

void IoTDeviceManager::printAllDevices() const { 
    std::cout << "===== IoT Devices =====" << std::endl;
    std::cout << devices.toStyledString() << std::endl;   
}

Json::Value IoTDeviceManager::getDeviceState(const std::string &id) const {   
    if (!devices.isMember(id)) {
        std::cerr << "Device ID not found!" << std::endl;
        return Json::Value();
    }
    return devices[id]["state"];   
}

bool IoTDeviceManager::isEndpointAvailable(int endpoint) const {
    for (const auto &device : devices) {
        if (device["endpoint"] == endpoint) {
            return false;
        }
    }
    return true;
}

Json::Value IoTDeviceManager::getDeviceStateByEndpoint(const int endpoint) const {
    for (auto &device : devices) {
        if (device["endpoint"] == endpoint) {
            // std::cout << "Device found: " << device.toStyledString() << std::endl;
            return device["state"]; // Giả định trạng thái luôn là active
        }
    }
    return Json::Value();
}

Json::Value& IoTDeviceManager::getDevices(){
    return devices;
}

void IoTDeviceManager::clearDevices(void)
{
    std::ofstream file(file_path, std::ios::trunc); // Mở file với chế độ xóa toàn bộ nội dung
    if (!file)
    {
        std::cerr << "Error: Cannot open file " << file_path << std::endl;
        return;
    }
    devices.clear(); // Xóa toàn bộ dữ liệu trong devices
    file << "{}"; // Ghi đối tượng JSON rỗng
    file.close();
    std::cout << "JSON file cleared: " << file_path << std::endl;
}
