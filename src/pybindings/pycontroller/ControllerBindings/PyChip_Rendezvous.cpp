#include <ble/BleApplicationDelegate.h>                    // chip::Ble::BleApplicationDelegate
#include <ble/BleConnectionDelegate.h>                     // chip::Ble::BleConnectionDelegate
#include <ble/BleLayer.h>                                  // chip::Ble::BleLayer
#include <ble/BlePlatformDelegate.h>                       // chip::Ble::BlePlatformDelegate
#include <ble/BleUUID.h>                                   // chip::Ble::ChipBleUUID
#include <core/Optional.h>                                 // chip::Optional
#include <inet/IPAddress.h>                                // chip::Inet::IPAddress
#include <protocols/secure_channel/RendezvousParameters.h> // chip::RendezvousAdvertisementDelegate
#include <protocols/secure_channel/RendezvousParameters.h> // chip::RendezvousParameters
#include <sstream>                                         // __str__
#include <support/SerializableIntegerSet.h>                // chip::SerializableU64Set
#include <support/SerializableIntegerSet.h>                // chip::SerializableU64SetBase
#include <support/Span.h>                                  // chip::Span
#include <system/SystemLayer.h>                            // chip::System::Layer
#include <transport/raw/PeerAddress.h>                     // chip::Transport::PeerAddress
#include <transport/raw/PeerAddress.h>                     // chip::Transport::Type

#include <controller/CHIPDeviceController.h>
#include <functional>
#include <pybind11/pybind11.h>
#include <string>

#ifndef BINDER_PYBIND11_TYPE_CASTER
#define BINDER_PYBIND11_TYPE_CASTER
PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>)
PYBIND11_DECLARE_HOLDER_TYPE(T, T *)
PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

// chip::RendezvousAdvertisementDelegate file:protocols/secure_channel/RendezvousParameters.h line:34
struct PyCallBack_chip_RendezvousAdvertisementDelegate : public chip::RendezvousAdvertisementDelegate
{
    using chip::RendezvousAdvertisementDelegate::RendezvousAdvertisementDelegate;

    int StartAdvertisement() const override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::RendezvousAdvertisementDelegate *>(this), "StartAdvertisement");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>();
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        return RendezvousAdvertisementDelegate::StartAdvertisement();
    }
    int StopAdvertisement() const override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::RendezvousAdvertisementDelegate *>(this), "StopAdvertisement");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>();
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        return RendezvousAdvertisementDelegate::StopAdvertisement();
    }
    void RendezvousComplete() const override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::RendezvousAdvertisementDelegate *>(this), "RendezvousComplete");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>();
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value)
            {
                static pybind11::detail::override_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<void>(std::move(o));
        }
        return RendezvousAdvertisementDelegate::RendezvousComplete();
    }
};

