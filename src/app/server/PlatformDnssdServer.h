#pragma once
#include <app/server/Dnssd.h>
#include <lib/dnssd/DnssdServer.h>

namespace chip {
namespace app {
class PlatformDnssdServer final : public chip::Dnssd::DnssdServer
{
    inline CHIP_ERROR AdvertiseOperational() override { return chip::app::DnssdServer::Instance().AdvertiseOperational(); }
    inline void StartServer() override { chip::app::DnssdServer::Instance().StartServer(); }
    inline void StopServer() override { chip::app::DnssdServer::Instance().StopServer(); }
};
} // namespace app
} // namespace chip
