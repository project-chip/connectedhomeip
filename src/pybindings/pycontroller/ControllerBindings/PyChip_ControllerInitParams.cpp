#include <app/Command.h>                                                   // chip::app::Command
#include <app/CommandPathParams.h>                                         // chip::app::CommandPathParams
#include <app/CommandSender.h>                                             // chip::app::CommandSender
#include <app/InteractionModelDelegate.h>                                  // chip::app::InteractionModelDelegate
#include <app/MessageDef/CommandDataElement.h>                             // chip::app::CommandDataElement::Parser
#include <controller/CHIPDevice.h>                                         //
#include <controller/CHIPDevice.h>                                         // chip::Controller::ControllerDeviceInitParams
#include <controller/CHIPDevice.h>                                         // chip::Controller::Device
#include <controller/CHIPDevice.h>                                         // chip::Controller::DeviceStatusDelegate
#include <controller/CHIPDevice.h>                                         // chip::Controller::SerializableDevice
#include <controller/CHIPDevice.h>                                         // chip::Controller::SerializedDevice
#include <controller/OperationalCredentialsDelegate.h>                     // chip::Controller::OperationalCredentialsDelegate
#include <core/CHIPCallback.h>                                             // chip::Callback::Cancelable
#include <core/CHIPTLV.h>                                                  // chip::TLV::TLVWriter
#include <core/PeerId.h>                                                   // chip::PeerId
#include <credentials/CHIPOperationalCredentials.h>                        // chip::Credentials::OperationalCredentialSet
#include <inet/IPAddress.h>                                                // chip::Inet::(anonymous)
#include <inet/IPAddress.h>                                                // chip::Inet::IPAddress
#include <messaging/ExchangeACL.h>                                         // chip::Messaging::ExchangeACL
#include <messaging/ExchangeContext.h>                                     // chip::Messaging::ExchangeContext
#include <messaging/ExchangeDelegate.h>                                    // chip::Messaging::ExchangeDelegate
#include <messaging/ExchangeMessageDispatch.h>                             // chip::Messaging::ExchangeMessageDispatch
#include <messaging/ExchangeMgr.h>                                         // chip::Messaging::ExchangeManager
#include <messaging/ReliableMessageContext.h>                              // chip::Messaging::ReliableMessageContext
#include <messaging/ReliableMessageMgr.h>                                  // chip::Messaging::ReliableMessageMgr
#include <netinet/in.h>                                                    // in_addr
#include <protocols/Protocols.h>                                           // chip::Protocols::Id
#include <protocols/interaction_model/Constants.h>                         // chip::Protocols::InteractionModel::ProtocolCode
#include <protocols/secure_channel/CASESession.h>                          // chip::CASESession
#include <protocols/secure_channel/CASESession.h>                          // chip::CASESessionSerializable
#include <protocols/secure_channel/CASESession.h>                          // chip::CASESessionSerialized
#include <protocols/secure_channel/Constants.h>                            // chip::Protocols::SecureChannel::GeneralStatusCode
#include <protocols/secure_channel/PASESession.h>                          // chip::PASESessionSerializable
#include <protocols/secure_channel/SessionEstablishmentDelegate.h>         // chip::SessionEstablishmentDelegate
#include <protocols/secure_channel/SessionEstablishmentExchangeDispatch.h> // chip::SessionEstablishmentExchangeDispatch
#include <setup_payload/SetupPayload.h>                                    // chip::SetupPayload
#include <sstream>                                                         // __str__
#include <support/Span.h>                                                  // chip::Span
#include <sys/socket.h>                                                    // sockaddr
#include <system/SystemPacketBuffer.h>                                     // chip::System::PacketBufferHandle
#include <transport/AdminPairingTable.h>                                   // chip::Transport::AdminPairingTable
#include <transport/PeerConnectionState.h>                                 // chip::Transport::PeerConnectionState
#include <transport/SecureSession.h>                                       //
#include <transport/SecureSession.h>                                       // chip::SecureSession
#include <transport/SecureSessionHandle.h>                                 // chip::SecureSessionHandle
#include <transport/SecureSessionMgr.h>                                    // chip::SecureSessionMgr
#include <transport/raw/PeerAddress.h>                                     // chip::Transport::PeerAddress
#include <transport/raw/PeerAddress.h>                                     // chip::Transport::Type

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

// chip::Controller::Device file:controller/CHIPDevice.h line:88
struct PyCallBack_chip_Controller_Device : public chip::Controller::Device
{
    using chip::Controller::Device::Device;

    void OnSessionEstablishmentError(int a0) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::Controller::Device *>(this), "OnSessionEstablishmentError");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value)
            {
                static pybind11::detail::override_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<void>(std::move(o));
        }
        return Device::OnSessionEstablishmentError(a0);
    }
    void OnSessionEstablished() override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::Controller::Device *>(this), "OnSessionEstablished");
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
        return Device::OnSessionEstablished();
    }
};

