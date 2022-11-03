/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorDriver.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorStorage.h>
#include <app/server/Server.h>
#include <platform/nrfconnect/OTAImageProcessorImpl.h>

using namespace chip;
using namespace chip::DeviceLayer;

namespace {

DefaultOTARequestorStorage sOTARequestorStorage;
DefaultOTARequestorDriver sOTARequestorDriver;
chip::BDXDownloader sBDXDownloader;
chip::DefaultOTARequestor sOTARequestor;
} // namespace

FlashHandler & GetFlashHandler()
{
    static FlashHandler sFlashHandler;
    return sFlashHandler;
}

// compile-time factory method
OTAImageProcessorImpl & GetOTAImageProcessor()
{
#if CONFIG_PM_DEVICE && CONFIG_NORDIC_QSPI_NOR
    static OTAImageProcessorImpl sOTAImageProcessor(&GetFlashHandler());
#else
    static OTAImageProcessorImpl sOTAImageProcessor;
#endif
    return sOTAImageProcessor;
}

void InitBasicOTARequestor()
{
    VerifyOrReturn(GetRequestorInstance() == nullptr);

    OTAImageProcessorImpl & imageProcessor = GetOTAImageProcessor();
    imageProcessor.SetOTADownloader(&sBDXDownloader);
    sBDXDownloader.SetImageProcessorDelegate(&imageProcessor);
    sOTARequestorStorage.Init(Server::GetInstance().GetPersistentStorage());
    sOTARequestor.Init(Server::GetInstance(), sOTARequestorStorage, sOTARequestorDriver, sBDXDownloader);
    chip::SetRequestorInstance(&sOTARequestor);
    sOTARequestorDriver.Init(&sOTARequestor, &imageProcessor);
    imageProcessor.TriggerFlashAction(FlashHandler::Action::SLEEP);
}
