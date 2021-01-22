#include <inttypes.h>
#include <string>
#include <unistd.h>

#include "lib/mdns/Scanner.h"
#include "platform/CHIPDeviceLayer.h"
#include "support/CHIPMem.h"

class TestScannerDelegate : public chip::Mdns::ScannerDelegate
{
public:
    void HandleNodeResolved(CHIP_ERROR error, uint64_t nodeId, uint64_t fabricId, const chip::Inet::IPAddress & address) override
    {
        char addrBuf[100];

        address.ToString(addrBuf, sizeof(addrBuf));
        printf("Found nodeId %" PRIX64 " at %s\n", nodeId, addrBuf);
        chip::Mdns::Scanner::Instance().UnsubscribeNode(nodeId, fabricId);
        chip::DeviceLayer::PlatformMgr().Shutdown();
    }
};

int main(int argc, char * argv[])
{
    uint64_t nodeId = 0, fabricId = 0;
    int opt;

    while ((opt = getopt(argc, argv, "n:f:")) != -1)
    {
        switch (opt)
        {
        case 'n':
            nodeId = std::stoull(optarg, 0, 16);
            break;
        case 'f':
            fabricId = std::stoull(optarg, 0, 16);
            break;
        default:
            break;
        }
    }

    TestScannerDelegate delegate;
    chip::Platform::MemoryInit();
    chip::DeviceLayer::PlatformMgr().InitChipStack();
    chip::Mdns::Scanner::Instance().RegisterScannerDelegate(&delegate);
    chip::Mdns::Scanner::Instance().SubscribeNode(nodeId, fabricId);
    chip::DeviceLayer::PlatformMgr().RunEventLoop();
    return 0;
}
