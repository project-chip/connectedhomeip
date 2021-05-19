/*
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <stack/Stack.h>
#include <stack/Traits.h>

#include <messaging/ExchangeMgr.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <transport/SecureSessionMgr.h>
#include <transport/TransportMgr.h>
#include <transport/raw/UDP.h>

#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#endif

namespace chip {

/* ========================== SystemLayer Configuration ========================== */
struct SystemLayerConfiguration
{
};
template <typename T>
struct IsSystemLayerConfiguration : std::is_base_of<SystemLayerConfiguration, T>
{
};

#if CONFIG_DEVICE_LAYER
class DefaultSystemLayer : SystemLayerConfiguration
{
public:
    CHIP_ERROR Init() { return CHIP_NO_ERROR; }
    CHIP_ERROR Shutdown() { return CHIP_NO_ERROR; }

    System::Layer & Get() { return DeviceLayer::SystemLayer; }
};
#else
class DefaultSystemLayer : SystemLayerConfiguration
{
public:
    CHIP_ERROR Init() { return mSystemLayer.Init(nullptr); }

    CHIP_ERROR Shutdown() { return mSystemLayer.Shutdown(); }

    System::Layer & Get() { return mSystemLayer; }

private:
    System::Layer mSystemLayer;
};
#endif // CONFIG_DEVICE_LAYER

/* ========================== InetLayer Configuration ========================== */
struct InetLayerConfiguration
{
};
template <typename T>
struct IsInetLayerConfiguration : std::is_base_of<InetLayerConfiguration, T>
{
};

#if CONFIG_DEVICE_LAYER
class DefaultInetLayer : InetLayerConfiguration
{
public:
    CHIP_ERROR Init(System::Layer & aSystemLayer) { return CHIP_NO_ERROR; }
    CHIP_ERROR Shutdown() { return CHIP_NO_ERROR; }

    Inet::InetLayer & Get() { return DeviceLayer::InetLayer; }
};
#else
class DefaultInetLayer : InetLayerConfiguration
{
public:
    CHIP_ERROR Init(System::Layer & aSystemLayer) { return mInetLayer.Init(aSystemLayer, nullptr); }

    CHIP_ERROR Shutdown() { return mInetLayer.Shutdown(); }

    Inet::InetLayer & Get() { return mInetLayer; }

private:
    Inet::InetLayer mInetLayer;
};
#endif // CONFIG_DEVICE_LAYER

/* ========================== Storage Configuration ========================== */
struct StorageConfiguration
{
};
template <typename T>
struct IsStorageConfiguration : std::is_base_of<StorageConfiguration, T>
{
};

class DefaultStorage : StorageConfiguration
{
public:
    CHIP_ERROR Init() { return CHIP_NO_ERROR; }
    CHIP_ERROR Shutdown() { return CHIP_NO_ERROR; }

    PersistentStorageDelegate * Get() { return nullptr; }
};

/* ========================== BleLayer Configuration ========================== */
struct BleLayerConfiguration
{
};
template <typename T>
struct IsBleLayerConfiguration : std::is_base_of<BleLayerConfiguration, T>
{
};

class DefaultBleLayer : BleLayerConfiguration
{
public:
    template <typename T>
    CHIP_ERROR Init(T * stack)
    {
        return CHIP_NO_ERROR;
    }

    Ble::BleLayer * Get() { return nullptr; }
};

/* ========================== Transport Configuration ========================== */
struct TransportConfiguration
{
};
template <typename T>
struct IsTransportConfiguration : std::is_base_of<TransportConfiguration, T>
{
};

class DefaultTransport : TransportConfiguration
{
public:
    using transport = TransportMgr<
#if INET_CONFIG_ENABLE_IPV4
        Transport::UDP, /* IPv4 */
#endif
        Transport::UDP>;

    CHIP_ERROR Init(Inet::InetLayer & inetLayer, Ble::BleLayer * bleLayer)
    {
        return mTransportManager.Init(
#if INET_CONFIG_ENABLE_IPV4
            Transport::UdpListenParameters(&inetLayer).SetAddressType(Inet::kIPAddressType_IPv4).SetListenPort(mPort),
#endif
            Transport::UdpListenParameters(&inetLayer).SetAddressType(Inet::kIPAddressType_IPv6).SetListenPort(mPort));
    }

