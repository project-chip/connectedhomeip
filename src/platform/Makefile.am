#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2014-2018 Nest Labs, Inc.
#    Copyright (c) 2018 Google LLC
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

#
#    Description:
#      This file is the GNU automake template for the CHIP Device Layer
#      library.

include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

SUBDIRS                                 = \
    $(NULL)

EXTRA_DIST                              = \
    $(NULL)

#
# Local headers to build against and distribute but not to install
# since they are not part of the CHIP SDK public API.
#
noinst_HEADERS                          = \
    @top_srcdir@/src/include/platform/ConfigurationManager.h \
    @top_srcdir@/src/include/platform/ConnectivityManager.h \
    @top_srcdir@/src/include/platform/SoftwareUpdateManagerImpl.h \
    @top_srcdir@/src/include/platform/GeneralUtils.h \
    @top_srcdir@/src/include/platform/PlatformManager.h \
    @top_srcdir@/src/include/platform/PersistedStorage.h \
    @top_srcdir@/src/include/platform/SoftwareUpdateManager.h \
    @top_srcdir@/src/include/platform/TimeSyncManager.h \
    @top_srcdir@/src/include/platform/ThreadStackManager.h \
    @top_srcdir@/src/include/platform/CHIPDeviceConfig.h \
    @top_srcdir@/src/include/platform/CHIPDeviceError.h \
    @top_srcdir@/src/include/platform/CHIPDeviceEvent.h \
    @top_srcdir@/src/include/platform/CHIPDeviceLayer.h \
    @top_srcdir@/src/include/platform/internal/BLEManager.h \
    @top_srcdir@/src/include/platform/internal/CHIPDeviceLayerInternal.h \
    @top_srcdir@/src/include/platform/internal/DeviceNetworkInfo.h \
    @top_srcdir@/src/include/platform/internal/EventLogging.h \
    @top_srcdir@/src/include/platform/internal/GenericConfigurationManagerImpl.h \
    @top_srcdir@/src/include/platform/internal/GenericConfigurationManagerImpl.ipp \
    @top_srcdir@/src/include/platform/internal/GenericConnectivityManagerImpl.h \
    @top_srcdir@/src/include/platform/internal/GenericConnectivityManagerImpl_BLE.h \
    @top_srcdir@/src/include/platform/internal/GenericConnectivityManagerImpl_BLE.ipp \
    @top_srcdir@/src/include/platform/internal/GenericConnectivityManagerImpl_NoBLE.h \
    @top_srcdir@/src/include/platform/internal/GenericConnectivityManagerImpl_NoThread.h \
    @top_srcdir@/src/include/platform/internal/GenericConnectivityManagerImpl_NoTunnel.h \
    @top_srcdir@/src/include/platform/internal/GenericConnectivityManagerImpl_NoWiFi.h \
    @top_srcdir@/src/include/platform/internal/GenericConnectivityManagerImpl_Thread.h \
    @top_srcdir@/src/include/platform/internal/GenericConnectivityManagerImpl_Thread.ipp \
    @top_srcdir@/src/include/platform/internal/GenericConnectivityManagerImpl_WiFi.h \
    @top_srcdir@/src/include/platform/internal/GenericConnectivityManagerImpl_WiFi.ipp \
    @top_srcdir@/src/include/platform/internal/GenericPlatformManagerImpl.h \
    @top_srcdir@/src/include/platform/internal/GenericPlatformManagerImpl.ipp \
    @top_srcdir@/src/include/platform/internal/GenericPlatformManagerImpl_POSIX.h \
    @top_srcdir@/src/include/platform/internal/GenericPlatformManagerImpl_POSIX.ipp \
    @top_srcdir@/src/include/platform/internal/GenericSoftwareUpdateManagerImpl.h \
    @top_srcdir@/src/include/platform/internal/GenericSoftwareUpdateManagerImpl.ipp \
    @top_srcdir@/src/include/platform/internal/GenericPlatformManagerImpl_FreeRTOS.h \
    @top_srcdir@/src/include/platform/internal/GenericPlatformManagerImpl_FreeRTOS.ipp \
    @top_srcdir@/src/platform/Darwin/BleApplicationDelegate.h \
    @top_srcdir@/src/platform/Darwin/BleConnectionDelegate.h \
    @top_srcdir@/src/platform/Darwin/BlePlatformDelegate.h \
    @top_srcdir@/src/platform/Darwin/BLEManagerImpl.h \
    @top_srcdir@/src/platform/Darwin/BlePlatformConfig.h \
    @top_srcdir@/src/platform/Darwin/CHIPDevicePlatformConfig.h \
    @top_srcdir@/src/platform/Darwin/CHIPDevicePlatformEvent.h \
    @top_srcdir@/src/platform/Darwin/CHIPPlatformConfig.h \
    @top_srcdir@/src/platform/Darwin/ConfigurationManagerImpl.h \
    @top_srcdir@/src/platform/Darwin/ConnectivityManagerImpl.h \
    @top_srcdir@/src/platform/Darwin/InetPlatformConfig.h \
    @top_srcdir@/src/platform/Darwin/PlatformManagerImpl.h \
    @top_srcdir@/src/platform/Darwin/PosixConfig.h \
    @top_srcdir@/src/platform/Darwin/SystemPlatformConfig.h \
    @top_srcdir@/src/platform/nRF5/ConnectivityManagerImpl.h \
    @top_srcdir@/src/platform/nRF5/nRF5Config.h \
    @top_srcdir@/src/platform/nRF5/CHIPDevicePlatformConfig.h \
    @top_srcdir@/src/platform/nRF5/CHIPDevicePlatformEvent.h \
    @top_srcdir@/src/platform/nRF5/PlatformManagerImpl.h \
    @top_srcdir@/src/platform/EFR32/CHIPDevicePlatformConfig.h \
    @top_srcdir@/src/platform/EFR32/GroupKeyStoreImpl.h \
    @top_srcdir@/src/platform/EFR32/efr32-chip-mbedtls-config.h \
    @top_srcdir@/src/platform/EFR32/EFR32Config.h \
    @top_srcdir@/src/platform/EFR32/gatt_db.h \
    @top_srcdir@/src/platform/EFR32/freertos_bluetooth.h \
    @top_srcdir@/src/platform/EFR32/ConnectivityManagerImpl.h \
    @top_srcdir@/src/platform/EFR32/BLEManagerImpl.h \
    @top_srcdir@/src/platform/EFR32/ConfigurationManagerImpl.h \
    @top_srcdir@/src/platform/EFR32/PlatformManagerImpl.h \
    @top_srcdir@/src/platform/EFR32/SoftwareUpdateManagerImpl.h \
    @top_srcdir@/src/platform/EFR32/CHIPDevicePlatformEvent.h \
    @top_srcdir@/src/platform/EFR32/NetworkProvisioningServerImpl.h \
    @top_srcdir@/src/platform/ESP32/CHIPDevicePlatformConfig.h \
    @top_srcdir@/src/platform/ESP32/ConnectivityManagerImpl.h \
    @top_srcdir@/src/platform/ESP32/BLEManagerImpl.h \
    @top_srcdir@/src/platform/ESP32/ConfigurationManagerImpl.h \
    @top_srcdir@/src/platform/ESP32/PlatformManagerImpl.h \
    @top_srcdir@/src/platform/ESP32/SoftwareUpdateManagerImpl.h \
    @top_srcdir@/src/platform/ESP32/CHIPDevicePlatformEvent.h \
    @top_srcdir@/src/platform/ESP32/NetworkProvisioningServerImpl.h \
    @top_srcdir@/src/platform/ESP32/ESP32Utils.h \
    @top_srcdir@/src/platform/ESP32/ESP32Config.h \
    @top_srcdir@/src/platform/Linux/BLEManagerImpl.h \
    @top_srcdir@/src/platform/Linux/BlePlatformConfig.h \
    @top_srcdir@/src/platform/Linux/CHIPLinuxStorage.h \
    @top_srcdir@/src/platform/Linux/CHIPLinuxStorageIni.h \
    @top_srcdir@/src/platform/Linux/CHIPDevicePlatformConfig.h \
    @top_srcdir@/src/platform/Linux/CHIPDevicePlatformEvent.h \
    @top_srcdir@/src/platform/Linux/CHIPPlatformConfig.h \
    @top_srcdir@/src/platform/Linux/ConfigurationManagerImpl.h \
    @top_srcdir@/src/platform/Linux/ConnectivityManagerImpl.h \
    @top_srcdir@/src/platform/Linux/InetPlatformConfig.h \
    @top_srcdir@/src/platform/Linux/PlatformManagerImpl.h \
    @top_srcdir@/src/platform/Linux/PosixConfig.h \
    @top_srcdir@/src/platform/Linux/SystemPlatformConfig.h \
    @top_srcdir@/src/platform/Linux/ThreadStackManagerImpl.h \
    @top_srcdir@/src/include/platform/internal/testing/ConfigUnitTest.h \
    $(NULL)

