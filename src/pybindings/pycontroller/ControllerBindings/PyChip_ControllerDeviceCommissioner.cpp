#include <app/Command.h>                                   // chip::app::Command
#include <app/CommandPathParams.h>                         // chip::app::CommandPathParams
#include <app/CommandSender.h>                             // chip::app::CommandSender
#include <app/InteractionModelDelegate.h>                  // chip::app::InteractionModelDelegate
#include <app/MessageDef/CommandDataElement.h>             // chip::app::CommandDataElement::Parser
#include <ble/BleLayer.h>                                  // chip::Ble::BleLayer
#include <controller/CHIPDevice.h>                         //
#include <controller/CHIPDevice.h>                         // chip::Controller::ControllerDeviceInitParams
#include <controller/CHIPDevice.h>                         // chip::Controller::Device
#include <controller/CHIPDevice.h>                         // chip::Controller::DeviceStatusDelegate
#include <controller/CHIPDevice.h>                         // chip::Controller::SerializedDevice
#include <controller/CHIPDeviceController.h>               //
#include <controller/CHIPDeviceController.h>               // chip::Controller::BasicFailure
#include <controller/CHIPDeviceController.h>               // chip::Controller::BasicSuccess
#include <controller/CHIPDeviceController.h>               // chip::Controller::CommissionerInitParams
#include <controller/CHIPDeviceController.h>               // chip::Controller::CommissioningStage
#include <controller/CHIPDeviceController.h>               // chip::Controller::ControllerInitParams
#include <controller/CHIPDeviceController.h>               // chip::Controller::DeviceCommissioner
#include <controller/CHIPDeviceController.h>               // chip::Controller::DeviceCommissionerRendezvousAdvertisementDelegate
#include <controller/CHIPDeviceController.h>               // chip::Controller::DeviceController
#include <controller/CHIPDeviceController.h>               // chip::Controller::DeviceControllerInteractionModelDelegate
#include <controller/CHIPDeviceController.h>               // chip::Controller::DevicePairingDelegate
#include <core/CHIPCallback.h>                             // chip::Callback::Cancelable
#include <core/CHIPTLV.h>                                  // chip::TLV::TLVWriter
#include <core/Optional.h>                                 // chip::Optional
#include <credentials/CHIPOperationalCredentials.h>        // chip::Credentials::OperationalCredentialSet
#include <inet/IPAddress.h>                                // chip::Inet::(anonymous)
#include <inet/IPAddress.h>                                // chip::Inet::IPAddress
#include <messaging/ExchangeACL.h>                         // chip::Messaging::ExchangeACL
#include <messaging/ExchangeContext.h>                     // chip::Messaging::ExchangeContext
#include <messaging/ExchangeDelegate.h>                    // chip::Messaging::ExchangeDelegate
#include <messaging/ExchangeMessageDispatch.h>             // chip::Messaging::ExchangeMessageDispatch
#include <messaging/ExchangeMgr.h>                         // chip::Messaging::ExchangeManager
#include <messaging/ReliableMessageContext.h>              // chip::Messaging::ReliableMessageContext
#include <messaging/ReliableMessageMgr.h>                  // chip::Messaging::ReliableMessageMgr
#include <netinet/in.h>                                    // in_addr
#include <protocols/Protocols.h>                           // chip::Protocols::Id
#include <protocols/interaction_model/Constants.h>         // chip::Protocols::InteractionModel::ProtocolCode
#include <protocols/secure_channel/CASESession.h>          // chip::CASESession
#include <protocols/secure_channel/CASESession.h>          // chip::CASESessionSerializable
#include <protocols/secure_channel/CASESession.h>          // chip::CASESessionSerialized
#include <protocols/secure_channel/Constants.h>            // chip::Protocols::SecureChannel::GeneralStatusCode
#include <protocols/secure_channel/PASESession.h>          // chip::PASESessionSerializable
#include <protocols/secure_channel/RendezvousParameters.h> // chip::RendezvousAdvertisementDelegate
#include <protocols/secure_channel/RendezvousParameters.h> // chip::RendezvousParameters
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

// chip::app::InteractionModelDelegate file:app/InteractionModelDelegate.h line:44
struct PyCallBack_chip_app_InteractionModelDelegate : public chip::app::InteractionModelDelegate
{
    using chip::app::InteractionModelDelegate::InteractionModelDelegate;

    int EventStreamReceived(const class chip::Messaging::ExchangeContext * a0, class chip::TLV::TLVReader * a1) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::app::InteractionModelDelegate *>(this), "EventStreamReceived");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        return InteractionModelDelegate::EventStreamReceived(a0, a1);
    }
    int ReportProcessed(const class chip::app::ReadClient * a0) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::app::InteractionModelDelegate *>(this), "ReportProcessed");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        return InteractionModelDelegate::ReportProcessed(a0);
    }
    int ReportError(const class chip::app::ReadClient * a0, int a1) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::app::InteractionModelDelegate *>(this), "ReportError");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        return InteractionModelDelegate::ReportError(a0, a1);
    }
    int CommandResponseStatus(const chip::app::CommandSender * a0, const chip::Protocols::SecureChannel::GeneralStatusCode a1,
                              const uint32_t a2, const uint16_t a3, chip::EndpointId a4, const chip::ClusterId a5,
                              chip::CommandId a6, uint8_t a7) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::app::InteractionModelDelegate *>(this), "CommandResponseStatus");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2, a3, a4, a5, a6, a7);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        return InteractionModelDelegate::CommandResponseStatus(a0, a1, a2, a3, a4, a5, a6, a7);
    }
    int CommandResponseProcessed(const class chip::app::CommandSender * a0) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::app::InteractionModelDelegate *>(this), "CommandResponseProcessed");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        return InteractionModelDelegate::CommandResponseProcessed(a0);
    }
    int CommandResponseProtocolError(const class chip::app::CommandSender * a0, unsigned char a1) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::app::InteractionModelDelegate *>(this), "CommandResponseProtocolError");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        return InteractionModelDelegate::CommandResponseProtocolError(a0, a1);
    }
    int CommandResponseError(const class chip::app::CommandSender * a0, int a1) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::app::InteractionModelDelegate *>(this), "CommandResponseError");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        return InteractionModelDelegate::CommandResponseError(a0, a1);
    }
    int WriteResponseStatus(const class chip::app::WriteClient * a0,
                            const enum chip::Protocols::SecureChannel::GeneralStatusCode a1, const unsigned int a2,
                            const unsigned short a3, struct chip::app::AttributePathParams & a4, unsigned char a5) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::app::InteractionModelDelegate *>(this), "WriteResponseStatus");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2, a3, a4, a5);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        return InteractionModelDelegate::WriteResponseStatus(a0, a1, a2, a3, a4, a5);
    }
    int WriteResponseProcessed(const class chip::app::WriteClient * a0) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::app::InteractionModelDelegate *>(this), "WriteResponseProcessed");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        return InteractionModelDelegate::WriteResponseProcessed(a0);
    }
    int WriteResponseProtocolError(const class chip::app::WriteClient * a0, unsigned char a1) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::app::InteractionModelDelegate *>(this), "WriteResponseProtocolError");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        return InteractionModelDelegate::WriteResponseProtocolError(a0, a1);
    }
    int WriteResponseError(const class chip::app::WriteClient * a0, int a1) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::app::InteractionModelDelegate *>(this), "WriteResponseError");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        return InteractionModelDelegate::WriteResponseError(a0, a1);
    }
};

