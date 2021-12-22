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

/** @file "ota.cpp"
 *
 * OTA handling for an application
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

#include <platform/CHIPDeviceLayer.h>

#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/OTARequestor.h>
#include <platform/GenericOTARequestorDriver.h>
#include <platform/qpg/OTAImageProcessorImpl.h>

using namespace chip;
using namespace chip::DeviceLayer;

/*****************************************************************************
 *                    Static Data Definitions
 *****************************************************************************/

OTARequestor gRequestorCore;
GenericOTARequestorDriver gRequestorUser;
BDXDownloader gDownloader;
OTAImageProcessorImpl gImageProcessor;

/*****************************************************************************
 *                    Application Function Definitions
 *****************************************************************************/

void InitializeOTARequestor(void)
{
    // Initialize and interconnect the Requestor and Image Processor objects
    SetRequestorInstance(&gRequestorCore);

    // Set server instance used for session establishment
    chip::Server * server = &(chip::Server::GetInstance());
    gRequestorCore.SetServerInstance(server);

    // Connect the Requestor and Requestor Driver objects
    gRequestorCore.SetOtaRequestorDriver(&gRequestorUser);
    gRequestorUser.Init(&gRequestorCore, &gImageProcessor);

    // WARNING: this is probably not realistic to know such details of the image or to even have an OTADownloader instantiated at
    // the beginning of program execution. We're using hardcoded values here for now since this is a reference application.
    // TODO: instantiate and initialize these values when QueryImageResponse tells us an image is available
    // TODO: add API for OTARequestor to pass QueryImageResponse info to the application to use for OTADownloader init
    OTAImageProcessorParams ipParams;
    ipParams.imageFile = CharSpan("test.txt");
    gImageProcessor.SetOTAImageProcessorParams(ipParams);
    gImageProcessor.SetOTADownloader(&gDownloader);

    // Connect the Downloader and Image Processor objects
    gDownloader.SetImageProcessorDelegate(&gImageProcessor);

    gRequestorCore.SetBDXDownloader(&gDownloader);
}

void TriggerOTAQuery(void)
{
    static_cast<OTARequestor *>(GetRequestorInstance())->TriggerImmediateQuery();
}