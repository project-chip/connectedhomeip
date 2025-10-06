#pragma once
#include <app/server/Dnssd.h>
#include <lib/dnssd/DnssdServer.h>

namespace chip {
namespace app {
class PlatformDnssdServer : public chip::Dnssd::DnssdServer
{
    CHIP_ERROR AdvertiseOperational() override { return chip::app::DnssdServer::Instance().AdvertiseOperational(); }
    void StartServer() override { chip::app::DnssdServer::Instance().StartServer(); }
    void StopServer() override { chip::app::DnssdServer::Instance().StopServer(); }
};
} // namespace app
} // namespace chip
