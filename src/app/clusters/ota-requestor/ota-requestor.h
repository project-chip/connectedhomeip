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

/* This file contains the declarations for the Matter OTA Requestor implementation and API.
 *  Applications and platforms implementing the OTA Requestor functionality must include this
 * file
 */

#pragma once

// Interface class to abstract the OTA-related business logic. Each application
// must implement this interface. All calls must be non-blocking unless stated otherwise
class OTARequestorDriver {
public:
    // A call into the application logic to give it a chance to allow or stop the Requestor
    // from proceeding with actual image download.
    virtual bool AllowImageDownload() = 0;

    // Notify the application that the download is complete and the image can be applied
    virtual void ImageReadyToBeApplied() = 0;
};

// Interface class to connect the OTA Software Update Requestor cluster command processing
// with the core OTA Requestor logic. The OTARequestor class implements this interface
class OTARequestorInterface {
public:
    // Handler for the AnnounceOTAProvider command
    virtual bool HandleAnnounceOTAProvider(commandData_t) = 0;

    // Handler for the QueryImageResponse command
    virtual bool HandleQueryImageResponse(commandData_t) = 0;
};

// This class implements all of the core logic of the OTA Requestor 
class OTARequestor : public OTARequestorInterface {
public:
    // Application interface declarations start

    // Application directs the Requestor to start the Image Query process
    // and download and apply the new image if available
    void TriggerImmediateQuery();

    // Application interface declarations end

private:
    OTARequestorDriver *driver;
};

// This is a platform-agnostic interface for processing downloaded
// chunks of OTA image data (data could be raw image data meant for flash or
// metadata). Each platform should provide an implementation of this
// interface.
class OTAImageProcessor {
public:
    // Open file, find block of space in persistent memory, or allocate a buffer, etc.
    virtual CHIP_ERROR PrepareDownload() = 0;

    // Must not be a blocking call to support cases that require IO to elements such as // external peripherals/radios
    virtual CHIP_ERROR ProcessBlock(ByteSpan & data) = 0;

    // Close file, close persistent storage, etc
    // (probably should not actually apply the image)
    virtual CHIP_ERROR Finalize() = 0;

    virtual chip::Optional<uint8_t> PercentComplete() = 0;

    // Clean up the download which could mean erasing everything that was written,
    // releasing buffers, etc.
    virtual CHIP_ERROR Abort() = 0;
};

// A class that abstracts the image download functionality from the particular
// protocol used for that (BDX or possibly HTTPS)
class OTADownloader : public BlockWriter {
public:

    // API declarations start

    // Application calls this method to direct OTADownloader to begin the download
    void BeginDownload();

    // Platform calls this method upon the completion of PrepareDownload() processing
    void OnDownloadPrepared();

    // Platform calls this method upon the completion of ProcessBlock() processing
    void OnBlockProcessed(action);

    // API declarations end

    // Invoked by an incoming block
    void HandleBlock();
};


