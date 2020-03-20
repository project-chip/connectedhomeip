#
#    <COPYRIGHT>
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

EXTRA_DIST                              = \
    $(NULL)

#
# Local headers to build against and distribute but not to install
# since they are not part of the CHIP SDK public API.
#
noinst_HEADERS                          = \
    include/platform/ConfigurationManager.h \
    include/platform/ConnectivityManager.h \
    include/platform/ConnectivityManagerImpl.h \
    include/platform/PlatformManagerImpl.h \
    include/platform/SoftwareUpdateManagerImpl.h \
    include/platform/nRF5/ConfigurationManagerImpl.h \
    include/platform/nRF5/nRF5Config.h \
    include/platform/nRF5/CHIPDevicePlatformConfig.h \
    include/platform/nRF5/CHIPDevicePlatformEvent.h \
    include/platform/internal/GenericPlatformManagerImpl_FreeRTOS.h \
    include/platform/internal/GenericPlatformManagerImpl_FreeRTOS.ipp \
    include/platform/GeneralUtils.h \
    include/platform/PlatformManager.h \
    include/platform/SoftwareUpdateManager.h \
    include/platform/TimeSyncManager.h \
    include/platform/CHIPDeviceConfig.h \
    include/platform/CHIPDeviceError.h \
    include/platform/CHIPDeviceEvent.h \
    include/platform/CHIPDeviceLayer.h \
    include/platform/internal/BLEManager.h \
    include/platform/internal/DeviceDescriptionServer.h \
    include/platform/internal/EventLogging.h \
    include/platform/internal/GenericConfigurationManagerImpl.h \
    include/platform/internal/GenericConfigurationManagerImpl.ipp \
    include/platform/internal/GenericPlatformManagerImpl.h \
    include/platform/internal/GenericPlatformManagerImpl.ipp \
    include/platform/internal/GenericSoftwareUpdateManagerImpl.h \
    include/platform/internal/GenericSoftwareUpdateManagerImpl.ipp \
    include/platform/internal/GenericSoftwareUpdateManagerImpl_BDX.h \
    include/platform/internal/GenericSoftwareUpdateManagerImpl_BDX.ipp \
    include/platform/internal/CHIPDeviceLayerInternal.h \
    $(NULL)

if CONFIG_DEVICE_LAYER

lib_LIBRARIES                           = libDeviceLayer.a

libDeviceLayer_a_CPPFLAGS               = \
    -I$(top_srcdir)/src/include           \
    $(LWIP_CPPFLAGS)                      \
    $(SOCKETS_CPPFLAGS)                   \
    $(NULL)

libDeviceLayer_a_SOURCES                = \
    $(NULL)

if CHIP_DEVICE_LAYER_TARGET_NRF5

libDeviceLayer_a_SOURCES               +=       \
    nRF5/BLEManagerImpl.cpp                     \
    nRF5/ConfigurationManagerImpl.cpp           \
    nRF5/PlatformManagerImpl.cpp                \
    nRF5/ConnectivityManagerImpl.cpp            \
    nRF5/ThreadStackManagerImpl.cpp             \
    nRF5/GroupKeyStoreImpl.cpp                  \
    nRF5/nRF5Config.cpp                         \
    nRF5/nRF5Utils.cpp                          \
    nRF5/Entropy.cpp                            \
    nRF5/Logging.cpp                            \
    nRF5/SoftwareUpdateManagerImpl.cpp          \
    $(NULL)

endif # CHIP_DEVICE_LAYER_TARGET_NRF5

if CHIP_DEVICE_LAYER_TARGET_EFR32

libDeviceLayer_a_SOURCES               +=       \
    EFR32/BLEManagerImpl.cpp                    \
    EFR32/freertos_bluetooth.c                  \
    EFR32/gatt_db.c                             \
    EFR32/ConfigurationManagerImpl.cpp          \
    EFR32/PlatformManagerImpl.cpp               \
    EFR32/ConnectivityManagerImpl.cpp           \
    EFR32/NetworkProvisioningServerImpl.cpp     \
    EFR32/ThreadStackManagerImpl.cpp            \
    EFR32/GroupKeyStoreImpl.cpp                 \
    EFR32/EFR32Config.cpp                       \
    EFR32/Entropy.cpp                           \
    EFR32/Logging.cpp                           \
    EFR32/SoftwareUpdateManagerImpl.cpp         \
    $(NULL)

endif # CHIP_DEVICE_LAYER_TARGET_EFR32

endif # CONFIG_DEVICE_LAYER

include $(abs_top_nlbuild_autotools_dir)/automake/post.am
