/* See Project CHIP LICENSE file for licensing information. */


#include <transport/TransportMgrBase.h>

#include <support/CodeUtils.h>
#include <transport/TransportMgr.h>
#include <transport/raw/Base.h>

namespace chip {

CHIP_ERROR TransportMgrBase::SendMessage(const PacketHeader & header, const Transport::PeerAddress & address,
                                         System::PacketBufferHandle && msgBuf)
{
    return mTransport->SendMessage(header, address, std::move(msgBuf));
}

void TransportMgrBase::Disconnect(const Transport::PeerAddress & address)
{
    mTransport->Disconnect(address);
}

CHIP_ERROR TransportMgrBase::Init(Transport::Base * transport)
{
    if (mTransport != nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    mTransport = transport;
    mTransport->SetDelegate(this);
    ChipLogDetail(Inet, "TransportMgr initialized");
    return CHIP_NO_ERROR;
}

void TransportMgrBase::HandleMessageReceived(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                             System::PacketBufferHandle msg)
{
    TransportMgrDelegate * handler = packetHeader.GetFlags().Has(Header::FlagValues::kSecure) ? mSecureSessionMgr : mRendezvous;
    if (handler != nullptr)
    {
        handler->OnMessageReceived(packetHeader, peerAddress, std::move(msg));
    }
    else
    {
        char addrBuffer[Transport::PeerAddress::kMaxToStringSize];
        peerAddress.ToString(addrBuffer, sizeof(addrBuffer));
        ChipLogError(Inet, "%s message from %s is dropped since no corresponding handler is set in TransportMgr.",
                     packetHeader.GetFlags().Has(Header::FlagValues::kSecure) ? "Encrypted" : "Unencrypted", addrBuffer);
    }
}

} // namespace chip
