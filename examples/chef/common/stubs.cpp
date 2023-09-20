#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app/data-model/Nullable.h>
#include <app/util/config.h>
#include <lib/core/DataModelTypes.h>

using chip::app::DataModel::Nullable;

using namespace chip;

// Include door lock callbacks only when the server is enabled
#ifdef EMBER_AF_PLUGIN_DOOR_LOCK_SERVER
#include <app/clusters/door-lock-server/door-lock-server.h>

class LockManager
{
public:
    static constexpr uint32_t kNumEndpoints              = 1;
    static constexpr uint32_t kNumUsersPerEndpoint       = 2;
    static constexpr uint32_t kNumCredentialsPerEndpoint = 20;
    static constexpr uint32_t kNumCredentialsPerUser     = 10;
    static constexpr uint32_t kMaxNameLength             = 32;
    static constexpr uint32_t kMaxDataLength             = 16;

    struct Credential
    {
        bool set(DlCredentialStatus status, CredentialTypeEnum type, chip::ByteSpan newData)
        {
            if (newData.size() > kMaxDataLength || type != CredentialTypeEnum::kPin)
                return false;
            memcpy(data, newData.data(), newData.size());
            info = EmberAfPluginDoorLockCredentialInfo{
                status,
                type,
                chip::ByteSpan(data, newData.size()),
            };
            return true;
        }

        EmberAfPluginDoorLockCredentialInfo info = { DlCredentialStatus::kAvailable };
        uint8_t data[kMaxDataLength];
    };

    struct User
    {
        void set(chip::CharSpan newName, uint32_t userId, UserStatusEnum userStatus, UserTypeEnum type,
                 CredentialRuleEnum credentialRule)
        {
            size_t sz = std::min(sizeof(name), newName.size());
            memcpy(name, newName.data(), sz);
            info = EmberAfPluginDoorLockUserInfo{
                chip::CharSpan(name, sz), chip::Span<const CredentialStruct>(), userId, userStatus, type, credentialRule,
            };
        }
        bool addCredential(CredentialTypeEnum type, uint16_t index)
        {
            if (info.credentials.size() == kNumCredentialsPerUser)
                return false;
            auto & cr          = credentialMap[info.credentials.size()];
            cr.credentialType  = type;
            cr.credentialIndex = index;
            info.credentials   = chip::Span<const CredentialStruct>(credentialMap, info.credentials.size() + 1);
            return true;
        }

        EmberAfPluginDoorLockUserInfo info = { .userStatus = UserStatusEnum::kAvailable };
        char name[kMaxNameLength];
        CredentialStruct credentialMap[kNumCredentialsPerUser];
    };

    struct Endpoint
    {
        chip::EndpointId id;
        User users[kNumUsersPerEndpoint];
        Credential credentials[kNumCredentialsPerEndpoint];
    };

    static LockManager & Instance()
    {
        static LockManager instance;
        return instance;
    }

    LockManager() { defaultInitialize(); }

    bool getUser(chip::EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user)
    {
        auto ep = findEndpoint(endpointId);
        if (!ep)
            return false;
        if (userIndex >= kNumUsersPerEndpoint)
            return false;
        user = ep->users[userIndex].info;
        return true;
    }

    bool setUser(chip::EndpointId endpointId, uint16_t userIndex, chip::FabricIndex creator, chip::FabricIndex modifier,
                 const chip::CharSpan & userName, uint32_t uniqueId, UserStatusEnum userStatus, UserTypeEnum usertype,
                 CredentialRuleEnum credentialRule, const CredentialStruct * credentials, size_t totalCredentials)
    {
        auto ep = findEndpoint(endpointId);
        if (!ep)
            return false;
        if (userIndex >= kNumUsersPerEndpoint || totalCredentials > kNumCredentialsPerUser)
            return false;
        ep->users[userIndex].set(userName, uniqueId, userStatus, usertype, credentialRule);
        ep->users[userIndex].info.creationSource     = DlAssetSource::kMatterIM;
        ep->users[userIndex].info.createdBy          = creator;
        ep->users[userIndex].info.modificationSource = DlAssetSource::kMatterIM;
        ep->users[userIndex].info.lastModifiedBy     = modifier;
        for (size_t i = 0; i < totalCredentials; i++)
            ep->users[userIndex].addCredential(credentials[i].credentialType, credentials[i].credentialIndex);
        return true;
    }

    bool getCredential(chip::EndpointId endpointId, uint16_t credentialIndex, CredentialTypeEnum credentialType,
                       EmberAfPluginDoorLockCredentialInfo & credential)
    {
        auto ep = findEndpoint(endpointId);
        if (!ep)
            return false;
        if (credentialIndex >= kNumCredentialsPerEndpoint)
            return false;
        if (credentialType != CredentialTypeEnum::kPin)
            return false;
        credential = ep->credentials[credentialIndex].info;
        return true;
    }