// chip::Controller::DevicePairingDelegate file:controller/CHIPDeviceController.h line:112
struct PyCallBack_chip_Controller_DevicePairingDelegate : public chip::Controller::DevicePairingDelegate
{
    using chip::Controller::DevicePairingDelegate::DevicePairingDelegate;

    void OnStatusUpdate(enum chip::Controller::DevicePairingDelegate::Status a0) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::Controller::DevicePairingDelegate *>(this), "OnStatusUpdate");
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
        return DevicePairingDelegate::OnStatusUpdate(a0);
    }
    void OnPairingComplete(int a0) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::Controller::DevicePairingDelegate *>(this), "OnPairingComplete");
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
        return DevicePairingDelegate::OnPairingComplete(a0);
    }
    void OnPairingDeleted(int a0) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::Controller::DevicePairingDelegate *>(this), "OnPairingDeleted");
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
        return DevicePairingDelegate::OnPairingDeleted(a0);
    }
};
// chip::Controller::DeviceControllerInteractionModelDelegate file:controller/CHIPDeviceController.h line:158
struct PyCallBack_chip_Controller_DeviceControllerInteractionModelDelegate
    : public chip::Controller::DeviceControllerInteractionModelDelegate
{
    using chip::Controller::DeviceControllerInteractionModelDelegate::DeviceControllerInteractionModelDelegate;
    // const app::CommandSender * apCommandSender,
    //                                      const Protocols::SecureChannel::GeneralStatusCode aGeneralCode, const uint32_t
    //                                      aProtocolId, const uint16_t aProtocolCode, chip::EndpointId aEndpointId, const
    //                                      chip::ClusterId aClusterId, chip::CommandId aCommandId, uint8_t aCommandIndex
    // //
    int CommandResponseStatus(const class chip::app::CommandSender * a0,
                              const enum chip::Protocols::SecureChannel::GeneralStatusCode a1, const uint32_t a2, const uint16_t a3,
                              chip::EndpointId a4, const chip::ClusterId a5, chip::CommandId a6, unsigned char a7) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(
            static_cast<const chip::Controller::DeviceControllerInteractionModelDelegate *>(this), "CommandResponseStatus");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2, a3, a4, a5, a6, a7);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        return DeviceControllerInteractionModelDelegate::CommandResponseStatus(a0, a1, a2, a3, a4, a5, a6, a7);
    }
    int CommandResponseProtocolError(const class chip::app::CommandSender * a0, unsigned char a1) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(
            static_cast<const chip::Controller::DeviceControllerInteractionModelDelegate *>(this), "CommandResponseProtocolError");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        return DeviceControllerInteractionModelDelegate::CommandResponseProtocolError(a0, a1);
    }
    int CommandResponseError(const class chip::app::CommandSender * a0, int a1) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(
            static_cast<const chip::Controller::DeviceControllerInteractionModelDelegate *>(this), "CommandResponseError");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        return DeviceControllerInteractionModelDelegate::CommandResponseError(a0, a1);
    }
    int CommandResponseProcessed(const class chip::app::CommandSender * a0) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(
            static_cast<const chip::Controller::DeviceControllerInteractionModelDelegate *>(this), "CommandResponseProcessed");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        return DeviceControllerInteractionModelDelegate::CommandResponseProcessed(a0);
    }
    int EventStreamReceived(const class chip::Messaging::ExchangeContext * a0, class chip::TLV::TLVReader * a1) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(
            static_cast<const chip::Controller::DeviceControllerInteractionModelDelegate *>(this), "EventStreamReceived");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        return InteractionModelDelegate::EventStreamReceived(a0, a1);
    }
    int ReportProcessed(const class chip::app::ReadClient * a0) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(
            static_cast<const chip::Controller::DeviceControllerInteractionModelDelegate *>(this), "ReportProcessed");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        return InteractionModelDelegate::ReportProcessed(a0);
    }
    int ReportError(const class chip::app::ReadClient * a0, int a1) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(
            static_cast<const chip::Controller::DeviceControllerInteractionModelDelegate *>(this), "ReportError");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        return InteractionModelDelegate::ReportError(a0, a1);
    }
    int WriteResponseStatus(const class chip::app::WriteClient * a0,
                            const enum chip::Protocols::SecureChannel::GeneralStatusCode a1, const unsigned int a2,
                            const unsigned short a3, struct chip::app::AttributePathParams & a4, unsigned char a5) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(
            static_cast<const chip::Controller::DeviceControllerInteractionModelDelegate *>(this), "WriteResponseStatus");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2, a3, a4, a5);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        return InteractionModelDelegate::WriteResponseStatus(a0, a1, a2, a3, a4, a5);
    }
    int WriteResponseProcessed(const class chip::app::WriteClient * a0) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(
            static_cast<const chip::Controller::DeviceControllerInteractionModelDelegate *>(this), "WriteResponseProcessed");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        return InteractionModelDelegate::WriteResponseProcessed(a0);
    }
    int WriteResponseProtocolError(const class chip::app::WriteClient * a0, unsigned char a1) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(
            static_cast<const chip::Controller::DeviceControllerInteractionModelDelegate *>(this), "WriteResponseProtocolError");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        return InteractionModelDelegate::WriteResponseProtocolError(a0, a1);
    }
    int WriteResponseError(const class chip::app::WriteClient * a0, int a1) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(
            static_cast<const chip::Controller::DeviceControllerInteractionModelDelegate *>(this), "WriteResponseError");
        if (overload)
        {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value)
            {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            else
                return pybind11::detail::cast_safe<int>(std::move(o));
        }
        return InteractionModelDelegate::WriteResponseError(a0, a1);
    }
};

// chip::Controller::DeviceController file:controller/CHIPDeviceController.h line:181
struct PyCallBack_chip_Controller_DeviceController : public chip::Controller::DeviceController
{
    using chip::Controller::DeviceController::DeviceController;

