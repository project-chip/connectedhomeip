/*
 *
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

/**
 * @brief Manages Content Apps
 */

#pragma once

#include <app-common/zap-generated/af-structs.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/enums.h>
#include <app/clusters/application-launcher-server/application-launcher-server.h>
#include <app/clusters/content-launch-server/content-launch-server.h>
#include <app/clusters/target-navigator-server/target-navigator-server.h>
#include <app/util/attribute-storage.h>
#include <functional>
#include <list>
#include <stdbool.h>
#include <stdint.h>
#include <string>

namespace chip {
namespace AppPlatform {

class DLL_EXPORT ContentAppCluster
{
public:
    virtual ~ContentAppCluster() = default;

    virtual EmberAfStatus HandleReadAttribute(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength) = 0;
    virtual EmberAfStatus HandleWriteAttribute(chip::AttributeId attributeId, uint8_t * buffer)                        = 0;
};

class DLL_EXPORT ApplicationBasic : public ContentAppCluster
{
public:
    virtual ~ApplicationBasic() = default;

    virtual const char * GetVendorName()                                                                        = 0;
    virtual uint16_t GetVendorId()                                                                              = 0;
    virtual const char * GetApplicationName()                                                                   = 0;
    virtual uint16_t GetProductId()                                                                             = 0;
    virtual app::Clusters::ApplicationBasic::ApplicationStatusEnum GetApplicationStatus()                       = 0;
    virtual const char * GetApplicationVersion()                                                                = 0;
    virtual void SetApplicationStatus(app::Clusters::ApplicationBasic::ApplicationStatusEnum applicationStatus) = 0;

    EmberAfStatus HandleReadAttribute(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength) override;
    EmberAfStatus HandleWriteAttribute(chip::AttributeId attributeId, uint8_t * buffer) override;
};

class DLL_EXPORT AccountLogin : public ContentAppCluster
{
public:
    virtual ~AccountLogin() = default;

    virtual void SetSetupPIN(uint32_t setupPIN)                       = 0;
    virtual uint32_t GetSetupPIN(const char * tempAccountId)          = 0;
    virtual bool Login(const char * tempAccountId, uint32_t setupPin) = 0;

    EmberAfStatus HandleReadAttribute(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength) override;
    EmberAfStatus HandleWriteAttribute(chip::AttributeId attributeId, uint8_t * buffer) override;
};

class DLL_EXPORT KeypadInput : public ContentAppCluster
{
public:
    virtual ~KeypadInput() = default;

    EmberAfStatus HandleReadAttribute(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength) override;
    EmberAfStatus HandleWriteAttribute(chip::AttributeId attributeId, uint8_t * buffer) override;
};

class DLL_EXPORT ApplicationLauncher : public ContentAppCluster
{
public:
    virtual ~ApplicationLauncher() = default;

    virtual ApplicationLauncherResponse LaunchApp(Application application, std::string data) = 0;

    EmberAfStatus HandleReadAttribute(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength) override;
    EmberAfStatus HandleWriteAttribute(chip::AttributeId attributeId, uint8_t * buffer) override;
};

class DLL_EXPORT ContentLauncher : public ContentAppCluster
{
public:
    virtual ~ContentLauncher() = default;

    virtual LaunchResponse LaunchContent(std::list<Parameter> parameterList, bool autoplay, std::string data) = 0;

    EmberAfStatus HandleReadAttribute(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength) override;
    EmberAfStatus HandleWriteAttribute(chip::AttributeId attributeId, uint8_t * buffer) override;
};

class DLL_EXPORT MediaPlayback : public ContentAppCluster
{
public:
    virtual ~MediaPlayback() = default;

    EmberAfStatus HandleReadAttribute(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength) override;
    EmberAfStatus HandleWriteAttribute(chip::AttributeId attributeId, uint8_t * buffer) override;
};

class DLL_EXPORT TargetNavigator : public ContentAppCluster
{
public:
    TargetNavigator(std::list<std::string> targets, uint8_t currentTarget);
    virtual ~TargetNavigator() = default;

    TargetNavigatorResponse NavigateTarget(uint8_t target, std::string data);
    CHIP_ERROR GetTargetInfoList(chip::app::AttributeValueEncoder & aEncoder);

    EmberAfStatus HandleReadAttribute(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength) override;
    EmberAfStatus HandleWriteAttribute(chip::AttributeId attributeId, uint8_t * buffer) override;

protected:
    std::list<std::string> mTargets;
    uint8_t mCurrentTarget;
};

class DLL_EXPORT Channel : public ContentAppCluster
{
public:
    virtual ~Channel() = default;

    EmberAfStatus HandleReadAttribute(chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength) override;
    EmberAfStatus HandleWriteAttribute(chip::AttributeId attributeId, uint8_t * buffer) override;
};

class DLL_EXPORT ContentApp
{
public:
    virtual ~ContentApp() = default;

    inline void SetEndpointId(chip::EndpointId id) { mEndpointId = id; };
    inline chip::EndpointId GetEndpointId() { return mEndpointId; };

    virtual ApplicationBasic * GetApplicationBasic()       = 0;
    virtual AccountLogin * GetAccountLogin()               = 0;
    virtual KeypadInput * GetKeypadInput()                 = 0;
    virtual ApplicationLauncher * GetApplicationLauncher() = 0;
    virtual ContentLauncher * GetContentLauncher()         = 0;
    virtual MediaPlayback * GetMediaPlayback()             = 0;
    virtual TargetNavigator * GetTargetNavigator()         = 0;
    virtual Channel * GetChannel()                         = 0;

    EmberAfStatus HandleReadAttribute(ClusterId clusterId, chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength);
    EmberAfStatus HandleWriteAttribute(ClusterId clusterId, chip::AttributeId attributeId, uint8_t * buffer);

protected:
    chip::EndpointId mEndpointId = 0;
};

} // namespace AppPlatform
} // namespace chip
