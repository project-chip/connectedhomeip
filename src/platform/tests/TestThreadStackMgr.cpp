#include "platform/internal/CHIPDeviceLayerInternal.h"

#include "platform/ThreadStackManager.h"
#include "platform/PlatformManager.h"

int TestThreadStackManager(void)
{
    chip::DeviceLayer::ThreadStackManagerImpl impl;
    chip::DeviceLayer::Internal::DeviceNetworkInfo info;
    uint16_t masterKey[16] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

    strncpy(info.ThreadNetworkName, "CHIP-TEST", sizeof(info.ThreadNetworkName));
    info.ThreadChannel                    = UINT8_MAX;
    info.ThreadPANId                      = 0x3455;
    info.FieldPresent.ThreadExtendedPANId = false;
    info.FieldPresent.ThreadMeshPrefix    = false;
    info.FieldPresent.ThreadPSKc          = false;
    memcpy(&info.ThreadNetworkKey, &masterKey, sizeof(masterKey));

    chip::DeviceLayer::PlatformMgrImpl().InitChipStack();

    impl.InitThreadStack();
    impl.StartThreadTask();
    impl._SetThreadProvision(info);
    impl._SetThreadEnabled(true);

    printf("Start Thread task done\n");

    //chip::DeviceLayer::PlatformMgrImpl().RunEventLoop();

    return 0;
}
