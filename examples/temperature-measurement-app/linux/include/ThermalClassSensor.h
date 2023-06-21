#pragma once

#include <AppMain.h>
#include <string>
#include <thread>
#include <atomic>

class ThermalClassSensor : public DefaultAppMainLoopImplementation
{
public:
    ThermalClassSensor(chip::EndpointId endpoint_id, 
                       const std::string& sysfs_zone_path, 
                       float min_temperature,
                       float max_temperature);
    ~ThermalClassSensor();

private:

    void pollTemperature();
    void RunMainLoop() override;
    void SignalSafeStopMainLoop() override;

    const chip::EndpointId m_endpoint_id;
    std::string m_sysfs_temp;
    int16_t m_last_temp_cdegc;
    const float m_min_temp;
    const float m_max_temp;
    std::thread m_poll_thread;
    std::atomic<bool> m_keep_polling;
};