# OTA Requestor

This is an implementation of the Matter OTA Requestor functionality that can be
used by Matter applications for OTA software updates

## Steps for including the OTA Requestor functionality in a Matter application

-   Enable the OTA Requestor Server cluster and the OTA Provider cluster in the
    application

-   Explicitly list all the source files in `src/app/clusters/ota-requestor` in
    the application's make/build file. See for example
    `examples/ota-requestor-app/ota-requestor-common/BUILD.gn`

-   Implement a class derived from `OTARequestorDriver`, see for example
    `LinuxOTARequestorDriver`. This class would typically be a part of the
    application.

-   In the application initialization logic create an instance of the
    `OTARequestor` class and register it through `SetRequestorInstance()`.
    Create an instance of the `OTARequestorDriver` implementation and connect it
    to the `OTARequestor` object by calling
    `OTARequestor::SetOtaRequestorDriver()`

-   Implement a class derived from `OTAImageProcessorInterface`, see for example
    `LinuxOTAImageProcessor`. This class would typically be a part of the
    platform.

-   In the application initialization logic create an instance of the
    `BDXDownloader` class. Create an instance of the
    `OTAImageProcessorInterface`-derived implementation (e.g.
    `LinuxOTAImageProcessor`) and connect it to the `BDXDownloader` object by
    calling `BDXDownloader::SetImageProcessorDelegate()`

-   See `examples/ota-requestor-app/linux/main.cpp` for an example of the
    initialization code discussed above

-   Implement application- and platform-specific logic related to image download
    and update such as platform-specific image storage and validation,
    application-specific logic for triggering image query and applying the
    downloaded image, etc.

-   The interface between the core OTA Requestor functionality and the
    platform/application logic is realized through the virtual functions of
    `OTARequestorDriver` and `OTAImageProcessorInterface` and through the
    application interface methods of `OTARequestor` and `BDXDownloader`.

## Design Overview

To be completed..