if CONFIG_DEVICE_LAYER

lib_LIBRARIES                           = libDeviceLayer.a

libDeviceLayer_a_CPPFLAGS               = \
    -I$(top_srcdir)/src/include           \
    -I$(top_srcdir)/src/lib               \
    -I$(top_srcdir)/src/system            \
    -I$(top_srcdir)/src                   \
    $(NLASSERT_CPPFLAGS)                  \
    $(NLIO_CPPFLAGS)                      \
    $(LWIP_CPPFLAGS)                      \
    $(SOCKETS_CPPFLAGS)                   \
    $(NULL)

libDeviceLayer_a_SOURCES                = \
    PersistedStorage.cpp                  \
    SystemEventSupport.cpp                \
    SystemTimerSupport.cpp                \
    GeneralUtils.cpp                      \
    Globals.cpp                           \
    $(NULL)

if CHIP_ENABLE_OPENTHREAD
libDeviceLayer_a_SOURCES               += \
    OpenThread/OpenThreadUtils.cpp        \
    $(NULL)
endif

if CHIP_DEVICE_LAYER_TARGET_DARWIN

libDeviceLayer_a_CPPFLAGS              += \
    $(NULL)

libDeviceLayer_a_SOURCES               += \
    Darwin/BleApplicationDelegateImpl.mm  \
    Darwin/BleConnectionDelegateImpl.mm   \
    Darwin/BlePlatformDelegateImpl.mm     \
    Darwin/BLEManagerImpl.cpp             \
    Darwin/ConfigurationManagerImpl.cpp   \
    Darwin/ConnectivityManagerImpl.cpp    \
    Darwin/PlatformManagerImpl.cpp        \
    Darwin/PosixConfig.cpp                \
    $(NULL)

