/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/enums.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/clusters/laundry-washer-controls-server/laundry-washer-controls-server.h>
#include <app/server/Server.h>
#include <app/util/error-mapping.h>
#include <lib/core/CHIPEncoding.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LaundryWasherControls;
using namespace chip::app::Clusters::LaundryWasherControls::Attributes;
using chip::Protocols::InteractionModel::Status;

LaundryWasherControlsServer LaundryWasherControlsServer::sInstance;

/**********************************************************
 * LaundryWasherControlsServer public methods
 *********************************************************/

CHIP_ERROR LaundryWasherControlsServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    if (aPath.mClusterId != LaundryWasherControls::Id)
    {
        // We shouldn't have been called at all.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    switch (aPath.mAttributeId)
    {
    case Attributes::SpinSpeeds::Id:
        return ReadSpinSpeeds(aPath, aEncoder);
    case Attributes::SupportedRinses::Id:
        return ReadSupportedRinses(aPath, aEncoder);
    default:
        break;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR LaundryWasherControlsServer::Init()
{
    mLaundryWasherDataProvider.Init(Server::GetInstance().GetPersistentStorage());

    return CHIP_NO_ERROR;
}

LaundryWasherControlsServer & LaundryWasherControlsServer::Instance()
{
    return sInstance;
}

EmberAfStatus LaundryWasherControlsServer::SetSpinSpeedCurrent(EndpointId endpointId, DataModel::Nullable<uint8_t> newSpinSpeedCurrent)
{
    DataModel::Nullable<uint8_t> spinSpeedCurrent;
    EmberAfStatus res = SpinSpeedCurrent::Get(endpointId, spinSpeedCurrent);

    if ((res == EMBER_ZCL_STATUS_SUCCESS) && (spinSpeedCurrent != newSpinSpeedCurrent))
    {
        res = SpinSpeedCurrent::Set(endpointId, newSpinSpeedCurrent);
    }

    return res;
}

EmberAfStatus LaundryWasherControlsServer::GetSpinSpeedCurrent(EndpointId endpointId, DataModel::Nullable<uint8_t> & spinSpeedCurrent)
{
    return SpinSpeedCurrent::Get(endpointId, spinSpeedCurrent);
}

EmberAfStatus LaundryWasherControlsServer::SetNumberOfRinses(EndpointId endpointId, NumberOfRinsesEnum newNumberOfRinses)
{
    NumberOfRinsesEnum numberOfRinses;
    EmberAfStatus res = NumberOfRinses::Get(endpointId, &numberOfRinses);

    if ((res == EMBER_ZCL_STATUS_SUCCESS) && (numberOfRinses != newNumberOfRinses))
    {
        res = NumberOfRinses::Set(endpointId, newNumberOfRinses);
    }

    return res;
}

EmberAfStatus LaundryWasherControlsServer::GetNumberOfRinses(EndpointId endpointId, NumberOfRinsesEnum & numberOfRinses)
{
    return NumberOfRinses::Get(endpointId, &numberOfRinses);
}

EmberAfStatus LaundryWasherControlsServer::SetSpinSpeedList(EndpointId endpointId, const SpinSpeedList & spinSpeedList)
{
    CHIP_ERROR err = mLaundryWasherDataProvider.StoreSpinSpeedList(endpointId, LaundryWasherControls::Id, spinSpeedList);
    return (err == CHIP_NO_ERROR)?(EMBER_ZCL_STATUS_SUCCESS):(EMBER_ZCL_STATUS_FAILURE);
}

EmberAfStatus LaundryWasherControlsServer::GetSpinSpeedList(EndpointId endpointId, SpinSpeedListCharSpan ** spinSpeedList, size_t & size)
{
    CHIP_ERROR err = mLaundryWasherDataProvider.LoadSpinSpeedList(endpointId, LaundryWasherControls::Id, spinSpeedList, size);
    return (err == CHIP_NO_ERROR)?(EMBER_ZCL_STATUS_SUCCESS):(EMBER_ZCL_STATUS_FAILURE);
}

EmberAfStatus LaundryWasherControlsServer::SetSupportedRinsesList(EndpointId endpointId, const SupportedRinsesList & supportedRinsesList)
{
    CHIP_ERROR err = mLaundryWasherDataProvider.StoreSupportedRinsesList(endpointId, LaundryWasherControls::Id, supportedRinsesList);
    return (err == CHIP_NO_ERROR)?(EMBER_ZCL_STATUS_SUCCESS):(EMBER_ZCL_STATUS_FAILURE);
}

EmberAfStatus LaundryWasherControlsServer::GetSupportedRinsesList(EndpointId endpointId, SupportedRinsesListSpan ** supportedRinsesList, size_t & size)
{
    CHIP_ERROR err = mLaundryWasherDataProvider.LoadSupportedRinsesList(endpointId, LaundryWasherControls::Id, supportedRinsesList, size);
    return (err == CHIP_NO_ERROR)?(EMBER_ZCL_STATUS_SUCCESS):(EMBER_ZCL_STATUS_FAILURE);
}

/**********************************************************
 * LaundryWasherControlsServer private methods
 *********************************************************/
void LaundryWasherControlsServer::ReleaseSpinSpeedList(SpinSpeedListCharSpan * spinSpeedList)
{
    mLaundryWasherDataProvider.ReleaseSpinSpeedList(spinSpeedList);
}

CHIP_ERROR LaundryWasherControlsServer::ClearSpinSpeedList(EndpointId endpointId)
{
    return mLaundryWasherDataProvider.ClearSpinSpeedList(endpointId, LaundryWasherControls::Id);
}

void LaundryWasherControlsServer::ReleaseSupportedRinsesList(SupportedRinsesListSpan* supportedRinsesList)
{
    mLaundryWasherDataProvider.ReleaseSupportedRinsesList(supportedRinsesList);
}

CHIP_ERROR LaundryWasherControlsServer::ClearSupportedRinsesList(EndpointId endpointId)
{
    return mLaundryWasherDataProvider.ClearSupportedRinsesList(endpointId, LaundryWasherControls::Id);
}

CHIP_ERROR LaundryWasherControlsServer::ReadSpinSpeeds(const ConcreteReadAttributePath & aPath,
                                                           AttributeValueEncoder & aEncoder)
{
    SpinSpeedListCharSpan * spinSpeedList = nullptr;
    size_t size                   = 0;
    EndpointId endpointId = aPath.mEndpointId;
    GetSpinSpeedList(endpointId, &spinSpeedList, size);
    CHIP_ERROR err = CHIP_NO_ERROR;

    aEncoder.EncodeEmptyList();
    if (size > 0)
    {
        return aEncoder.EncodeList([&](const auto & encoder) -> CHIP_ERROR {
            for (SpinSpeedListCharSpan * pHead = spinSpeedList; pHead != nullptr; pHead = pHead->Next)
            {
                ReturnErrorOnFailure(encoder.Encode(pHead->spinSpeed));
            }
            ReleaseSpinSpeedList(spinSpeedList);
            spinSpeedList = nullptr;
            return CHIP_NO_ERROR;
        });
    }
    return err;
}

CHIP_ERROR LaundryWasherControlsServer::ReadSupportedRinses(const ConcreteReadAttributePath & aPath,
                                                                AttributeValueEncoder & aEncoder)
{
    SupportedRinsesListSpan * supportedRinsesList = nullptr;
    size_t size                   = 0;
    EndpointId endpointId = aPath.mEndpointId;
    GetSupportedRinsesList(endpointId, &supportedRinsesList, size);
    CHIP_ERROR err = CHIP_NO_ERROR;

    aEncoder.EncodeEmptyList();
    if (size > 0)
    {
        return aEncoder.EncodeList([&](const auto & encoder) -> CHIP_ERROR {

            for (SupportedRinsesListSpan* pHead = supportedRinsesList; pHead != nullptr; pHead = pHead->Next)
            {
                ReturnErrorOnFailure(encoder.Encode(pHead->numberOfRinses));
            }
            ReleaseSupportedRinsesList(supportedRinsesList);
            supportedRinsesList = nullptr;
            return CHIP_NO_ERROR;
        });
    }
    return err;
}

/**********************************************************
 * Register LaundryWasherControlsServer
 *********************************************************/

void MatterLaundryWasherControlsPluginServerInitCallback()
{
    LaundryWasherControlsServer & laundryWasherControlsServer = LaundryWasherControlsServer::Instance();
    registerAttributeAccessOverride(&laundryWasherControlsServer);
    laundryWasherControlsServer.Init();
}