    int Shutdown() override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::Controller::DeviceController *>(this), "Shutdown");
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
        return DeviceController::Shutdown();
    }
    void ReleaseDevice(class chip::Controller::Device * a0) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::Controller::DeviceController *>(this), "ReleaseDevice");
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
        return DeviceController::ReleaseDevice(a0);
    }
};

// chip::Controller::DeviceCommissionerRendezvousAdvertisementDelegate file:controller/CHIPDeviceController.h line:366
struct PyCallBack_chip_Controller_DeviceCommissionerRendezvousAdvertisementDelegate
    : public chip::Controller::DeviceCommissionerRendezvousAdvertisementDelegate
{
    using chip::Controller::DeviceCommissionerRendezvousAdvertisementDelegate::DeviceCommissionerRendezvousAdvertisementDelegate;

    int StartAdvertisement() const override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(
            static_cast<const chip::Controller::DeviceCommissionerRendezvousAdvertisementDelegate *>(this), "StartAdvertisement");
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
        return DeviceCommissionerRendezvousAdvertisementDelegate::StartAdvertisement();
    }
    int StopAdvertisement() const override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(
            static_cast<const chip::Controller::DeviceCommissionerRendezvousAdvertisementDelegate *>(this), "StopAdvertisement");
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
        return DeviceCommissionerRendezvousAdvertisementDelegate::StopAdvertisement();
    }
    void RendezvousComplete() const override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(
            static_cast<const chip::Controller::DeviceCommissionerRendezvousAdvertisementDelegate *>(this), "RendezvousComplete");
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

// chip::Controller::DeviceCommissioner file:controller/CHIPDeviceController.h line:388
struct PyCallBack_chip_Controller_DeviceCommissioner : public chip::Controller::DeviceCommissioner
{
    using chip::Controller::DeviceCommissioner::DeviceCommissioner;

    int Shutdown() override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::Controller::DeviceCommissioner *>(this), "Shutdown");
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
        return DeviceCommissioner::Shutdown();
    }
    void OnSessionEstablishmentError(int a0) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::Controller::DeviceCommissioner *>(this), "OnSessionEstablishmentError");
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
        return DeviceCommissioner::OnSessionEstablishmentError(a0);
    }
    void OnSessionEstablished() override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::Controller::DeviceCommissioner *>(this), "OnSessionEstablished");
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
        return DeviceCommissioner::OnSessionEstablished();
    }
    void ReleaseDevice(class chip::Controller::Device * a0) override
    {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload =
            pybind11::get_overload(static_cast<const chip::Controller::DeviceCommissioner *>(this), "ReleaseDevice");
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
        return DeviceCommissioner::ReleaseDevice(a0);
    }
};