Platformdir                                                     = $(includedir)/platform/Darwin
dist_Platform_HEADERS                                           = \
    @top_srcdir@/src/platform/Darwin/BlePlatformConfig.h        \
    @top_srcdir@/src/platform/Darwin/InetPlatformConfig.h       \
    @top_srcdir@/src/platform/Darwin/CHIPPlatformConfig.h       \
    @top_srcdir@/src/platform/Darwin/CHIPDevicePlatformConfig.h \
    @top_srcdir@/src/platform/Darwin/SystemPlatformConfig.h     \
    $(NULL)

endif # CHIP_DEVICE_LAYER_TARGET_DARWIN

if CHIP_DEVICE_LAYER_TARGET_LINUX

libDeviceLayer_a_CPPFLAGS              += \
    -I$(builddir)/Linux/gen               \
    $(NULL)

SUBDIRS                                += tests

.PHONY: gen
gen:
	mkdir -p Linux/gen

BUILT_SOURCES                         = \
    Linux/gen                           \
    $(NULL)

libDeviceLayer_a_CPPFLAGS              += \
    $(GIO_CFLAGS)                         \
    $(GIO_UNIX_CFLAGS)                    \
    $(INIPP_CPPFLAGS)                     \
    $(NULL)

if CONFIG_BLE_PLATFORM_BLUEZ