    TransportMgrBase & Get() { return mTransportManager; }
    void SetListenPort(uint16_t port) { mPort = port; }

private:
    transport mTransportManager;
    uint16_t mPort = CHIP_PORT;
};

/* ========================== Stack ========================== */
template <typename... Configurations>
class Stack
{
private:
    using SystemLayerConfig =
        typename first_if_any_or_default<IsSystemLayerConfiguration, DefaultSystemLayer, Configurations...>::type;
    using InetLayerConfig = typename first_if_any_or_default<IsInetLayerConfiguration, DefaultInetLayer, Configurations...>::type;
    using StorageConfig   = typename first_if_any_or_default<IsStorageConfiguration, DefaultStorage, Configurations...>::type;
    using BleLayerConfig  = typename first_if_any_or_default<IsBleLayerConfiguration, DefaultBleLayer, Configurations...>::type;
    using TransportConfig = typename first_if_any_or_default<IsTransportConfiguration, DefaultTransport, Configurations...>::type;

public:
    Stack(NodeId localDeviceId) : mLocalDeviceId(localDeviceId) {}
    virtual ~Stack() {}

    CHIP_ERROR Init()
    {
        ReturnErrorOnFailure(chip::Platform::MemoryInit());
#if CONFIG_DEVICE_LAYER
        // Initialize the CHIP stack.
        ReturnErrorOnFailure(chip::DeviceLayer::PlatformMgr().InitChipStack());
#endif
        ReturnErrorOnFailure(mSystemLayer.Init());
        ReturnErrorOnFailure(mInetLayer.Init(GetSystemLayer()));
        ReturnErrorOnFailure(mStorage.Init());
        ReturnErrorOnFailure(mBleLayer.Init(this));
        ReturnErrorOnFailure(mTransport.Init(GetInetLayer(), GetBleLayer()));
        auto * storage = mStorage.Get();
        if (storage != nullptr)
        {
            ReturnErrorOnFailure(mAdmins.Init(storage));
        }
        ReturnErrorOnFailure(
            mSessionManager.Init(mLocalDeviceId, &GetSystemLayer(), &GetTransportManager(), &mAdmins, &mMessageCounterManager));
        ReturnErrorOnFailure(mExchangeManager.Init(&mSessionManager));
        ReturnErrorOnFailure(mMessageCounterManager.Init(&mExchangeManager));
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Shutdown()
    {
        mMessageCounterManager.Shutdown();
        mExchangeManager.Shutdown();
        mSessionManager.Shutdown();

        Ble::BleLayer * ble = GetBleLayer();
        if (ble != nullptr)
            ble->Shutdown();
        mInetLayer.Shutdown();
        mSystemLayer.Shutdown();
#if CONFIG_DEVICE_LAYER
        ReturnErrorOnFailure(DeviceLayer::PlatformMgr().Shutdown());
#endif // CONFIG_DEVICE_LAYER
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ResetTransport() { return mTransport.Init(mInetLayer.Get()); }

    TransportConfig & GetTransportConfig() { return mTransport; }

    NodeId GetLocalNodeId() { return mLocalDeviceId; }
    System::Layer & GetSystemLayer() { return mSystemLayer.Get(); }
    Inet::InetLayer & GetInetLayer() { return mInetLayer.Get(); }
    BleLayerConfig & GetBleLayerConfig() { return mBleLayer; }
    Ble::BleLayer * GetBleLayer() { return mBleLayer.Get(); }
    PersistentStorageDelegate * GetStorage() { return mStorage.Get(); }
    TransportMgrBase & GetTransportManager() { return mTransport.Get(); }
    SecureSessionMgr & GetSecureSessionManager() { return mSessionManager; }
    Transport::AdminPairingTable & GetAdmins() { return mAdmins; }
    Messaging::ExchangeManager & GetExchangeManager() { return mExchangeManager; }

private:
    NodeId mLocalDeviceId;

    SystemLayerConfig mSystemLayer;
    InetLayerConfig mInetLayer;
    StorageConfig mStorage;
    BleLayerConfig mBleLayer;

    Transport::AdminPairingTable mAdmins;

    TransportConfig mTransport;
    SecureSessionMgr mSessionManager;
    Messaging::ExchangeManager mExchangeManager;
    secure_channel::MessageCounterManager mMessageCounterManager;
};

} // namespace chip