void bind_PyChip_ControllerDeviceCommissioner(std::function<pybind11::module &(std::string const & namespace_)> & M)
{
    // chip::Controller::BasicSuccess(void *, unsigned short) file:controller/CHIPDeviceController.h line:74
    M("chip::Controller")
        .def("BasicSuccess", (void (*)(void *, unsigned short)) & chip::Controller::BasicSuccess,
             "C++: chip::Controller::BasicSuccess(void *, unsigned short) --> void", pybind11::arg("context"),
             pybind11::arg("val"));

    // chip::Controller::BasicFailure(void *, unsigned char) file:controller/CHIPDeviceController.h line:75
    M("chip::Controller")
        .def("BasicFailure", (void (*)(void *, unsigned char)) & chip::Controller::BasicFailure,
             "C++: chip::Controller::BasicFailure(void *, unsigned char) --> void", pybind11::arg("context"),
             pybind11::arg("status"));

    { // chip::Controller::ControllerInitParams file:controller/CHIPDeviceController.h line:77
        pybind11::class_<chip::Controller::ControllerInitParams, std::shared_ptr<chip::Controller::ControllerInitParams>> cl(
            M("chip::Controller"), "ControllerInitParams", "");
        cl.def(pybind11::init([]() { return new chip::Controller::ControllerInitParams(); }));
        cl.def_readwrite("storageDelegate", &chip::Controller::ControllerInitParams::storageDelegate);
        cl.def_readwrite("systemLayer", &chip::Controller::ControllerInitParams::systemLayer);
        cl.def_readwrite("inetLayer", &chip::Controller::ControllerInitParams::inetLayer);
        cl.def_readwrite("bleLayer", &chip::Controller::ControllerInitParams::bleLayer);
        cl.def_readwrite("imDelegate", &chip::Controller::ControllerInitParams::imDelegate);
        cl.def_readwrite("mDeviceAddressUpdateDelegate", &chip::Controller::ControllerInitParams::mDeviceAddressUpdateDelegate);
        cl.def_readwrite("operationalCredentialsDelegate", &chip::Controller::ControllerInitParams::operationalCredentialsDelegate);
    }

    // chip::Controller::CommissioningStage file:controller/CHIPDeviceController.h line:93
    pybind11::enum_<chip::Controller::CommissioningStage>(M("chip::Controller"), "CommissioningStage", pybind11::arithmetic(), "")
        .value("kError", chip::Controller::kError)
        .value("kSecurePairing", chip::Controller::kSecurePairing)
        .value("kArmFailsafe", chip::Controller::kArmFailsafe)
        .value("kConfigRegulatory", chip::Controller::kConfigRegulatory)
        .value("kCheckCertificates", chip::Controller::kCheckCertificates)
        .value("kConfigACL", chip::Controller::kConfigACL)
        .value("kNetworkSetup", chip::Controller::kNetworkSetup)
        .value("kScanNetworks", chip::Controller::kScanNetworks)
        .value("kNetworkEnable", chip::Controller::kNetworkEnable)
        .value("kFindOperational", chip::Controller::kFindOperational)
        .value("kSendComplete", chip::Controller::kSendComplete)
        .value("kCleanup", chip::Controller::kCleanup)
        .export_values();

    ;

    { // chip::Controller::DevicePairingDelegate file:controller/CHIPDeviceController.h line:112
        pybind11::class_<chip::Controller::DevicePairingDelegate, std::shared_ptr<chip::Controller::DevicePairingDelegate>,
                         PyCallBack_chip_Controller_DevicePairingDelegate>
            cl(M("chip::Controller"), "DevicePairingDelegate", "");
        cl.def(pybind11::init([]() { return new chip::Controller::DevicePairingDelegate(); },
                              []() { return new PyCallBack_chip_Controller_DevicePairingDelegate(); }));

        pybind11::enum_<chip::Controller::DevicePairingDelegate::Status>(cl, "Status", pybind11::arithmetic(), "")
            .value("SecurePairingSuccess", chip::Controller::DevicePairingDelegate::SecurePairingSuccess)
            .value("SecurePairingFailed", chip::Controller::DevicePairingDelegate::SecurePairingFailed)
            .export_values();

        cl.def("OnStatusUpdate",
               (void (chip::Controller::DevicePairingDelegate::*)(enum chip::Controller::DevicePairingDelegate::Status)) &
                   chip::Controller::DevicePairingDelegate::OnStatusUpdate,
               "Called when the pairing reaches a certain stage.\n\n \n Current status of pairing\n\nC++: "
               "chip::Controller::DevicePairingDelegate::OnStatusUpdate(enum chip::Controller::DevicePairingDelegate::Status) --> "
               "void",
               pybind11::arg("status"));
        cl.def("OnPairingComplete",
               (void (chip::Controller::DevicePairingDelegate::*)(int)) &
                   chip::Controller::DevicePairingDelegate::OnPairingComplete,
               "Called when the pairing is complete (with success or error)\n\n \n Error cause, if any\n\nC++: "
               "chip::Controller::DevicePairingDelegate::OnPairingComplete(int) --> void",
               pybind11::arg("error"));
        cl.def("OnPairingDeleted",
               (void (chip::Controller::DevicePairingDelegate::*)(int)) & chip::Controller::DevicePairingDelegate::OnPairingDeleted,
               "Called when the pairing is deleted (with success or error)\n\n \n Error cause, if any\n\nC++: "
               "chip::Controller::DevicePairingDelegate::OnPairingDeleted(int) --> void",
               pybind11::arg("error"));
        cl.def("assign",
               (class chip::Controller::DevicePairingDelegate &
                (chip::Controller::DevicePairingDelegate::*) (const class chip::Controller::DevicePairingDelegate &) ) &
                   chip::Controller::DevicePairingDelegate::operator=,
               "C++: chip::Controller::DevicePairingDelegate::operator=(const class chip::Controller::DevicePairingDelegate &) --> "
               "class chip::Controller::DevicePairingDelegate &",
               pybind11::return_value_policy::automatic, pybind11::arg(""));
    }
    { // chip::Controller::CommissionerInitParams file:controller/CHIPDeviceController.h line:148
        pybind11::class_<chip::Controller::CommissionerInitParams, std::shared_ptr<chip::Controller::CommissionerInitParams>,
                         chip::Controller::ControllerInitParams>
            cl(M("chip::Controller"), "CommissionerInitParams", "");
        cl.def(pybind11::init([]() { return new chip::Controller::CommissionerInitParams(); }));
        cl.def_readwrite("pairingDelegate", &chip::Controller::CommissionerInitParams::pairingDelegate);
        cl.def_readwrite("storageDelegate", &chip::Controller::ControllerInitParams::storageDelegate);
        cl.def_readwrite("systemLayer", &chip::Controller::ControllerInitParams::systemLayer);
        cl.def_readwrite("inetLayer", &chip::Controller::ControllerInitParams::inetLayer);
        cl.def_readwrite("bleLayer", &chip::Controller::ControllerInitParams::bleLayer);
        cl.def_readwrite("imDelegate", &chip::Controller::ControllerInitParams::imDelegate);
        cl.def_readwrite("mDeviceAddressUpdateDelegate", &chip::Controller::ControllerInitParams::mDeviceAddressUpdateDelegate);
        cl.def_readwrite("operationalCredentialsDelegate", &chip::Controller::ControllerInitParams::operationalCredentialsDelegate);
    }
    { // chip::app::InteractionModelDelegate file:app/InteractionModelDelegate.h line:44
        pybind11::class_<chip::app::InteractionModelDelegate, std::shared_ptr<chip::app::InteractionModelDelegate>,
                         PyCallBack_chip_app_InteractionModelDelegate>
            cl(M("chip::app"), "InteractionModelDelegate",
               "This class defines the API for a delegate that an SDK consumer can use to interface with the interaction model.");
        cl.def(pybind11::init([]() { return new chip::app::InteractionModelDelegate(); },
                              []() { return new PyCallBack_chip_app_InteractionModelDelegate(); }));
        cl.def(pybind11::init([](PyCallBack_chip_app_InteractionModelDelegate const & o) {
            return new PyCallBack_chip_app_InteractionModelDelegate(o);
        }));
        cl.def(pybind11::init(
            [](chip::app::InteractionModelDelegate const & o) { return new chip::app::InteractionModelDelegate(o); }));
        cl.def("EventStreamReceived",
               (int (chip::app::InteractionModelDelegate::*)(const class chip::Messaging::ExchangeContext *,
                                                             class chip::TLV::TLVReader *)) &
                   chip::app::InteractionModelDelegate::EventStreamReceived,
               "Notification that the interaction model has received a list of events in response to a Read request and that "
               "list\n of events needs to be processed.\n \n\n   An exchange context that represents the exchange the Report Data "
               "came in on.\n                                 This can be used to recover the NodeId of the node that sent the "
               "Report Data.\n                                 It is managed externally and should not be closed by the SDK "
               "consumer.\n \n\n  TLV reader positioned at the list that contains the events.  The\n                               "
               " implementation of EventStreamReceived is expected to call Next() on the reader to\n                               "
               " advance it to the first element of the list, then process the elements from beginning to the\n                    "
               "            end. The callee is expected to consume all events.\n\n   # CHIP_ERROR_NOT_IMPLEMENTED if not "
               "implemented\n\nC++: chip::app::InteractionModelDelegate::EventStreamReceived(const class "
               "chip::Messaging::ExchangeContext *, class chip::TLV::TLVReader *) --> int",
               pybind11::arg("apExchangeContext"), pybind11::arg("apEventListReader"));
        cl.def("ReportProcessed",
               (int (chip::app::InteractionModelDelegate::*)(const class chip::app::ReadClient *)) &
                   chip::app::InteractionModelDelegate::ReportProcessed,
               "Notification that the last message for a Report Data action for the given ReadClient has been received and "
               "processed.\n \n\n   A current readClient which can identify the read to the consumer, particularly during\n        "
               "                    multiple read interactions\n  # CHIP_ERROR_NOT_IMPLEMENTED if not implemented\n\nC++: "
               "chip::app::InteractionModelDelegate::ReportProcessed(const class chip::app::ReadClient *) --> int",
               pybind11::arg("apReadClient"));
        cl.def("ReportError",
               (int (chip::app::InteractionModelDelegate::*)(const class chip::app::ReadClient *, int)) &
                   chip::app::InteractionModelDelegate::ReportError,
               "Notification that a read attempt encountered an asynchronous failure.\n \n\n   A current readClient which can "
               "identify the read to the consumer, particularly during\n                            multiple read interactions\n "
               "\n\n         A error that could be CHIP_ERROR_TIMEOUT when read client fails to receive, or other error when\n     "
               "                       fail to process report data.\n  # CHIP_ERROR_NOT_IMPLEMENTED if not implemented\n\nC++: "
               "chip::app::InteractionModelDelegate::ReportError(const class chip::app::ReadClient *, int) --> int",
               pybind11::arg("apReadClient"), pybind11::arg("aError"));
        cl.def("CommandResponseStatus",
               (int (chip::app::InteractionModelDelegate::*)(
                   const class chip::app::CommandSender *, const enum chip::Protocols::SecureChannel::GeneralStatusCode,
                   const unsigned int, const unsigned short, unsigned char, const unsigned short, unsigned char, unsigned char)) &
                   chip::app::InteractionModelDelegate::CommandResponseStatus,
               "Notification that a Command Send has received an Invoke Command Response containing a status code.\n \n\n A "
               "current command sender which can identify the command sender to the consumer, particularly\n during multiple "
               "command interactions\n \n\n   Status code defined by the standard\n \n\n    Protocol Id\n \n\n  Detailed error "
               "information, protocol-specific.\n \n\n    Endpoint identifier\n \n\n     Cluster identifier\n \n\n     Command "
               "identifier\n \n\n  Current processing command index which can identify command if there exists multiple commands "
               "with\n same command Id\n  # CHIP_ERROR_NOT_IMPLEMENTED if not implemented\n\nC++: "
               "chip::app::InteractionModelDelegate::CommandResponseStatus(const class chip::app::CommandSender *, const enum "
               "chip::Protocols::SecureChannel::GeneralStatusCode, const unsigned int, const unsigned short, unsigned char, const "
               "unsigned short, unsigned char, unsigned char) --> int",
               pybind11::arg("apCommandSender"), pybind11::arg("aGeneralCode"), pybind11::arg("aProtocolId"),
               pybind11::arg("aProtocolCode"), pybind11::arg("aEndpointId"), pybind11::arg("aClusterId"),
               pybind11::arg("aCommandId"), pybind11::arg("aCommandIndex"));
        cl.def("CommandResponseProcessed",
               (int (chip::app::InteractionModelDelegate::*)(const class chip::app::CommandSender *)) &
                   chip::app::InteractionModelDelegate::CommandResponseProcessed,
               "Notification that a Command Response has already been processed.\n \n\n A current command sender which can "
               "identify the command sender to the consumer, particularly\n during multiple command interactions\n  # "
               "CHIP_ERROR_NOT_IMPLEMENTED if not implemented\n\nC++: "
               "chip::app::InteractionModelDelegate::CommandResponseProcessed(const class chip::app::CommandSender *) --> int",
               pybind11::arg("apCommandSender"));
        cl.def("CommandResponseProtocolError",
               (int (chip::app::InteractionModelDelegate::*)(const class chip::app::CommandSender *, unsigned char)) &
                   chip::app::InteractionModelDelegate::CommandResponseProtocolError,
               "Notification that a Command Send has received an Invoke Command Response and fails to process a command data "
               "element in that\n command response\n \n\n A current command sender which can identify the command sender to the "
               "consumer, particularly\n during multiple command interactions\n \n\n  Current processing command index which can "
               "identify failed command\n  # CHIP_ERROR_NOT_IMPLEMENTED if not implemented\n\nC++: "
               "chip::app::InteractionModelDelegate::CommandResponseProtocolError(const class chip::app::CommandSender *, unsigned "
               "char) --> int",
               pybind11::arg("apCommandSender"), pybind11::arg("aCommandIndex"));
        cl.def("CommandResponseError",
               (int (chip::app::InteractionModelDelegate::*)(const class chip::app::CommandSender *, int)) &
                   chip::app::InteractionModelDelegate::CommandResponseError,
               "Notification that a command sender encountered an asynchronous failure.\n \n\n A current command sender which can "
               "identify the command sender to the consumer, particularly\n during multiple command interactions\n \n\n         A "
               "error that could be CHIP_ERROR_TIMEOUT when command sender fails to receive, or other error when\n                 "
               "           fail to process command response.\n  # CHIP_ERROR_NOT_IMPLEMENTED if not implemented\n\nC++: "
               "chip::app::InteractionModelDelegate::CommandResponseError(const class chip::app::CommandSender *, int) --> int",
               pybind11::arg("apCommandSender"), pybind11::arg("aError"));
        cl.def("WriteResponseStatus",
               (int (chip::app::InteractionModelDelegate::*)(
                   const class chip::app::WriteClient *, const enum chip::Protocols::SecureChannel::GeneralStatusCode,
                   const unsigned int, const unsigned short, struct chip::app::AttributePathParams &, unsigned char)) &
                   chip::app::InteractionModelDelegate::WriteResponseStatus,
               "Notification that a WriteClient has received an Write Response containing a status code.\n\nC++: "
               "chip::app::InteractionModelDelegate::WriteResponseStatus(const class chip::app::WriteClient *, const enum "
               "chip::Protocols::SecureChannel::GeneralStatusCode, const unsigned int, const unsigned short, struct "
               "chip::app::AttributePathParams &, unsigned char) --> int",
               pybind11::arg("apWriteClient"), pybind11::arg("aGeneralCode"), pybind11::arg("aProtocolId"),
               pybind11::arg("aProtocolCode"), pybind11::arg("aAttributePathParams"), pybind11::arg("aCommandIndex"));
        cl.def("WriteResponseProcessed",
               (int (chip::app::InteractionModelDelegate::*)(const class chip::app::WriteClient *)) &
                   chip::app::InteractionModelDelegate::WriteResponseProcessed,
               "Notification that a Write Response has already been processed.\n\nC++: "
               "chip::app::InteractionModelDelegate::WriteResponseProcessed(const class chip::app::WriteClient *) --> int",
               pybind11::arg("apWriteClient"));
        cl.def("WriteResponseProtocolError",
               (int (chip::app::InteractionModelDelegate::*)(const class chip::app::WriteClient *, unsigned char)) &
                   chip::app::InteractionModelDelegate::WriteResponseProtocolError,
               "Notification that a Write Client has received an Write Response and fails to process a attribute data element in "
               "that\n write response\n\nC++: chip::app::InteractionModelDelegate::WriteResponseProtocolError(const class "
               "chip::app::WriteClient *, unsigned char) --> int",
               pybind11::arg("apWriteClient"), pybind11::arg("aAttributeIndex"));
        cl.def("WriteResponseError",
               (int (chip::app::InteractionModelDelegate::*)(const class chip::app::WriteClient *, int)) &
                   chip::app::InteractionModelDelegate::WriteResponseError,
               "Notification that a write client encountered an asynchronous failure.\n \n\n write interactions\n \n\n         A "
               "error that could be CHIP_ERROR_TIMEOUT when write client fails to receive, or other error when\n                   "
               "         fail to process write response.\n  # CHIP_ERROR_NOT_IMPLEMENTED if not implemented\n\nC++: "
               "chip::app::InteractionModelDelegate::WriteResponseError(const class chip::app::WriteClient *, int) --> int",
               pybind11::arg("apWriteClient"), pybind11::arg("aError"));
        cl.def("assign",
               (class chip::app::InteractionModelDelegate &
                (chip::app::InteractionModelDelegate::*) (const class chip::app::InteractionModelDelegate &) ) &
                   chip::app::InteractionModelDelegate::operator=,
               "C++: chip::app::InteractionModelDelegate::operator=(const class chip::app::InteractionModelDelegate &) --> class "
               "chip::app::InteractionModelDelegate &",
               pybind11::return_value_policy::automatic, pybind11::arg(""));
    }
    { // chip::Controller::DeviceControllerInteractionModelDelegate file:controller/CHIPDeviceController.h line:158
        pybind11::class_<chip::Controller::DeviceControllerInteractionModelDelegate,
                         std::shared_ptr<chip::Controller::DeviceControllerInteractionModelDelegate>,
                         PyCallBack_chip_Controller_DeviceControllerInteractionModelDelegate, chip::app::InteractionModelDelegate>
            cl(M("chip::Controller"), "DeviceControllerInteractionModelDelegate",
               "Used for make current OnSuccessCallback & OnFailureCallback works when interaction model landed, it will be "
               "removed\n after #6308 is landed.");
        cl.def(pybind11::init([]() { return new chip::Controller::DeviceControllerInteractionModelDelegate(); },
                              []() { return new PyCallBack_chip_Controller_DeviceControllerInteractionModelDelegate(); }));
        cl.def(pybind11::init([](PyCallBack_chip_Controller_DeviceControllerInteractionModelDelegate const & o) {
            return new PyCallBack_chip_Controller_DeviceControllerInteractionModelDelegate(o);
        }));
        cl.def(pybind11::init([](chip::Controller::DeviceControllerInteractionModelDelegate const & o) {
            return new chip::Controller::DeviceControllerInteractionModelDelegate(o);
        }));
        cl.def("CommandResponseStatus",
               (int (chip::Controller::DeviceControllerInteractionModelDelegate::*)(
                   const class chip::app::CommandSender *, const enum chip::Protocols::SecureChannel::GeneralStatusCode,
                   const unsigned int, const unsigned short, unsigned char, const unsigned short, unsigned char, unsigned char)) &
                   chip::Controller::DeviceControllerInteractionModelDelegate::CommandResponseStatus,
               "C++: chip::Controller::DeviceControllerInteractionModelDelegate::CommandResponseStatus(const class "
               "chip::app::CommandSender *, const enum chip::Protocols::SecureChannel::GeneralStatusCode, const unsigned int, "
               "const unsigned short, unsigned char, const unsigned short, unsigned char, unsigned char) --> int",
               pybind11::arg("apCommandSender"), pybind11::arg("aGeneralCode"), pybind11::arg("aProtocolId"),
               pybind11::arg("aProtocolCode"), pybind11::arg("aEndpointId"), pybind11::arg("aClusterId"),
               pybind11::arg("aCommandId"), pybind11::arg("aCommandIndex"));
        cl.def("CommandResponseProtocolError",
               (int (chip::Controller::DeviceControllerInteractionModelDelegate::*)(const class chip::app::CommandSender *,
                                                                                    unsigned char)) &
                   chip::Controller::DeviceControllerInteractionModelDelegate::CommandResponseProtocolError,
               "C++: chip::Controller::DeviceControllerInteractionModelDelegate::CommandResponseProtocolError(const class "
               "chip::app::CommandSender *, unsigned char) --> int",
               pybind11::arg("apCommandSender"), pybind11::arg("aCommandIndex"));
        cl.def("CommandResponseError",
               (int (chip::Controller::DeviceControllerInteractionModelDelegate::*)(const class chip::app::CommandSender *, int)) &
                   chip::Controller::DeviceControllerInteractionModelDelegate::CommandResponseError,
               "C++: chip::Controller::DeviceControllerInteractionModelDelegate::CommandResponseError(const class "
               "chip::app::CommandSender *, int) --> int",
               pybind11::arg("apCommandSender"), pybind11::arg("aError"));
        cl.def("CommandResponseProcessed",
               (int (chip::Controller::DeviceControllerInteractionModelDelegate::*)(const class chip::app::CommandSender *)) &
                   chip::Controller::DeviceControllerInteractionModelDelegate::CommandResponseProcessed,
               "C++: chip::Controller::DeviceControllerInteractionModelDelegate::CommandResponseProcessed(const class "
               "chip::app::CommandSender *) --> int",
               pybind11::arg("apCommandSender"));
        cl.def("assign",
               (class chip::Controller::DeviceControllerInteractionModelDelegate &
                (chip::Controller::DeviceControllerInteractionModelDelegate::*) (const class chip::Controller::
                                                                                     DeviceControllerInteractionModelDelegate &) ) &
                   chip::Controller::DeviceControllerInteractionModelDelegate::operator=,
               "C++: chip::Controller::DeviceControllerInteractionModelDelegate::operator=(const class "
               "chip::Controller::DeviceControllerInteractionModelDelegate &) --> class "
               "chip::Controller::DeviceControllerInteractionModelDelegate &",
               pybind11::return_value_policy::automatic, pybind11::arg(""));
    }
    { // chip::Controller::DeviceController file:controller/CHIPDeviceController.h line:181
        pybind11::class_<chip::Controller::DeviceController, std::shared_ptr<chip::Controller::DeviceController>,
                         PyCallBack_chip_Controller_DeviceController>
            cl(M("chip::Controller"), "DeviceController",
               "Controller applications can use this class to communicate with already paired CHIP devices. The\n   application is "
               "required to provide access to the persistent storage, where the paired device information\n   is stored. This "
               "object of this class can be initialized with the data from the storage (List of devices,\n   and device pairing "
               "information for individual devices). Alternatively, this class can retrieve the\n   relevant information when the "
               "application tries to communicate with the device");
        cl.def(pybind11::init([]() { return new chip::Controller::DeviceController(); },
                              []() { return new PyCallBack_chip_Controller_DeviceController(); }));
        cl.def("Init",
               (int (chip::Controller::DeviceController::*)(unsigned long long, struct chip::Controller::ControllerInitParams)) &
                   chip::Controller::DeviceController::Init,
               "C++: chip::Controller::DeviceController::Init(unsigned long long, struct chip::Controller::ControllerInitParams) "
               "--> int",
               pybind11::arg("localDeviceId"), pybind11::arg("params"));
        cl.def("Shutdown", (int (chip::Controller::DeviceController::*)()) & chip::Controller::DeviceController::Shutdown,
               "Tears down the entirety of the stack, including destructing key objects in the system.\n  This expects to be "
               "called with external thread synchronization, and will not internally\n  grab the CHIP stack lock.\n\n  This will "
               "also not stop the CHIP event queue / thread (if one exists).  Consumers are expected to\n  ensure this happend "
               "before calling this method.\n\nC++: chip::Controller::DeviceController::Shutdown() --> int");
        cl.def("UpdateDevice",
               (int (chip::Controller::DeviceController::*)(class chip::Controller::Device *, unsigned long long)) &
                   chip::Controller::DeviceController::UpdateDevice,
               "This function update the device informations asynchronously using mdns.\n   If new device informations has been "
               "found, it will be persisted.\n\n \n    The input device object to update\n \n\n  The fabricId used for mdns "
               "resolution\n\n \n CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.\n\nC++: "
               "chip::Controller::DeviceController::UpdateDevice(class chip::Controller::Device *, unsigned long long) --> int",
               pybind11::arg("device"), pybind11::arg("fabricId"));
        cl.def("PersistDevice",
               (void (chip::Controller::DeviceController::*)(class chip::Controller::Device *)) &
                   chip::Controller::DeviceController::PersistDevice,
               "C++: chip::Controller::DeviceController::PersistDevice(class chip::Controller::Device *) --> void",
               pybind11::arg("device"));
        cl.def("SetUdpListenPort",
               (int (chip::Controller::DeviceController::*)(unsigned short)) & chip::Controller::DeviceController::SetUdpListenPort,
               "C++: chip::Controller::DeviceController::SetUdpListenPort(unsigned short) --> int", pybind11::arg("listenPort"));
        cl.def("ReleaseDevice",
               (void (chip::Controller::DeviceController::*)(class chip::Controller::Device *)) &
                   chip::Controller::DeviceController::ReleaseDevice,
               "C++: chip::Controller::DeviceController::ReleaseDevice(class chip::Controller::Device *) --> void",
               pybind11::arg("device"));
        cl.def("ServiceEvents", (int (chip::Controller::DeviceController::*)()) & chip::Controller::DeviceController::ServiceEvents,
               "Start the event loop task within the CHIP stack\n \n\n CHIP_ERROR   The return status\n\nC++: "
               "chip::Controller::DeviceController::ServiceEvents() --> int");
        cl.def("ServiceEventSignal",
               (int (chip::Controller::DeviceController::*)()) & chip::Controller::DeviceController::ServiceEventSignal,
               "Allow the CHIP Stack to process any pending events\n   This can be called in an event handler loop to trigger "
               "callbacks within the CHIP stack\n \n\n CHIP_ERROR   The return status\n\nC++: "
               "chip::Controller::DeviceController::ServiceEventSignal() --> int");
        cl.def("GetFabricId",
               (int (chip::Controller::DeviceController::*)(unsigned long long &)) &
                   chip::Controller::DeviceController::GetFabricId,
               "Get the Fabric ID assigned to the device.\n\n \n   Fabric ID of the device.\n\n \n CHIP_ERROR CHIP_NO_ERROR on "
               "success, or corresponding error code.\n\nC++: chip::Controller::DeviceController::GetFabricId(unsigned long long "
               "&) --> int",
               pybind11::arg("fabricId"));
    }
    { // chip::Controller::DeviceCommissionerRendezvousAdvertisementDelegate file:controller/CHIPDeviceController.h line:366
        pybind11::class_<chip::Controller::DeviceCommissionerRendezvousAdvertisementDelegate,
                         std::shared_ptr<chip::Controller::DeviceCommissionerRendezvousAdvertisementDelegate>,
                         PyCallBack_chip_Controller_DeviceCommissionerRendezvousAdvertisementDelegate,
                         chip::RendezvousAdvertisementDelegate>
            cl(M("chip::Controller"), "DeviceCommissionerRendezvousAdvertisementDelegate",
               "The commissioner applications doesn't advertise itself as an available device for rendezvous\n   process. This "
               "delegate class provides no-op functions for the advertisement delegate.");
        cl.def(pybind11::init([]() { return new chip::Controller::DeviceCommissionerRendezvousAdvertisementDelegate(); },
                              []() { return new PyCallBack_chip_Controller_DeviceCommissionerRendezvousAdvertisementDelegate(); }));
        cl.def(pybind11::init([](PyCallBack_chip_Controller_DeviceCommissionerRendezvousAdvertisementDelegate const & o) {
            return new PyCallBack_chip_Controller_DeviceCommissionerRendezvousAdvertisementDelegate(o);
        }));
        cl.def(pybind11::init([](chip::Controller::DeviceCommissionerRendezvousAdvertisementDelegate const & o) {
            return new chip::Controller::DeviceCommissionerRendezvousAdvertisementDelegate(o);
        }));
        cl.def("StartAdvertisement",
               (int (chip::Controller::DeviceCommissionerRendezvousAdvertisementDelegate::*)() const) &
                   chip::Controller::DeviceCommissionerRendezvousAdvertisementDelegate::StartAdvertisement,
               "Starts advertisement of the device for rendezvous availability.\n\nC++: "
               "chip::Controller::DeviceCommissionerRendezvousAdvertisementDelegate::StartAdvertisement() const --> int");
        cl.def("StopAdvertisement",
               (int (chip::Controller::DeviceCommissionerRendezvousAdvertisementDelegate::*)() const) &
                   chip::Controller::DeviceCommissionerRendezvousAdvertisementDelegate::StopAdvertisement,
               "Stops advertisement of the device for rendezvous availability.\n\nC++: "
               "chip::Controller::DeviceCommissionerRendezvousAdvertisementDelegate::StopAdvertisement() const --> int");
        cl.def(
            "assign",
            (class chip::Controller::DeviceCommissionerRendezvousAdvertisementDelegate &
             (chip::Controller::
                  DeviceCommissionerRendezvousAdvertisementDelegate::*) (const class chip::Controller::
                                                                             DeviceCommissionerRendezvousAdvertisementDelegate &) ) &
                chip::Controller::DeviceCommissionerRendezvousAdvertisementDelegate::operator=,
            "C++: chip::Controller::DeviceCommissionerRendezvousAdvertisementDelegate::operator=(const class "
            "chip::Controller::DeviceCommissionerRendezvousAdvertisementDelegate &) --> class "
            "chip::Controller::DeviceCommissionerRendezvousAdvertisementDelegate &",
            pybind11::return_value_policy::automatic, pybind11::arg(""));
    }
    { // chip::Controller::DeviceCommissioner file:controller/CHIPDeviceController.h line:388
        pybind11::class_<chip::Controller::DeviceCommissioner, std::shared_ptr<chip::Controller::DeviceCommissioner>,
                         PyCallBack_chip_Controller_DeviceCommissioner, chip::Controller::DeviceController>
            cl(M("chip::Controller"), "DeviceCommissioner",
               "The commissioner applications can use this class to pair new/unpaired CHIP devices. The application is\n   "
               "required to provide write access to the persistent storage, where the paired device information\n   will be "
               "stored.");
        cl.def(pybind11::init([]() { return new chip::Controller::DeviceCommissioner(); },
                              []() { return new PyCallBack_chip_Controller_DeviceCommissioner(); }));
        cl.def(
            "Init",
            (int (chip::Controller::DeviceCommissioner::*)(unsigned long long, struct chip::Controller::CommissionerInitParams)) &
                chip::Controller::DeviceCommissioner::Init,
            "Commissioner-specific initialization, includes parameters such as the pairing delegate.\n\nC++: "
            "chip::Controller::DeviceCommissioner::Init(unsigned long long, struct chip::Controller::CommissionerInitParams) --> "
            "int",
            pybind11::arg("localDeviceId"), pybind11::arg("params"));
        cl.def("Shutdown", (int (chip::Controller::DeviceCommissioner::*)()) & chip::Controller::DeviceCommissioner::Shutdown,
               "Tears down the entirety of the stack, including destructing key objects in the system.\n  This is not a "
               "thread-safe API, and should be called with external synchronization.\n\n  Please see implementation for more "
               "details.\n\nC++: chip::Controller::DeviceCommissioner::Shutdown() --> int");
        cl.def("PairDevice",
               (int (chip::Controller::DeviceCommissioner::*)(unsigned long long, class chip::RendezvousParameters &)) &
                   chip::Controller::DeviceCommissioner::PairDevice,
               "Pair a CHIP device with the provided Rendezvous connection parameters.\n   Use registered DevicePairingDelegate "
               "object to receive notifications on\n   pairing status updates.\n\n   Note: Pairing process requires that the "
               "caller has registered PersistentStorageDelegate\n         in the Init() call.\n\n \n        The remote device "
               "Id.\n \n\n                The Rendezvous connection parameters\n\nC++: "
               "chip::Controller::DeviceCommissioner::PairDevice(unsigned long long, class chip::RendezvousParameters &) --> int",
               pybind11::arg("remoteDeviceId"), pybind11::arg("params"));
        cl.def("PairTestDeviceWithoutSecurity",
               (int (chip::Controller::DeviceCommissioner::*)(unsigned long long, const class chip::Transport::PeerAddress &,
                                                              struct chip::Controller::SerializedDevice &)) &
                   chip::Controller::DeviceCommissioner::PairTestDeviceWithoutSecurity,
               "C++: chip::Controller::DeviceCommissioner::PairTestDeviceWithoutSecurity(unsigned long long, const class "
               "chip::Transport::PeerAddress &, struct chip::Controller::SerializedDevice &) --> int",
               pybind11::arg("remoteDeviceId"), pybind11::arg("peerAddress"), pybind11::arg("serialized"));
        cl.def("StopPairing",
               (int (chip::Controller::DeviceCommissioner::*)(unsigned long long)) &
                   chip::Controller::DeviceCommissioner::StopPairing,
               "This function stops a pairing process that's in progress. It does not delete the pairing of a previously\n   "
               "paired device.\n\n \n        The remote device Id.\n\n \n CHIP_ERROR               CHIP_NO_ERROR on success, or "
               "corresponding error\n\nC++: chip::Controller::DeviceCommissioner::StopPairing(unsigned long long) --> int",
               pybind11::arg("remoteDeviceId"));
        cl.def("UnpairDevice",
               (int (chip::Controller::DeviceCommissioner::*)(unsigned long long)) &
                   chip::Controller::DeviceCommissioner::UnpairDevice,
               "Remove pairing for a paired device. If the device is currently being paired, it'll stop the pairing process.\n\n "
               "\n        The remote device Id.\n\n \n CHIP_ERROR               CHIP_NO_ERROR on success, or corresponding "
               "error\n\nC++: chip::Controller::DeviceCommissioner::UnpairDevice(unsigned long long) --> int",
               pybind11::arg("remoteDeviceId"));
        cl.def("OnSessionEstablishmentError",
               (void (chip::Controller::DeviceCommissioner::*)(int)) &
                   chip::Controller::DeviceCommissioner::OnSessionEstablishmentError,
               "///////// SessionEstablishmentDelegate Implementation ///////////////\n\nC++: "
               "chip::Controller::DeviceCommissioner::OnSessionEstablishmentError(int) --> void",
               pybind11::arg("error"));
        cl.def("OnSessionEstablished",
               (void (chip::Controller::DeviceCommissioner::*)()) & chip::Controller::DeviceCommissioner::OnSessionEstablished,
               "C++: chip::Controller::DeviceCommissioner::OnSessionEstablished() --> void");
        cl.def("RendezvousCleanup",
               (void (chip::Controller::DeviceCommissioner::*)(int)) & chip::Controller::DeviceCommissioner::RendezvousCleanup,
               "C++: chip::Controller::DeviceCommissioner::RendezvousCleanup(int) --> void", pybind11::arg("status"));
        cl.def("ReleaseDevice",
               (void (chip::Controller::DeviceCommissioner::*)(class chip::Controller::Device *)) &
                   chip::Controller::DeviceCommissioner::ReleaseDevice,
               "C++: chip::Controller::DeviceCommissioner::ReleaseDevice(class chip::Controller::Device *) --> void",
               pybind11::arg("device"));
        cl.def("AdvanceCommissioningStage",
               (void (chip::Controller::DeviceCommissioner::*)(int)) &
                   chip::Controller::DeviceCommissioner::AdvanceCommissioningStage,
               "C++: chip::Controller::DeviceCommissioner::AdvanceCommissioningStage(int) --> void", pybind11::arg("err"));
        cl.def("CloseBleConnection",
               (int (chip::Controller::DeviceCommissioner::*)()) & chip::Controller::DeviceCommissioner::CloseBleConnection,
               "Once we have finished all commissioning work, the Controller should close the BLE\n   connection to the device and "
               "establish CASE session / another PASE session to the device\n   if needed.\n \n\n CHIP_ERROR   The return "
               "status\n\nC++: chip::Controller::DeviceCommissioner::CloseBleConnection() --> int");
        cl.def("RegisterPairingDelegate",
               (void (chip::Controller::DeviceCommissioner::*)(class chip::Controller::DevicePairingDelegate *)) &
                   chip::Controller::DeviceCommissioner::RegisterPairingDelegate,
               "C++: chip::Controller::DeviceCommissioner::RegisterPairingDelegate(class chip::Controller::DevicePairingDelegate "
               "*) --> void",
               pybind11::arg("pairingDelegate"));
    }
}