BUILT_SOURCES                         += \
    Linux/gen/DbusBluez.h                      \
    Linux/gen/DbusBluez.c                      \
    $(NULL)

GDBUSCODEGENOPTS                          = \
    --interface-prefix org.bluez            \
    --c-namespace Bluez                     \
    --c-generate-object-manager             \
    $(NULL)

Linux/gen/DbusBluez.h: Linux/dbus/org.bluez.xml
	$(GDBUSCODEGEN) $(GDBUSCODEGENOPTS) --header --output $@ $<

Linux/gen/DbusBluez.c: Linux/dbus/org.bluez.xml
	$(GDBUSCODEGEN) $(GDBUSCODEGENOPTS) --body --output $@ $<
	sed -i 's/config\.h/BuildConfig.h/g' $@

libDeviceLayer_a_SOURCES               += \
    Linux/gen/DbusBluez.c                 \
    $(NULL)

endif #CONFIG_BLE_PLATFORM_BLUEZ

if CONFIG_WIFI_PLATFORM_WPA
BUILT_SOURCES                          += \
    Linux/gen/DBusWpa.h                   \
    Linux/gen/DBusWpaInterface.h          \
    Linux/gen/DBusWpaNetwork.h            \
    Linux/gen/DBusWpa.c                   \
    Linux/gen/DBusWpaInterface.c          \
    Linux/gen/DBusWpaNetwork.c            \
    $(NULL)

Linux/gen/DBusWpa.h: Linux/dbus/wpa.xml
	gdbus-codegen --header --output $@ --c-namespace Wpa $<

Linux/gen/DBusWpaInterface.h: Linux/dbus/wpa_interface.xml
	gdbus-codegen --header --output $@ --c-namespace Wpa $<

Linux/gen/DBusWpaNetwork.h: Linux/dbus/wpa_network.xml
	gdbus-codegen --header --output $@ --c-namespace Wpa $<

Linux/gen/DBusWpa.c: Linux/dbus/wpa.xml
	gdbus-codegen --body --output $@ --c-namespace Wpa $<
	sed -i 's/config\.h/BuildConfig.h/g' $@

Linux/gen/DBusWpaInterface.c: Linux/dbus/wpa_interface.xml
	gdbus-codegen --body --output $@ --c-namespace Wpa $<
	sed -i 's/config\.h/BuildConfig.h/g' $@

Linux/gen/DBusWpaNetwork.c: Linux/dbus/wpa_network.xml
	gdbus-codegen --body --output $@ --c-namespace Wpa $<
	sed -i 's/config\.h/BuildConfig.h/g' $@

libDeviceLayer_a_SOURCES               += \
    Linux/gen/DBusWpa.c                   \
    Linux/gen/DBusWpaInterface.c          \
    Linux/gen/DBusWpaNetwork.c            \
    $(NULL)
endif # CONFIG_WIFI_PLATFORM_WPA

libDeviceLayer_a_SOURCES               += \
    Linux/BLEManagerImpl.cpp              \
    Linux/ConfigurationManagerImpl.cpp    \
    Linux/ConnectivityManagerImpl.cpp     \
    Linux/Logging.cpp                     \
    Linux/PosixConfig.cpp                 \
    Linux/CHIPLinuxStorage.cpp            \
    Linux/CHIPLinuxStorageIni.cpp         \
    Linux/PlatformManagerImpl.cpp         \
    Linux/SystemTimeSupport.cpp           \
    $(NULL)

if CHIP_WITH_OT_BR_POSIX
libDeviceLayer_a_SOURCES               += \
    Linux/ThreadStackManagerImpl.cpp      \
    $(NULL)

