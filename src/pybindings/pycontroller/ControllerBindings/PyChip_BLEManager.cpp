#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#include <sstream> // __str__
#include <system/SystemLayer.h>

#include <functional>
#include <pybind11/pybind11.h>
#include <string>

#ifndef BINDER_PYBIND11_TYPE_CASTER
#define BINDER_PYBIND11_TYPE_CASTER
PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>)
PYBIND11_DECLARE_HOLDER_TYPE(T, T *)
PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_PyChip_BLEManager(std::function<pybind11::module &(std::string const & namespace_)> & M)
{
    { // chip::DeviceLayer::Internal::GenericConnectivityManagerImpl file:platform/internal/GenericConnectivityManagerImpl.h line:40
        pybind11::class_<chip::DeviceLayer::Internal::GenericConnectivityManagerImpl<chip::DeviceLayer::ConnectivityManagerImpl>,
                         std::shared_ptr<chip::DeviceLayer::Internal::GenericConnectivityManagerImpl<
                             chip::DeviceLayer::ConnectivityManagerImpl>>>
            cl(M("chip::DeviceLayer::Internal"), "GenericConnectivityManagerImpl_chip_DeviceLayer_ConnectivityManagerImpl_t", "");
        cl.def(pybind11::init([]() {
            return new chip::DeviceLayer::Internal::GenericConnectivityManagerImpl<chip::DeviceLayer::ConnectivityManagerImpl>();
        }));
        cl.def(pybind11::init(
            [](chip::DeviceLayer::Internal::GenericConnectivityManagerImpl<chip::DeviceLayer::ConnectivityManagerImpl> const & o) {
                return new chip::DeviceLayer::Internal::GenericConnectivityManagerImpl<chip::DeviceLayer::ConnectivityManagerImpl>(
                    o);
            }));
        cl.def(
            "_IsUserSelectedModeActive",
            (bool (chip::DeviceLayer::Internal::GenericConnectivityManagerImpl<chip::DeviceLayer::ConnectivityManagerImpl>::*)()) &
                chip::DeviceLayer::Internal::GenericConnectivityManagerImpl<
                    chip::DeviceLayer::ConnectivityManagerImpl>::_IsUserSelectedModeActive,
            "C++: "
            "chip::DeviceLayer::Internal::GenericConnectivityManagerImpl<chip::DeviceLayer::ConnectivityManagerImpl>::_"
            "IsUserSelectedModeActive() --> bool");
        cl.def("_SetUserSelectedMode",
               (void (chip::DeviceLayer::Internal::GenericConnectivityManagerImpl<chip::DeviceLayer::ConnectivityManagerImpl>::*)(
                   bool)) &
                   chip::DeviceLayer::Internal::GenericConnectivityManagerImpl<
                       chip::DeviceLayer::ConnectivityManagerImpl>::_SetUserSelectedMode,
               "C++: "
               "chip::DeviceLayer::Internal::GenericConnectivityManagerImpl<chip::DeviceLayer::ConnectivityManagerImpl>::_"
               "SetUserSelectedMode(bool) --> void",
               pybind11::arg("val"));
        cl.def("_GetUserSelectedModeTimeout",
               (unsigned short (
                   chip::DeviceLayer::Internal::GenericConnectivityManagerImpl<chip::DeviceLayer::ConnectivityManagerImpl>::*)()) &
                   chip::DeviceLayer::Internal::GenericConnectivityManagerImpl<
                       chip::DeviceLayer::ConnectivityManagerImpl>::_GetUserSelectedModeTimeout,
               "C++: "
               "chip::DeviceLayer::Internal::GenericConnectivityManagerImpl<chip::DeviceLayer::ConnectivityManagerImpl>::_"
               "GetUserSelectedModeTimeout() --> unsigned short");
        cl.def("_SetUserSelectedModeTimeout",
               (void (chip::DeviceLayer::Internal::GenericConnectivityManagerImpl<chip::DeviceLayer::ConnectivityManagerImpl>::*)(
                   unsigned short)) &
                   chip::DeviceLayer::Internal::GenericConnectivityManagerImpl<
                       chip::DeviceLayer::ConnectivityManagerImpl>::_SetUserSelectedModeTimeout,
               "C++: "
               "chip::DeviceLayer::Internal::GenericConnectivityManagerImpl<chip::DeviceLayer::ConnectivityManagerImpl>::_"
               "SetUserSelectedModeTimeout(unsigned short) --> void",
               pybind11::arg("val"));
        cl.def(
            "assign",
            (class chip::DeviceLayer::Internal::GenericConnectivityManagerImpl<class chip::DeviceLayer::ConnectivityManagerImpl> &
             (chip::DeviceLayer::Internal::GenericConnectivityManagerImpl<
                 chip::DeviceLayer::ConnectivityManagerImpl>::*) (const class chip::DeviceLayer::Internal::
                                                                      GenericConnectivityManagerImpl<
                                                                          class chip::DeviceLayer::ConnectivityManagerImpl> &) ) &
                chip::DeviceLayer::Internal::GenericConnectivityManagerImpl<chip::DeviceLayer::ConnectivityManagerImpl>::operator=,
            "C++: "
            "chip::DeviceLayer::Internal::GenericConnectivityManagerImpl<chip::DeviceLayer::ConnectivityManagerImpl>::operator=("
            "const class chip::DeviceLayer::Internal::GenericConnectivityManagerImpl<class "
            "chip::DeviceLayer::ConnectivityManagerImpl> &) --> class "
            "chip::DeviceLayer::Internal::GenericConnectivityManagerImpl<class chip::DeviceLayer::ConnectivityManagerImpl> &",
            pybind11::return_value_policy::reference, pybind11::arg(""));
    }
    { // chip::DeviceLayer::Internal::BLEManager file:platform/internal/BLEManager.h line:46
        pybind11::class_<chip::DeviceLayer::Internal::BLEManager, chip::DeviceLayer::Internal::BLEManager *> cl(
            M("chip::DeviceLayer::Internal"), "BLEManager",
            "Provides control over CHIPoBLE services and connectivity for a chip device.\n\n BLEManager defines the abstract "
            "interface of a singleton object that provides\n control over CHIPoBLE services and connectivity for a chip device.  "
            "BLEManager\n is an internal object that is used by other components with the chip Device\n Layer, but is not directly "
            "accessible to the application.");
        cl.def("Init", &chip::DeviceLayer::Internal::BLEManager::Init,
               "C++: chip::DeviceLayer::Internal::BLEManager::Init() --> int");
        cl.def("GetCHIPoBLEServiceMode", &chip::DeviceLayer::Internal::BLEManager::GetCHIPoBLEServiceMode,
               "C++: chip::DeviceLayer::Internal::BLEManager::GetCHIPoBLEServiceMode() --> enum "
               "chip::DeviceLayer::ConnectivityManager::CHIPoBLEServiceMode");
        cl.def("SetCHIPoBLEServiceMode", &chip::DeviceLayer::Internal::BLEManager::SetCHIPoBLEServiceMode,
               "C++: chip::DeviceLayer::Internal::BLEManager::SetCHIPoBLEServiceMode(enum "
               "chip::DeviceLayer::ConnectivityManager::CHIPoBLEServiceMode) --> int",
               pybind11::arg("val"));
        cl.def("IsAdvertisingEnabled", &chip::DeviceLayer::Internal::BLEManager::IsAdvertisingEnabled,
               "C++: chip::DeviceLayer::Internal::BLEManager::IsAdvertisingEnabled() --> bool");
        cl.def("SetAdvertisingEnabled", &chip::DeviceLayer::Internal::BLEManager::SetAdvertisingEnabled,
               "C++: chip::DeviceLayer::Internal::BLEManager::SetAdvertisingEnabled(bool) --> int", pybind11::arg("val"));
        cl.def("IsAdvertising", &chip::DeviceLayer::Internal::BLEManager::IsAdvertising,
               "C++: chip::DeviceLayer::Internal::BLEManager::IsAdvertising() --> bool");
        cl.def("SetAdvertisingMode", &chip::DeviceLayer::Internal::BLEManager::SetAdvertisingMode,
               "C++: chip::DeviceLayer::Internal::BLEManager::SetAdvertisingMode(enum "
               "chip::DeviceLayer::ConnectivityManager::BLEAdvertisingMode) --> int",
               pybind11::arg("mode"));
        cl.def("GetDeviceName", &chip::DeviceLayer::Internal::BLEManager::GetDeviceName,
               "C++: chip::DeviceLayer::Internal::BLEManager::GetDeviceName(char *, unsigned long) --> int", pybind11::arg("buf"),
               pybind11::arg("bufSize"));
        cl.def("SetDeviceName", &chip::DeviceLayer::Internal::BLEManager::SetDeviceName,
               "C++: chip::DeviceLayer::Internal::BLEManager::SetDeviceName(const char *) --> int", pybind11::arg("deviceName"));
        cl.def("NumConnections", &chip::DeviceLayer::Internal::BLEManager::NumConnections,
               "C++: chip::DeviceLayer::Internal::BLEManager::NumConnections() --> unsigned short");
        cl.def("OnPlatformEvent", &chip::DeviceLayer::Internal::BLEManager::OnPlatformEvent,
               "C++: chip::DeviceLayer::Internal::BLEManager::OnPlatformEvent(const struct chip::DeviceLayer::ChipDeviceEvent *) "
               "--> void",
               pybind11::arg("event"));
        cl.def("GetBleLayer", &chip::DeviceLayer::Internal::BLEManager::GetBleLayer,
               "C++: chip::DeviceLayer::Internal::BLEManager::GetBleLayer() --> class chip::Ble::BleLayer *",
               pybind11::return_value_policy::reference);
    }
    // chip::DeviceLayer::Internal::BLEMgr() file:platform/internal/BLEManager.h line:86
    M("chip::DeviceLayer::Internal")
        .def("BLEMgr", (class chip::DeviceLayer::Internal::BLEManager & (*) ()) & chip::DeviceLayer::Internal::BLEMgr,
             "Returns a reference to the public interface of the BLEManager singleton object.\n\n Internal components should use "
             "this to access features of the BLEManager object\n that are common to all platforms.\n\nC++: "
             "chip::DeviceLayer::Internal::BLEMgr() --> class chip::DeviceLayer::Internal::BLEManager &",
             pybind11::return_value_policy::reference);

    // chip::DeviceLayer::Internal::BLEMgrImpl() file:platform/internal/BLEManager.h line:94
    M("chip::DeviceLayer::Internal")
        .def("BLEMgrImpl", (class chip::DeviceLayer::Internal::BLEManagerImpl & (*) ()) & chip::DeviceLayer::Internal::BLEMgrImpl,
             "Returns the platform-specific implementation of the BLEManager singleton object.\n\n chip applications can use this "
             "to gain access to features of the BLEManager\n that are specific to the selected platform.\n\nC++: "
             "chip::DeviceLayer::Internal::BLEMgrImpl() --> class chip::DeviceLayer::Internal::BLEManagerImpl &",
             pybind11::return_value_policy::reference);

    { // chip::DeviceLayer::Internal::BLEManagerImpl file: line:37
        pybind11::class_<chip::DeviceLayer::Internal::BLEManagerImpl, std::shared_ptr<chip::DeviceLayer::Internal::BLEManagerImpl>,
                         chip::DeviceLayer::Internal::BLEManager>
            cl(M("chip::DeviceLayer::Internal"), "BLEManagerImpl",
               "Concrete implementation of the BLEManagerImpl singleton object for the Darwin platforms.");
        cl.def(pybind11::init([]() { return new chip::DeviceLayer::Internal::BLEManagerImpl(); }));
        cl.def("ConfigureBle", &chip::DeviceLayer::Internal::BLEManagerImpl::ConfigureBle,
               "C++: chip::DeviceLayer::Internal::BLEManagerImpl::ConfigureBle(unsigned int, bool) --> int",
               pybind11::arg("aNodeId"), pybind11::arg("aIsCentral"));
    }
}
