# OTA Requestor

This is an implementation of the Matter OTA Requestor functionality that can be
used by Matter applications for OTA software updates

## Design Overview

There are various components defined to support OTA software updates. The Matter
SDK supplies the default implementation to all components. If so desired, a
custom implementation may be used to replace the default implementation.

### OTARequestorInterface

This is an interface for processing the core requestor logic. This includes
sending commands to the OTA Provider cluster as well as handling the responses
for those commands. This component also maintains the server attributes for the
OTA Requestor cluster.

`DefaultOTARequestor` class is the default implementation of this interface. Any
custom implementation should reside under `examples/platform/<platform-name>`.

### OTARequestorDriver

This is an interface for using/driving the `OTARequestorInterface`. This
component determines the next action to take based on the current status
returned by `OTARequestorInterface`. For instance, after `OTARequestorInterface`
receives a QueryImageResponse that an update is available, it informs
`OTARequestorDriver` which then decides whether it is ready to immediately start
the download or to wait on some conditions.

`DefaultOTARequestorDriver` class is the default implementation of this
interface. Any custom implementation should reside under
`examples/platform/<platform-name>`.

Please note the following implementation choices in the default implementation:

-   Upon being notified that an update is available, the image is
    unconditionally downloaded
-   Upon being notified that the provider is busy, a max number of retries is
    attempted before the next provider in the default OTA provider list (if one
    exists) is attempted
-   Upon being notified that an update is not available, querying of the next
    provider in the default OTA Provider list (if one exists) is attempted
-   Upon being notified that an update has been downloaded, the request to apply
    the image is immediately sent to the provider
-   Upon being notified that the update may proceed after download, the image is
    unconditionally applied via the `OTAImageProcessorInterface`
-   Upon being notified that the update should be discontinued after download,
    the entire process is aborted
-   Upon a first time boot into a newly applied image, if the image can be
    confirmed, the provider is immediately notified of the update being applied
    successfully
-   If an existing OTA update is already in progress, any new attempts to query
    will be denied
-   A periodic query is attempted every 24 hours, unless overridden
-   The periodic query timer starts whenever `OTARequestorInterface` is in the
    idle state

### OTAImageProcessorInterface

This is a platform-agnostic interface for processing downloaded chunks of OTA
image data. The data could be raw image data meant for flash or metadata. This
component should interact with the `OTADownloader` to drive the download
process.

Each platform should provide an implementation of this interface which should
reside under `src/platform/<platform-name>`.

### OTADownloader

This is an interface for image download functionality over a particular
protocol. Each `DownloadProtocolEnum` supported should provide an implementation
of this interface.

`BDXDownloader` class is an implementation of this interface for the BDX
protocol.

### OTARequestorStorage

This is an interface for storing/loading persistent data related to OTA.

`DefaultOTARequestorStorage` class is the default implementation of this
interface. Any custom implementation should reside under
`examples/platform/<platform-name>`.

## Steps for including the OTA Requestor functionality in a Matter application

-   Enable `Server` for the OTA Software Update Requestor cluster in the
    application zap file
-   Enable `Client` for the OTA Software Update Provider cluster in the
    application zap file
-   Implement OTA Requestor components:
    -   Use the `DefaultOTARequestor` class or implement a class derived from
        `OTARequestorInterface`
    -   Use the `DefaultOTARequestorDriver` class or implement a class derived
        from `OTARequestorDriver`
    -   Use the `BDXDownloader` class or implement a class derived from
        `OTADownloader`
    -   Implement a class derived from `OTAImageProcessorInterface`
    -   Use the `DefaultOTARequestorStorage` class or implement a class derived
        from `OTARequestorStorage`
-   If using the default implementation of the interfaces defined above,
    explicitly list all the source files in `src/app/clusters/ota-requestor` in
    the application make/build file. For example: `src/app/chip_data_model.gni`.
    Otherwise, list the source files where the component implementation reside.
-   Explicitly list all the OTA platform specific files in `src/platform`. For
    example: `src/platform/Linux/BUILD.gn`
-   Instantiate and initialize each component in the application. For example,
    in an application which uses the default implementation on the Linux
    platform:
    -   Create an instance of the `DefaultOTARequestor` class
    -   Create an instance of the `DefaultOTARequestorDriver` class
    -   Create an instance of the `OTAImageProcessorImpl` class from
        `src/platform/Linux/OTAImageProcessorImpl.h`
    -   Create an instance of the `BDXDownloader` class
    -   Create an instance of the `DefaultOTARequestorStorage` class
    -   Register the instance of `DefaultOTARequestor` through
        `SetRequestorInstance()`
    -   Initialize the instance of `DefaultOTARequestorStorage` through
        `DefaultOTARequestorStorage::Init`
    -   Connect the instances of `DefaultOTARequestorStorage`,
        `DefaultOTARequestorDriver`, and `BDXDownloader` with the instance of
        `DefaultOTARequestor` through `DefaultOTARequestor::Init`()
    -   Connect the instances of `DefaultOTARequestor` and
        `OTAImageProcessorImpl` with the instance of `DefaultOTARequestorDriver`
        through `DefaultOTARequestorDriver::Init`(). It is important that this
        is performed after `DefaultOTARequestor::Init` as there are dependencies
        that `DefaultOTARequestor` already has access to
        `DefaultOTARequestorDriver` by the time this initialization occurs.
    -   Connect the instance of `BDXDownloader` with the instance of
        `OTAImageProcessorImpl` through
        `OTAImageProcessorImpl::SetOTADownloader`
    -   Connect the instance of `OTAImageProcessorImpl` with the instance of
        `BDXDownloader` through `OTADownloader::SetImageProcessorDelegate`
-   See `examples/ota-requestor-app/linux/main.cpp` for an example of the
    initialization code above
