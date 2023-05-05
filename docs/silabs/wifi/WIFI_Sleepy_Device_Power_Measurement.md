# Power Measurements for Wi-Fi Devices

This page explains how to measure the power values for EFR and Wi-Fi co-processor.

### EFR32 + RS9116 Setup for Sleepy Devices

![Power measurment GPIO pin connection for 9116](./images/WiFi_9116_GPIO_Connection.png)

> Note: The following GPIO pins should be connected for 9116 and Host handshakes. \
pin 7 and 9 to UULP_2 and UULP_0 respectively.

## Using Simplicity Studio Energy Profiler for Current Measurement

After flashing the Matter application to the module, Energy profiler or a power meter can be used for power measurements.

![Power measurment connection overview](./images/WiFi_PowerSave_Overview.png)

In Simplicity Studio, click **Tools** on the toolbar, select Energy Profiler, and click "OK".

![Power measurment PowerProfiler](./images/WiFi_PowerProfiler.png)

From the Quick Access or Profiler menu, select 'Start Energy Capture'. 

![Power measurment using PowerProfiler](./images/WiFi_PowerProfiler_start.png)

> **Note**: A quick-start guide on the Energy Profiler user interface is in the Simplicity Studio User's Guide's [Energy Profiler User Interface section](https://docs.silabs.com/simplicity-studio-5-users-guide/latest/ss-5-users-guide-tools-energy-profiler/energy-profiler-user-interface).

## Power Consumption Measurment (EFR) Using Energy Profiler

Analyze the power values using Energy Profiler.
![Power measurment for EFR using EnergyProfiler](./images/WiFi_EFR_Power_EnergyProfiler.png)

## Power Consumption mMasurment Using a Power Meter

### Power Consumption Measurement Pins for RS9116 Evk Boards
![Power measurment pins for NCP mode](./images/WiFi_9116_PowerMeter_WiFiModule.png)

### Power Consumption Measurement Pins for EXP Boards
![Power measurment pins for NCP mode](./images/SiWx917_Ncp_PowerMeasurment_pins.png)

The power meter's negative probe is used for pin-1 and the positive probe is used for pin-2.

Analyze the power values using the power meter.
![Power measurment for 9116/917 using power meter](./images/WiFi_9116_Power_PowerProfiler.png)
