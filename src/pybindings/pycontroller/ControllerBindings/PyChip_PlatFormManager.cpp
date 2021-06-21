#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#include <sstream> // __str__

#include <functional>
#include <pthread.h>
#include <pybind11/pybind11.h>
#include <string>

#ifndef BINDER_PYBIND11_TYPE_CASTER
#define BINDER_PYBIND11_TYPE_CASTER
PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>)
PYBIND11_DECLARE_HOLDER_TYPE(T, T *)
PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void * PlatformMainLoop(void *)
{
    ChipLogProgress(DeviceLayer, "Platform main loop started.");
    chip::DeviceLayer::PlatformMgr().RunEventLoop();
    ChipLogProgress(DeviceLayer, "Platform main loop completed.");
    return nullptr;
}

void * RunMainLoop(void *)
{
    pthread_t sPlatformMainThread;
    int result = pthread_create(&sPlatformMainThread, nullptr, PlatformMainLoop, nullptr);
}

void bind_PyChip_PlatFormManager(std::function<pybind11::module &(std::string const & namespace_)> & M)
{
    { // chip::DeviceLayer::PlatformManager file:platform/PlatformManager.h line:79
        pybind11::class_<chip::DeviceLayer::PlatformManager, chip::DeviceLayer::PlatformManager *> cl(
            M("chip::DeviceLayer"), "PlatformManager",
            "Provides features for initializing and interacting with the chip network\n stack on a chip-enabled device.");
        cl.def("InitChipStack", (int (chip::DeviceLayer::PlatformManager::*)()) & chip::DeviceLayer::PlatformManager::InitChipStack,
               "C++: chip::DeviceLayer::PlatformManager::InitChipStack() --> int");
        cl.def("RunEventLoop", &RunMainLoop, "C++: chip::DeviceLayer::PlatformManager::RunEventLoop() --> void");
        cl.def("StartEventLoopTask",
               (int (chip::DeviceLayer::PlatformManager::*)()) & chip::DeviceLayer::PlatformManager::StartEventLoopTask,
               "C++: chip::DeviceLayer::PlatformManager::StartEventLoopTask() --> int");
        cl.def("StopEventLoopTask",
               (int (chip::DeviceLayer::PlatformManager::*)()) & chip::DeviceLayer::PlatformManager::StopEventLoopTask,
               "C++: chip::DeviceLayer::PlatformManager::StopEventLoopTask() --> int");
        cl.def("LockChipStack",
               (void (chip::DeviceLayer::PlatformManager::*)()) & chip::DeviceLayer::PlatformManager::LockChipStack,
               "C++: chip::DeviceLayer::PlatformManager::LockChipStack() --> void");
        cl.def("TryLockChipStack",
               (bool (chip::DeviceLayer::PlatformManager::*)()) & chip::DeviceLayer::PlatformManager::TryLockChipStack,
               "C++: chip::DeviceLayer::PlatformManager::TryLockChipStack() --> bool");
        cl.def("UnlockChipStack",
               (void (chip::DeviceLayer::PlatformManager::*)()) & chip::DeviceLayer::PlatformManager::UnlockChipStack,
               "C++: chip::DeviceLayer::PlatformManager::UnlockChipStack() --> void");
        cl.def("Shutdown", (int (chip::DeviceLayer::PlatformManager::*)()) & chip::DeviceLayer::PlatformManager::Shutdown,
               "C++: chip::DeviceLayer::PlatformManager::Shutdown() --> int");
    }
    // chip::DeviceLayer::PlatformMgr() file:platform/PlatformManager.h line:165
    M("chip::DeviceLayer")
        .def("PlatformMgr", (class chip::DeviceLayer::PlatformManager & (*) ()) & chip::DeviceLayer::PlatformMgr,
             "Returns the public interface of the PlatformManager singleton object.\n\n chip applications should use this to "
             "access features of the PlatformManager object\n that are common to all platforms.\n\nC++: "
             "chip::DeviceLayer::PlatformMgr() --> class chip::DeviceLayer::PlatformManager &",
             pybind11::return_value_policy::reference);

    // chip::DeviceLayer::PlatformMgrImpl() file:platform/PlatformManager.h line:173
    M("chip::DeviceLayer")
        .def("PlatformMgrImpl", (class chip::DeviceLayer::PlatformManagerImpl & (*) ()) & chip::DeviceLayer::PlatformMgrImpl,
             "Returns the platform-specific implementation of the PlatformManager singleton object.\n\n chip applications can use "
             "this to gain access to features of the PlatformManager\n that are specific to the selected platform.\n\nC++: "
             "chip::DeviceLayer::PlatformMgrImpl() --> class chip::DeviceLayer::PlatformManagerImpl &",
             pybind11::return_value_policy::reference);

    { // chip::DeviceLayer::StackLock file:platform/PlatformManager.h line:180
        pybind11::class_<chip::DeviceLayer::StackLock, std::shared_ptr<chip::DeviceLayer::StackLock>> cl(
            M("chip::DeviceLayer"), "StackLock",
            "RAII locking for PlatformManager to simplify management of\n LockChipStack()/UnlockChipStack calls.");
        cl.def(pybind11::init([]() { return new chip::DeviceLayer::StackLock(); }));
    }
}
