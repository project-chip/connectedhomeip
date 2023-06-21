#include "ThermalClassSensor.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <fstream>

ThermalClassSensor::ThermalClassSensor(
                       chip::EndpointId endpoint_id, 
                       const std::string& sysfs_zone_path, 
                       float min_temperature,
                       float max_temperature):
    m_endpoint_id(endpoint_id),
    m_sysfs_temp(sysfs_zone_path + "/temp"),
    m_last_temp_cdegc(0),
    m_min_temp(min_temperature),
    m_max_temp(max_temperature) {
}

ThermalClassSensor::~ThermalClassSensor() {
    if (m_poll_thread.joinable()) {
        m_poll_thread.join();
    }
}

void ThermalClassSensor::pollTemperature() {

    // set limits
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    chip::app::Clusters::TemperatureMeasurement::Attributes::MinMeasuredValue::Set(m_endpoint_id, m_min_temp * 100);
    chip::app::Clusters::TemperatureMeasurement::Attributes::MaxMeasuredValue::Set(m_endpoint_id, m_max_temp * 100);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    while (m_keep_polling.load()) {
        std::ifstream sensor_file(m_sysfs_temp);
        if (sensor_file.good()) {
            ChipLogDetail(AppServer, "Polling temperature")
            int temp_mdegc;
            sensor_file >> temp_mdegc;
            int temp_cdegc = temp_mdegc / 10;
            if (temp_cdegc != m_last_temp_cdegc) {
                chip::DeviceLayer::PlatformMgr().LockChipStack();
                chip::app::Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Set(m_endpoint_id, temp_cdegc);
                chip::DeviceLayer::PlatformMgr().UnlockChipStack();
            }
            m_last_temp_cdegc = temp_cdegc;
        } else {
            ChipLogError(AppServer, "Failed opening/reading file %s: %s", m_sysfs_temp.c_str(), strerror(errno));   
        }
        if (m_keep_polling.load()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

void ThermalClassSensor::RunMainLoop() { 
    m_keep_polling.store(true);
    m_poll_thread = std::thread(&ThermalClassSensor::pollTemperature, this);
    DefaultAppMainLoopImplementation::RunMainLoop();
}

void ThermalClassSensor::SignalSafeStopMainLoop() {
    m_keep_polling.store(false);
    DefaultAppMainLoopImplementation::SignalSafeStopMainLoop();
}
