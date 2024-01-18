/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "MediaReadBase.h"
#include "MediaSubscriptionBase.h"

#include <controller/CHIPCluster.h>

#include <app-common/zap-generated/cluster-objects.h>

// SUBSCRIBER CLASSES
class VendorNameSubscriber : public MediaSubscriptionBase<chip::app::Clusters::ApplicationBasic::Attributes::VendorName::TypeInfo>
{
public:
    VendorNameSubscriber() : MediaSubscriptionBase(chip::app::Clusters::ApplicationBasic::Id) {}
};

class VendorIDSubscriber : public MediaSubscriptionBase<chip::app::Clusters::ApplicationBasic::Attributes::VendorID::TypeInfo>
{
public:
    VendorIDSubscriber() : MediaSubscriptionBase(chip::app::Clusters::ApplicationBasic::Id) {}
};

class ApplicationNameSubscriber
    : public MediaSubscriptionBase<chip::app::Clusters::ApplicationBasic::Attributes::ApplicationName::TypeInfo>
{
public:
    ApplicationNameSubscriber() : MediaSubscriptionBase(chip::app::Clusters::ApplicationBasic::Id) {}
};

class ProductIDSubscriber : public MediaSubscriptionBase<chip::app::Clusters::ApplicationBasic::Attributes::ProductID::TypeInfo>
{
public:
    ProductIDSubscriber() : MediaSubscriptionBase(chip::app::Clusters::ApplicationBasic::Id) {}
};

class ApplicationSubscriber : public MediaSubscriptionBase<chip::app::Clusters::ApplicationBasic::Attributes::Application::TypeInfo>
{
public:
    ApplicationSubscriber() : MediaSubscriptionBase(chip::app::Clusters::ApplicationBasic::Id) {}
};

class StatusSubscriber : public MediaSubscriptionBase<chip::app::Clusters::ApplicationBasic::Attributes::Status::TypeInfo>
{
public:
    StatusSubscriber() : MediaSubscriptionBase(chip::app::Clusters::ApplicationBasic::Id) {}
};

class ApplicationVersionSubscriber
    : public MediaSubscriptionBase<chip::app::Clusters::ApplicationBasic::Attributes::ApplicationVersion::TypeInfo>
{
public:
    ApplicationVersionSubscriber() : MediaSubscriptionBase(chip::app::Clusters::ApplicationBasic::Id) {}
};

class AllowedVendorListSubscriber
    : public MediaSubscriptionBase<chip::app::Clusters::ApplicationBasic::Attributes::AllowedVendorList::TypeInfo>
{
public:
    AllowedVendorListSubscriber() : MediaSubscriptionBase(chip::app::Clusters::ApplicationBasic::Id) {}
};

// READER CLASSES

class VendorNameReader : public MediaReadBase<chip::app::Clusters::ApplicationBasic::Attributes::VendorName::TypeInfo>
{
public:
    VendorNameReader() : MediaReadBase(chip::app::Clusters::ApplicationBasic::Id) {}
};

class VendorIDReader : public MediaReadBase<chip::app::Clusters::ApplicationBasic::Attributes::VendorID::TypeInfo>
{
public:
    VendorIDReader() : MediaReadBase(chip::app::Clusters::ApplicationBasic::Id) {}
};

class ApplicationNameReader : public MediaReadBase<chip::app::Clusters::ApplicationBasic::Attributes::ApplicationName::TypeInfo>
{
public:
    ApplicationNameReader() : MediaReadBase(chip::app::Clusters::ApplicationBasic::Id) {}
};

class ProductIDReader : public MediaReadBase<chip::app::Clusters::ApplicationBasic::Attributes::ProductID::TypeInfo>
{
public:
    ProductIDReader() : MediaReadBase(chip::app::Clusters::ApplicationBasic::Id) {}
};

class ApplicationReader : public MediaReadBase<chip::app::Clusters::ApplicationBasic::Attributes::Application::TypeInfo>
{
public:
    ApplicationReader() : MediaReadBase(chip::app::Clusters::ApplicationBasic::Id) {}
};

class StatusReader : public MediaReadBase<chip::app::Clusters::ApplicationBasic::Attributes::Status::TypeInfo>
{
public:
    StatusReader() : MediaReadBase(chip::app::Clusters::ApplicationBasic::Id) {}
};

class ApplicationVersionReader
    : public MediaReadBase<chip::app::Clusters::ApplicationBasic::Attributes::ApplicationVersion::TypeInfo>
{
public:
    ApplicationVersionReader() : MediaReadBase(chip::app::Clusters::ApplicationBasic::Id) {}
};

class AllowedVendorListReader : public MediaReadBase<chip::app::Clusters::ApplicationBasic::Attributes::AllowedVendorList::TypeInfo>
{
public:
    AllowedVendorListReader() : MediaReadBase(chip::app::Clusters::ApplicationBasic::Id) {}
};
