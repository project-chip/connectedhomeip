/* See Project CHIP LICENSE file for licensing information. */


/**
 *    @file
 *          Provides an implementation of the PlatformManager object
 *          for Linux platforms.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl_POSIX.cpp>
#include <support/CHIPMem.h>

#include <thread>

#include "MdnsImpl.h"

namespace chip {
namespace DeviceLayer {

PlatformManagerImpl PlatformManagerImpl::sInstance;

#if CHIP_WITH_GIO
static void GDBus_Thread()
{
    GMainLoop * loop = g_main_loop_new(nullptr, false);

    g_main_loop_run(loop);
    g_main_loop_unref(loop);
}
#endif

CHIP_ERROR PlatformManagerImpl::_InitChipStack()
{
    CHIP_ERROR err;

#if CHIP_WITH_GIO
    GError * error = nullptr;

    this->mpGDBusConnection = UniqueGDBusConnection(g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error));

    std::thread gdbusThread(GDBus_Thread);
    gdbusThread.detach();
#endif

    // Initialize the configuration system.
    err = Internal::PosixConfig::Init();
    SuccessOrExit(err);
    // Call _InitChipStack() on the generic implementation base class
    // to finish the initialization process.
    err = Internal::GenericPlatformManagerImpl_POSIX<PlatformManagerImpl>::_InitChipStack();
    SuccessOrExit(err);

exit:
    return err;
}

#if CHIP_WITH_GIO
GDBusConnection * PlatformManagerImpl::GetGDBusConnection()
{
    return this->mpGDBusConnection.get();
}
#endif

} // namespace DeviceLayer
} // namespace chip