void bind_PyChip_ControllerInitParams(std::function<pybind11::module &(std::string const & namespace_)> & M)
{
    { // chip::Controller::ControllerDeviceInitParams file:controller/CHIPDevice.h line:75
        pybind11::class_<chip::Controller::ControllerDeviceInitParams,
                         std::shared_ptr<chip::Controller::ControllerDeviceInitParams>>
            cl(M("chip::Controller"), "ControllerDeviceInitParams", "");
        cl.def(pybind11::init([]() { return new chip::Controller::ControllerDeviceInitParams(); }));
        cl.def(pybind11::init([](chip::Controller::ControllerDeviceInitParams const & o) {
            return new chip::Controller::ControllerDeviceInitParams(o);
        }));
    }
    { // chip::Controller::Device file:controller/CHIPDevice.h line:88
        pybind11::class_<chip::Controller::Device, std::shared_ptr<chip::Controller::Device>, PyCallBack_chip_Controller_Device> cl(
            M("chip::Controller"), "Device", "");
        cl.def(pybind11::init([]() { return new chip::Controller::Device(); },
                              []() { return new PyCallBack_chip_Controller_Device(); }));

        pybind11::enum_<chip::Controller::Device::PairingWindowOption>(cl, "PairingWindowOption", "")
            .value("kOriginalSetupCode", chip::Controller::Device::PairingWindowOption::kOriginalSetupCode)
            .value("kTokenWithRandomPIN", chip::Controller::Device::PairingWindowOption::kTokenWithRandomPIN)
            .value("kTokenWithProvidedPIN", chip::Controller::Device::PairingWindowOption::kTokenWithProvidedPIN);

        cl.def("Init",
               (void (chip::Controller::Device::*)(struct chip::Controller::ControllerDeviceInitParams, unsigned short,
                                                   unsigned short)) &
                   chip::Controller::Device::Init,
               "Initialize the device object with secure session manager and inet layer object\n   references. This variant of "
               "function is typically used when the device object\n   is created from a serialized device information. The other "
               "parameters (address, port,\n   interface etc) are part of the serialized device, so those are not required to be\n "
               "  initialized.\n\n   Note: The lifetime of session manager and inet layer objects must be longer than\n   that of "
               "this device object. If these objects are freed, while the device object is\n   still using them, it can lead to "
               "unknown behavior and crashes.\n\n \n       Wrapper object for transport manager etc.\n \n\n   Port on which "
               "controller is listening (typically CHIP_PORT)\n \n\n        Local administrator that's initializing this device "
               "object\n\nC++: chip::Controller::Device::Init(struct chip::Controller::ControllerDeviceInitParams, unsigned short, "
               "unsigned short) --> void",
               pybind11::arg("params"), pybind11::arg("listenPort"), pybind11::arg("admin"));
        cl.def("Serialize",
               (int (chip::Controller::Device::*)(struct chip::Controller::SerializedDevice &)) &
                   chip::Controller::Device::Serialize,
               "Serialize the Pairing Session to a string. It's guaranteed that the string\n         will be null terminated, and "
               "there won't be any embedded null characters.\n\n \n Returns a CHIP_ERROR on error, CHIP_NO_ERROR otherwise\n\nC++: "
               "chip::Controller::Device::Serialize(struct chip::Controller::SerializedDevice &) --> int",
               pybind11::arg("output"));
        cl.def("Deserialize",
               (int (chip::Controller::Device::*)(const struct chip::Controller::SerializedDevice &)) &
                   chip::Controller::Device::Deserialize,
               "Deserialize the Pairing Session from the string. It's expected that the string\n         will be null terminated, "
               "and there won't be any embedded null characters.\n\n \n Returns a CHIP_ERROR on error, CHIP_NO_ERROR "
               "otherwise\n\nC++: chip::Controller::Device::Deserialize(const struct chip::Controller::SerializedDevice &) --> int",
               pybind11::arg("input"));
        cl.def("Persist", (int (chip::Controller::Device::*)()) & chip::Controller::Device::Persist,
               "Serialize and store the Device in persistent storage\n\n \n Returns a CHIP_ERROR if either serialization or "
               "storage fails\n\nC++: chip::Controller::Device::Persist() --> int");
        cl.def("OpenPairingWindow",
               (int (chip::Controller::Device::*)(unsigned int, enum chip::Controller::Device::PairingWindowOption,
                                                  class chip::SetupPayload &)) &
                   chip::Controller::Device::OpenPairingWindow,
               "Trigger a paired device to re-enter the pairing mode. If an onboarding token is provided, the device will use\n   "
               "the provided setup PIN code and the discriminator to advertise itself for pairing availability. If the token\n   "
               "is not provided, the device will use the manufacturer assigned setup PIN code and discriminator.\n\n   The device "
               "will exit the pairing mode after a successful pairing, or after the given `timeout` time.\n\n \n         The "
               "pairing mode should terminate after this much time.\n \n\n          The pairing window can be opened using the "
               "original setup code, or an\n                            onboarding token can be generated using a random setup PIN "
               "code (or with\n                            the PIN code provied in the setupPayload). This argument selects one of "
               "these\n                            methods.\n \n\n   The setup payload corresponding to the generated onboarding "
               "token.\n\n \n CHIP_ERROR               CHIP_NO_ERROR on success, or corresponding error\n\nC++: "
               "chip::Controller::Device::OpenPairingWindow(unsigned int, enum chip::Controller::Device::PairingWindowOption, "
               "class chip::SetupPayload &) --> int",
               pybind11::arg("timeout"), pybind11::arg("option"), pybind11::arg("setupPayload"));
        cl.def("UpdateAddress",
               (int (chip::Controller::Device::*)(const class chip::Transport::PeerAddress &)) &
                   chip::Controller::Device::UpdateAddress,
               "Update address of the device.\n\n   This function will set new IP address and port of the device. Since the device "
               "settings might\n   have been moved from RAM to the persistent storage, the function will load the device "
               "settings\n   first, before making the changes.\n\n \n   Address of the device to be set.\n\n \n CHIP_NO_ERROR if "
               "the address has been updated, an error code otherwise.\n\nC++: chip::Controller::Device::UpdateAddress(const class "
               "chip::Transport::PeerAddress &) --> int",
               pybind11::arg("addr"));
        cl.def(
            "IsActive", (bool (chip::Controller::Device::*)() const) & chip::Controller::Device::IsActive,
            "Return whether the current device object is actively associated with a paired CHIP\n   device. An active object can "
            "be used to communicate with the corresponding device.\n\nC++: chip::Controller::Device::IsActive() const --> bool");
        cl.def("SetActive", (void (chip::Controller::Device::*)(bool)) & chip::Controller::Device::SetActive,
               "C++: chip::Controller::Device::SetActive(bool) --> void", pybind11::arg("active"));
        cl.def("IsSecureConnected", (bool (chip::Controller::Device::*)() const) & chip::Controller::Device::IsSecureConnected,
               "C++: chip::Controller::Device::IsSecureConnected() const --> bool");
        cl.def("Reset", (void (chip::Controller::Device::*)()) & chip::Controller::Device::Reset,
               "C++: chip::Controller::Device::Reset() --> void");
        cl.def("GetDeviceId", (unsigned long long (chip::Controller::Device::*)() const) & chip::Controller::Device::GetDeviceId,
               "C++: chip::Controller::Device::GetDeviceId() const --> unsigned long long");
        cl.def("GetNextSequenceNumber",
               (unsigned char (chip::Controller::Device::*)()) & chip::Controller::Device::GetNextSequenceNumber,
               "C++: chip::Controller::Device::GetNextSequenceNumber() --> unsigned char");
        cl.def("CancelResponseHandler",
               (void (chip::Controller::Device::*)(unsigned char)) & chip::Controller::Device::CancelResponseHandler,
               "C++: chip::Controller::Device::CancelResponseHandler(unsigned char) --> void", pybind11::arg("seqNum"));
        cl.def("OnSessionEstablished", (void (chip::Controller::Device::*)()) & chip::Controller::Device::OnSessionEstablished,
               "C++: chip::Controller::Device::OnSessionEstablished() --> void");
    }
    { // chip::Controller::SerializableDevice file:controller/CHIPDevice.h line:481
        pybind11::class_<chip::Controller::SerializableDevice, std::shared_ptr<chip::Controller::SerializableDevice>> cl(
            M("chip::Controller"), "SerializableDevice", "");
        cl.def(pybind11::init([]() { return new chip::Controller::SerializableDevice(); }));
        cl.def_readwrite("mOpsCreds", &chip::Controller::SerializableDevice::mOpsCreds);
        cl.def_readwrite("mDeviceId", &chip::Controller::SerializableDevice::mDeviceId);
        cl.def_readwrite("mDevicePort", &chip::Controller::SerializableDevice::mDevicePort);
        cl.def_readwrite("mAdminId", &chip::Controller::SerializableDevice::mAdminId);
        cl.def_readwrite("mDeviceTransport", &chip::Controller::SerializableDevice::mDeviceTransport);
        cl.def_readwrite("mLocalMessageCounter", &chip::Controller::SerializableDevice::mLocalMessageCounter);
        cl.def_readwrite("mPeerMessageCounter", &chip::Controller::SerializableDevice::mPeerMessageCounter);
    }
    { // chip::Controller::SerializedDevice file:controller/CHIPDevice.h line:496
        pybind11::class_<chip::Controller::SerializedDevice, std::shared_ptr<chip::Controller::SerializedDevice>> cl(
            M("chip::Controller"), "SerializedDevice", "");
        cl.def(pybind11::init([]() { return new chip::Controller::SerializedDevice(); }));
    }
}