    bool setCredential(chip::EndpointId endpointId, uint16_t credentialIndex, chip::FabricIndex creator, chip::FabricIndex modifier,
                       DlCredentialStatus credentialStatus, CredentialTypeEnum credentialType,
                       const chip::ByteSpan & credentialData)
    {
        auto ep = findEndpoint(endpointId);
        if (!ep)
            return false;
        if (credentialIndex >= kNumCredentialsPerEndpoint)
            return false;
        if (credentialType != CredentialTypeEnum::kPin)
            return false;
        auto & credential = ep->credentials[credentialIndex];
        if (!credential.set(credentialStatus, credentialType, credentialData))
            return false;
        credential.info.creationSource     = DlAssetSource::kMatterIM;
        credential.info.createdBy          = creator;
        credential.info.modificationSource = DlAssetSource::kMatterIM;
        credential.info.lastModifiedBy     = modifier;
        return true;
    }

    bool checkPin(chip::EndpointId endpointId, const chip::Optional<chip::ByteSpan> & pinCode,
                  chip::app::Clusters::DoorLock::OperationErrorEnum & err)
    {
        if (!pinCode.HasValue())
        {
            err = OperationErrorEnum::kInvalidCredential;
            return false;
        }
        auto ep = findEndpoint(endpointId);
        if (!ep)
            return false;
        for (auto & pin : ep->credentials)
        {
            if (pin.info.status == DlCredentialStatus::kOccupied && pin.info.credentialData.data_equal(pinCode.Value()))
            {
                return true;
            }
        }
        err = OperationErrorEnum::kInvalidCredential;
        return false;
    }

private:
    Endpoint * findEndpoint(chip::EndpointId endpointId)
    {
        for (auto & e : endpoints)
        {
            if (e.id == endpointId)
                return &e;
        }
        return nullptr;
    }

    void defaultInitialize()
    {
        endpoints[0].id = 1;
        uint8_t pin[6]  = { 0x31, 0x32, 0x33, 0x34, 0x35, 0x36 };
        endpoints[0].credentials[0].set(DlCredentialStatus::kOccupied, CredentialTypeEnum::kPin, chip::ByteSpan(pin));
        endpoints[0].users[0].set(chip::CharSpan("default"), 1, UserStatusEnum::kOccupiedEnabled, UserTypeEnum::kUnrestrictedUser,
                                  CredentialRuleEnum::kSingle);
        endpoints[0].users[0].addCredential(CredentialTypeEnum::kPin, 1);
    }

    Endpoint endpoints[kNumEndpoints];
};

bool emberAfPluginDoorLockOnDoorLockCommand(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                                            const Nullable<chip::NodeId> & nodeId, const chip::Optional<chip::ByteSpan> & pinCode,
                                            chip::app::Clusters::DoorLock::OperationErrorEnum & err)
{
    err = OperationErrorEnum::kUnspecified;
    return DoorLockServer::Instance().SetLockState(endpointId, DlLockState::kLocked);
}

bool emberAfPluginDoorLockOnDoorUnlockCommand(chip::EndpointId endpointId, const Nullable<chip::FabricIndex> & fabricIdx,
                                              const Nullable<chip::NodeId> & nodeId, const chip::Optional<chip::ByteSpan> & pinCode,
                                              chip::app::Clusters::DoorLock::OperationErrorEnum & err)
{
    err = OperationErrorEnum::kUnspecified;
    return DoorLockServer::Instance().SetLockState(endpointId, DlLockState::kUnlocked);
}

bool emberAfPluginDoorLockGetUser(chip::EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user)
{
    return LockManager::Instance().getUser(endpointId, userIndex - 1, user);
}

bool emberAfPluginDoorLockSetUser(chip::EndpointId endpointId, uint16_t userIndex, chip::FabricIndex creator,
                                  chip::FabricIndex modifier, const chip::CharSpan & userName, uint32_t uniqueId,
                                  UserStatusEnum userStatus, UserTypeEnum usertype, CredentialRuleEnum credentialRule,
                                  const CredentialStruct * credentials, size_t totalCredentials)
{
    return LockManager::Instance().setUser(endpointId, userIndex - 1, creator, modifier, userName, uniqueId, userStatus, usertype,
                                           credentialRule, credentials, totalCredentials);
}

bool emberAfPluginDoorLockGetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, CredentialTypeEnum credentialType,
                                        EmberAfPluginDoorLockCredentialInfo & credential)
{
    return LockManager::Instance().getCredential(endpointId, credentialIndex - 1, credentialType, credential);
}

bool emberAfPluginDoorLockSetCredential(chip::EndpointId endpointId, uint16_t credentialIndex, chip::FabricIndex creator,
                                        chip::FabricIndex modifier, DlCredentialStatus credentialStatus,
                                        CredentialTypeEnum credentialType, const chip::ByteSpan & credentialData)
{
    return LockManager::Instance().setCredential(endpointId, credentialIndex - 1, creator, modifier, credentialStatus,
                                                 credentialType, credentialData);
}

#endif /* EMBER_AF_PLUGIN_DOOR_LOCK_SERVER */

#ifdef EMBER_AF_PLUGIN_CHANNEL_SERVER
#include <chef-channel-manager.h>

void emberAfChannelClusterInitCallback(EndpointId endpoint)
{
    app::Clusters::Channel::SetDefaultDelegate(endpoint,
                                               static_cast<app::Clusters::Channel::Delegate *>(&(ChefChannelManager::Instance())));
}
#endif // EMBER_AF_PLUGIN_CHANNEL_SERVER

void emberAfPluginSmokeCoAlarmSelfTestRequestCommand(EndpointId endpointId) {}
