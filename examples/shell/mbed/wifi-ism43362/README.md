# ISM43362 WiFi driver for mbed-os

The mbed OS driver for the ISM43362 WiFi module

https://www.inventeksys.com/ism4336-m3g-l44-e-embedded-serial-to-wifi-module/


## Currently supported platforms

ISM43362 module is soldered on the following platforms from STMicroelectronics

 * [DISCO_L475VG_IOT01A](https://os.mbed.com/platforms/ST-Discovery-L475E-IOT01A/)
 * [DISCO_F413ZH](https://os.mbed.com/platforms/ST-Discovery-F413H/)
 * [B_L4S5I_IOT01A](https://os.mbed.com/platforms/B-L4S5I-IOT01A/)

## Configuration

Correct pins have already been configured for both supported platforms.

Here is configured pins:

- MBED_CONF_ISM43362_WIFI_MISO      : spi-miso pin for the ism43362 connection
- MBED_CONF_ISM43362_WIFI_MOSI     : spi-mosi pin for the ism43362 connection
- MBED_CONF_ISM43362_WIFI_SPI_SCLK  : spi-clock pin for the ism43362 connection
- MBED_CONF_ISM43362_WIFI_SPI_NSS   : spi-nss pin for the ism43362 connection
- MBED_CONF_ISM43362_WIFI_RESET     : Reset pin for the ism43362 wifi module
- MBED_CONF_ISM43362_WIFI_DATAREADY : Data Ready pin for the ism43362 wifi module
- MBED_CONF_ISM43362_WIFI_WAKEUP    : Wakeup pin for the ism43362 wifi module

## Debug

Some debug print on console can help to debug if necessary.

- in ISM43362Interface.cpp file, set ism_interface_debug to 1
- in ISM43362/ISM43362.cpp file, set ism_debug to 1
- in ISM43362/ATParser/ATParser.cpp file, there are 3 different level : dbg_on / AT_DATA_PRINT / AT_COMMAND_PRINT

Another way to enable these prints is overwrite MBED_CONF_ISM43362_WIFI_DEBUG in your json file:
            "ism43362.wifi-debug": true
			
## Options

Country code can be specified by using the "ism43362.wifi-country-code" parameter of mbed_app.json.

The default country code = US (USA)

Link between code and country is here :
https://www.juniper.net/documentation/en_US/release-independent/junos/topics/reference/specifications/access-point-ax411-country-channel-support.html

Useful to use all the Radio Channels available in a country.


## Firmware version & firmware update

The recommended firmware version is ISM43362-M3G-L44-SPI-C3.5.2.5.STM

The utility to upgrade firmware on B-L475-IOT01A and B_L4S5I_IOT01A boards can be found here: https://www.st.com/resource/en/utilities/inventek_fw_updater.zip

It contains instructions and URL where to get firmware from (https://www.inventeksys.com/iwin/firmware/)

Note that only Wifi module from DISCO_F413ZH cannot be updated (HW limitation).
