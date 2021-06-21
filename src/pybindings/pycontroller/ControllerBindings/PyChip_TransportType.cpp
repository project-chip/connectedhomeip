#include <core/Optional.h>                   // chip::Optional
#include <crypto/CHIPCryptoPAL.h>            // chip::Crypto::P256Keypair
#include <crypto/CHIPCryptoPAL.h>            // chip::Crypto::P256PublicKey
#include <inet/IPAddress.h>                  // chip::Inet::(anonymous)
#include <inet/IPAddress.h>                  // chip::Inet::IPAddress
#include <lib/support/BitFlags.h>            // chip::BitFlags
#include <netinet/in.h>                      // in_addr
#include <sstream>                           // __str__
#include <support/Span.h>                    // chip::FixedSpan
#include <support/Span.h>                    // chip::Span
#include <sys/socket.h>                      // sockaddr
#include <system/SystemPacketBuffer.h>       // chip::System::PacketBufferHandle
#include <transport/MessageCounter.h>        //
#include <transport/MessageCounter.h>        // chip::MessageCounter
#include <transport/PeerConnectionState.h>   // chip::Transport::PeerConnectionState
#include <transport/PeerMessageCounter.h>    // chip::Transport::PeerMessageCounter
#include <transport/SecureSession.h>         //
#include <transport/SecureSession.h>         // chip::SecureSession
#include <transport/SessionMessageCounter.h> // chip::Transport::SessionMessageCounter
#include <transport/raw/Base.h>              // chip::Transport::Base
#include <transport/raw/Base.h>              // chip::Transport::RawTransportDelegate
#include <transport/raw/MessageHeader.h>     // chip::Header::EncryptionType
#include <transport/raw/MessageHeader.h>     // chip::Header::FlagValues
#include <transport/raw/MessageHeader.h>     // chip::MessageAuthenticationCode
#include <transport/raw/MessageHeader.h>     // chip::PacketHeader
#include <transport/raw/PeerAddress.h>       // chip::Transport::PeerAddress
#include <transport/raw/PeerAddress.h>       // chip::Transport::Type

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

void bind_PyChip_TransportType(std::function<pybind11::module &(std::string const & namespace_)> & M)
{
    {
        // chip::Transport::Type file:transport/raw/PeerAddress.h line:50
        pybind11::enum_<chip::Transport::Type>(M("chip::Transport"), "Type",
                                               "Here we specified Type to be uint8_t, so the PeerAddress can be serialized easily.")
            .value("kUndefined", chip::Transport::Type::kUndefined)
            .value("kUdp", chip::Transport::Type::kUdp)
            .value("kBle", chip::Transport::Type::kBle)
            .value("kTcp", chip::Transport::Type::kTcp);
    }

    { // chip::Transport::PeerAddress file:transport/raw/PeerAddress.h line:61
        pybind11::class_<chip::Transport::PeerAddress, std::shared_ptr<chip::Transport::PeerAddress>> cl(
            M("chip::Transport"), "PeerAddress", "Describes how a peer on a CHIP network can be addressed.");
        cl.def(pybind11::init([]() { return new chip::Transport::PeerAddress(); }));
        cl.def(pybind11::init<enum chip::Transport::Type>(), pybind11::arg("type"));

        cl.def(pybind11::init([](chip::Transport::PeerAddress const & o) { return new chip::Transport::PeerAddress(o); }));
        cl.def(
            "assign",
            (class chip::Transport::PeerAddress & (chip::Transport::PeerAddress::*) (const class chip::Transport::PeerAddress &) ) &
                chip::Transport::PeerAddress::operator=,
            "C++: chip::Transport::PeerAddress::operator=(const class chip::Transport::PeerAddress &) --> class "
            "chip::Transport::PeerAddress &",
            pybind11::return_value_policy::automatic, pybind11::arg(""));
        cl.def("GetTransportType",
               (enum chip::Transport::Type(chip::Transport::PeerAddress::*)() const) &
                   chip::Transport::PeerAddress::GetTransportType,
               "C++: chip::Transport::PeerAddress::GetTransportType() const --> enum chip::Transport::Type");
        cl.def("SetTransportType",
               (class chip::Transport::PeerAddress & (chip::Transport::PeerAddress::*) (enum chip::Transport::Type)) &
                   chip::Transport::PeerAddress::SetTransportType,
               "C++: chip::Transport::PeerAddress::SetTransportType(enum chip::Transport::Type) --> class "
               "chip::Transport::PeerAddress &",
               pybind11::return_value_policy::automatic, pybind11::arg("type"));
        cl.def("GetPort", (unsigned short (chip::Transport::PeerAddress::*)() const) & chip::Transport::PeerAddress::GetPort,
               "C++: chip::Transport::PeerAddress::GetPort() const --> unsigned short");
        cl.def("SetPort",
               (class chip::Transport::PeerAddress & (chip::Transport::PeerAddress::*) (unsigned short) ) &
                   chip::Transport::PeerAddress::SetPort,
               "C++: chip::Transport::PeerAddress::SetPort(unsigned short) --> class chip::Transport::PeerAddress &",
               pybind11::return_value_policy::automatic, pybind11::arg("port"));
        cl.def("GetInterface",
               (unsigned int (chip::Transport::PeerAddress::*)() const) & chip::Transport::PeerAddress::GetInterface,
               "C++: chip::Transport::PeerAddress::GetInterface() const --> unsigned int");
        cl.def("SetInterface",
               (class chip::Transport::PeerAddress & (chip::Transport::PeerAddress::*) (unsigned int) ) &
                   chip::Transport::PeerAddress::SetInterface,
               "C++: chip::Transport::PeerAddress::SetInterface(unsigned int) --> class chip::Transport::PeerAddress &",
               pybind11::return_value_policy::automatic, pybind11::arg("interface"));
        cl.def("IsInitialized", (bool (chip::Transport::PeerAddress::*)() const) & chip::Transport::PeerAddress::IsInitialized,
               "C++: chip::Transport::PeerAddress::IsInitialized() const --> bool");
        cl.def("__eq__",
               (bool (chip::Transport::PeerAddress::*)(const class chip::Transport::PeerAddress &) const) &
                   chip::Transport::PeerAddress::operator==,
               "C++: chip::Transport::PeerAddress::operator==(const class chip::Transport::PeerAddress &) const --> bool",
               pybind11::arg("other"));
        cl.def("__ne__",
               (bool (chip::Transport::PeerAddress::*)(const class chip::Transport::PeerAddress &) const) &
                   chip::Transport::PeerAddress::operator!=,
               "C++: chip::Transport::PeerAddress::operator!=(const class chip::Transport::PeerAddress &) const --> bool",
               pybind11::arg("other"));
        cl.def("ToString",
               (void (chip::Transport::PeerAddress::*)(char *, unsigned long) const) & chip::Transport::PeerAddress::ToString,
               "C++: chip::Transport::PeerAddress::ToString(char *, unsigned long) const --> void", pybind11::arg("buf"),
               pybind11::arg("bufSize"));
        cl.def_static("Uninitialized", (class chip::Transport::PeerAddress(*)()) & chip::Transport::PeerAddress::Uninitialized,
                      "**** Factory methods for convenience *****\n\nC++: chip::Transport::PeerAddress::Uninitialized() --> class "
                      "chip::Transport::PeerAddress");
        cl.def_static("BLE", (class chip::Transport::PeerAddress(*)()) & chip::Transport::PeerAddress::BLE,
                      "C++: chip::Transport::PeerAddress::BLE() --> class chip::Transport::PeerAddress");
    }
}