void bind_PyChip_Rendezvous(std::function<pybind11::module &(std::string const & namespace_)> & M)
{
    { // chip::RendezvousAdvertisementDelegate file:protocols/secure_channel/RendezvousParameters.h line:34
        pybind11::class_<chip::RendezvousAdvertisementDelegate, std::shared_ptr<chip::RendezvousAdvertisementDelegate>,
                         PyCallBack_chip_RendezvousAdvertisementDelegate>
            cl(M("chip"), "RendezvousAdvertisementDelegate", "");
        cl.def(pybind11::init([]() { return new chip::RendezvousAdvertisementDelegate(); },
                              []() { return new PyCallBack_chip_RendezvousAdvertisementDelegate(); }));
        cl.def(pybind11::init([](PyCallBack_chip_RendezvousAdvertisementDelegate const & o) {
            return new PyCallBack_chip_RendezvousAdvertisementDelegate(o);
        }));
        cl.def(pybind11::init(
            [](chip::RendezvousAdvertisementDelegate const & o) { return new chip::RendezvousAdvertisementDelegate(o); }));
        cl.def("StartAdvertisement",
               (int (chip::RendezvousAdvertisementDelegate::*)() const) & chip::RendezvousAdvertisementDelegate::StartAdvertisement,
               "called to start advertising that rendezvous is possible (commisioning available)\n\nC++: "
               "chip::RendezvousAdvertisementDelegate::StartAdvertisement() const --> int");
        cl.def("StopAdvertisement",
               (int (chip::RendezvousAdvertisementDelegate::*)() const) & chip::RendezvousAdvertisementDelegate::StopAdvertisement,
               "called when advertisement is not needed for Rendezvous (e.g. got a BLE connection)\n\nC++: "
               "chip::RendezvousAdvertisementDelegate::StopAdvertisement() const --> int");
        cl.def("RendezvousComplete",
               (void (chip::RendezvousAdvertisementDelegate::*)() const) &
                   chip::RendezvousAdvertisementDelegate::RendezvousComplete,
               "Called when a rendezvous operation is complete\n\nC++: chip::RendezvousAdvertisementDelegate::RendezvousComplete() "
               "const --> void");
        cl.def("assign",
               (class chip::RendezvousAdvertisementDelegate &
                (chip::RendezvousAdvertisementDelegate::*) (const class chip::RendezvousAdvertisementDelegate &) ) &
                   chip::RendezvousAdvertisementDelegate::operator=,
               "C++: chip::RendezvousAdvertisementDelegate::operator=(const class chip::RendezvousAdvertisementDelegate &) --> "
               "class chip::RendezvousAdvertisementDelegate &",
               pybind11::return_value_policy::automatic, pybind11::arg(""));
    }
    { // chip::RendezvousParameters file:protocols/secure_channel/RendezvousParameters.h line:49
        pybind11::class_<chip::RendezvousParameters, std::shared_ptr<chip::RendezvousParameters>> cl(M("chip"),
                                                                                                     "RendezvousParameters", "");
        cl.def(pybind11::init([]() { return new chip::RendezvousParameters(); }));
        cl.def("IsController", (bool (chip::RendezvousParameters::*)() const) & chip::RendezvousParameters::IsController,
               "C++: chip::RendezvousParameters::IsController() const --> bool");
        cl.def("HasSetupPINCode", (bool (chip::RendezvousParameters::*)() const) & chip::RendezvousParameters::HasSetupPINCode,
               "C++: chip::RendezvousParameters::HasSetupPINCode() const --> bool");
        cl.def("GetSetupPINCode",
               (unsigned int (chip::RendezvousParameters::*)() const) & chip::RendezvousParameters::GetSetupPINCode,
               "C++: chip::RendezvousParameters::GetSetupPINCode() const --> unsigned int");
        cl.def("SetSetupPINCode",
               (class chip::RendezvousParameters & (chip::RendezvousParameters::*) (unsigned int) ) &
                   chip::RendezvousParameters::SetSetupPINCode,
               "C++: chip::RendezvousParameters::SetSetupPINCode(unsigned int) --> class chip::RendezvousParameters &",
               pybind11::return_value_policy::automatic, pybind11::arg("setupPINCode"));
        cl.def("HasPeerAddress", (bool (chip::RendezvousParameters::*)() const) & chip::RendezvousParameters::HasPeerAddress,
               "C++: chip::RendezvousParameters::HasPeerAddress() const --> bool");
        cl.def("GetPeerAddress",
               (class chip::Transport::PeerAddress(chip::RendezvousParameters::*)() const) &
                   chip::RendezvousParameters::GetPeerAddress,
               "C++: chip::RendezvousParameters::GetPeerAddress() const --> class chip::Transport::PeerAddress");
        cl.def("SetPeerAddress",
               (class chip::RendezvousParameters & (chip::RendezvousParameters::*) (const class chip::Transport::PeerAddress &) ) &
                   chip::RendezvousParameters::SetPeerAddress,
               "C++: chip::RendezvousParameters::SetPeerAddress(const class chip::Transport::PeerAddress &) --> class "
               "chip::RendezvousParameters &",
               pybind11::return_value_policy::automatic, pybind11::arg("peerAddress"));
        cl.def("HasDiscriminator", (bool (chip::RendezvousParameters::*)() const) & chip::RendezvousParameters::HasDiscriminator,
               "C++: chip::RendezvousParameters::HasDiscriminator() const --> bool");
        cl.def("GetDiscriminator",
               (unsigned short (chip::RendezvousParameters::*)() const) & chip::RendezvousParameters::GetDiscriminator,
               "C++: chip::RendezvousParameters::GetDiscriminator() const --> unsigned short");
        cl.def("SetDiscriminator",
               (class chip::RendezvousParameters & (chip::RendezvousParameters::*) (unsigned short) ) &
                   chip::RendezvousParameters::SetDiscriminator,
               "C++: chip::RendezvousParameters::SetDiscriminator(unsigned short) --> class chip::RendezvousParameters &",
               pybind11::return_value_policy::automatic, pybind11::arg("discriminator"));
        cl.def("HasLocalNodeId", (bool (chip::RendezvousParameters::*)() const) & chip::RendezvousParameters::HasLocalNodeId,
               "C++: chip::RendezvousParameters::HasLocalNodeId() const --> bool");
        cl.def("SetLocalNodeId",
               (class chip::RendezvousParameters & (chip::RendezvousParameters::*) (unsigned long long) ) &
                   chip::RendezvousParameters::SetLocalNodeId,
               "C++: chip::RendezvousParameters::SetLocalNodeId(unsigned long long) --> class chip::RendezvousParameters &",
               pybind11::return_value_policy::automatic, pybind11::arg("nodeId"));
        cl.def("HasRemoteNodeId", (bool (chip::RendezvousParameters::*)() const) & chip::RendezvousParameters::HasRemoteNodeId,
               "C++: chip::RendezvousParameters::HasRemoteNodeId() const --> bool");
        cl.def("SetRemoteNodeId",
               (class chip::RendezvousParameters & (chip::RendezvousParameters::*) (unsigned long long) ) &
                   chip::RendezvousParameters::SetRemoteNodeId,
               "C++: chip::RendezvousParameters::SetRemoteNodeId(unsigned long long) --> class chip::RendezvousParameters &",
               pybind11::return_value_policy::automatic, pybind11::arg("nodeId"));
        cl.def("HasPASEVerifier", (bool (chip::RendezvousParameters::*)() const) & chip::RendezvousParameters::HasPASEVerifier,
               "C++: chip::RendezvousParameters::HasPASEVerifier() const --> bool");
        cl.def("HasAdvertisementDelegate",
               (bool (chip::RendezvousParameters::*)() const) & chip::RendezvousParameters::HasAdvertisementDelegate,
               "C++: chip::RendezvousParameters::HasAdvertisementDelegate() const --> bool");
        cl.def("GetAdvertisementDelegate",
               (const class chip::RendezvousAdvertisementDelegate * (chip::RendezvousParameters::*) () const) &
                   chip::RendezvousParameters::GetAdvertisementDelegate,
               "C++: chip::RendezvousParameters::GetAdvertisementDelegate() const --> const class "
               "chip::RendezvousAdvertisementDelegate *",
               pybind11::return_value_policy::automatic);
        cl.def(
            "SetAdvertisementDelegate",
            (class chip::RendezvousParameters & (chip::RendezvousParameters::*) (class chip::RendezvousAdvertisementDelegate *) ) &
                chip::RendezvousParameters::SetAdvertisementDelegate,
            "C++: chip::RendezvousParameters::SetAdvertisementDelegate(class chip::RendezvousAdvertisementDelegate *) --> class "
            "chip::RendezvousParameters &",
            pybind11::return_value_policy::automatic, pybind11::arg("delegate"));
        cl.def("HasBleLayer", (bool (chip::RendezvousParameters::*)() const) & chip::RendezvousParameters::HasBleLayer,
               "C++: chip::RendezvousParameters::HasBleLayer() const --> bool");
        cl.def("HasConnectionObject",
               (bool (chip::RendezvousParameters::*)() const) & chip::RendezvousParameters::HasConnectionObject,
               "C++: chip::RendezvousParameters::HasConnectionObject() const --> bool");
        cl.def("GetConnectionObject",
               (void * (chip::RendezvousParameters::*) () const) & chip::RendezvousParameters::GetConnectionObject,
               "C++: chip::RendezvousParameters::GetConnectionObject() const --> void *", pybind11::return_value_policy::automatic);
        cl.def("SetConnectionObject",
               (class chip::RendezvousParameters & (chip::RendezvousParameters::*) (void *) ) &
                   chip::RendezvousParameters::SetConnectionObject,
               "C++: chip::RendezvousParameters::SetConnectionObject(void *) --> class chip::RendezvousParameters &",
               pybind11::return_value_policy::automatic, pybind11::arg("connObj"));
    }
}