libDeviceLayer_a_CPPFLAGS              += \
    -DCHIP_DEVICE_CONFIG_ENABLE_THREAD=1  \
    $(OT_BR_POSIX_CPPFLAGS)               \
    $(DBUS_CFLAGS)                        \
    $(NULL)
endif

endif # CHIP_DEVICE_LAYER_TARGET_LINUX

if CHIP_DEVICE_LAYER_TARGET_NRF5

libDeviceLayer_a_SOURCES               +=       \
    nRF5/BLEManagerImpl.cpp                     \
    nRF5/PlatformManagerImpl.cpp                \
    nRF5/ConfigurationManagerImpl.cpp           \
    nRF5/ConnectivityManagerImpl.cpp            \
    nRF5/nRF5Config.cpp                         \
    nRF5/nRF5Utils.cpp                          \
    nRF5/Logging.cpp                            \
    FreeRTOS/SystemTimeSupport.cpp              \
    $(NULL)

if CHIP_ENABLE_OPENTHREAD
libDeviceLayer_a_SOURCES               +=       \
    nRF5/ThreadStackManagerImpl.cpp             \
    $(NULL)
endif

endif # CHIP_DEVICE_LAYER_TARGET_NRF5

if CHIP_DEVICE_LAYER_TARGET_EFR32

libDeviceLayer_a_SOURCES               +=       \
    EFR32/BLEManagerImpl.cpp                    \
    EFR32/freertos_bluetooth.c                  \
    EFR32/gatt_db.c                             \
    EFR32/PlatformManagerImpl.cpp               \
    EFR32/ConfigurationManagerImpl.cpp          \
    EFR32/ConnectivityManagerImpl.cpp           \
    EFR32/EFR32Config.cpp                       \
    EFR32/Logging.cpp                           \
    FreeRTOS/SystemTimeSupport.cpp              \
    $(NULL)

if CHIP_ENABLE_OPENTHREAD
libDeviceLayer_a_SOURCES               +=       \
    EFR32/ThreadStackManagerImpl.cpp            \
    $(NULL)
endif

endif # CHIP_DEVICE_LAYER_TARGET_EFR32

if CHIP_DEVICE_LAYER_TARGET_ESP32

libDeviceLayer_a_SOURCES               +=       \
    ESP32/BLEManagerImpl.cpp                    \
    ESP32/ConfigurationManagerImpl.cpp          \
    ESP32/PlatformManagerImpl.cpp               \
    ESP32/ConnectivityManagerImpl.cpp           \
    ESP32/ESP32Config.cpp                       \
    ESP32/ESP32Utils.cpp                        \
    ESP32/Logging.cpp                           \
    ESP32/SystemTimeSupport.cpp                 \
    ESP32/LwIPCoreLock.cpp                      \
    FreeRTOS/SystemTimeSupport.cpp              \
    $(NULL)

endif # CHIP_DEVICE_LAYER_TARGET_ESP32

if CHIP_DEVICE_LAYER_TARGET_NRFCONNECT

libDeviceLayer_a_SOURCES               +=       \
    nrfconnect/BLEManagerImpl.cpp               \
    nrfconnect/ConfigurationManagerImpl.cpp     \
    nrfconnect/ConnectivityManagerImpl.cpp      \
    nrfconnect/Logging.cpp                      \
    nrfconnect/ZephyrConfig.cpp                 \
    nrfconnect/PlatformManagerImpl.cpp          \
    Zephyr/SystemTimeSupport.cpp                \
    $(NULL)

if CHIP_ENABLE_OPENTHREAD
libDeviceLayer_a_SOURCES               +=       \
    nrfconnect/ThreadStackManagerImpl.cpp       \
    $(NULL)
endif

endif # CHIP_DEVICE_LAYER_TARGET_NRFCONNECT

endif # CONFIG_DEVICE_LAYER

include $(abs_top_nlbuild_autotools_dir)/automake/post.am
