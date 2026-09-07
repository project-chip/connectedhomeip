# Camera AV Stream Management Cluster Server Implementation

This directory contains the server-side implementation of the Camera AV Stream
Management Cluster. This implementation is code-driven, and it does not rely on
ZAP tool code generation for its core logic, and applications should not use the
ZAP tool for configuring its attributes and commands.

## Purpose

The Camera AV Stream Management Cluster manages the allocation, configuration,
and lifecycle of audio and video streams from a camera device. It allows
commissioners to request streams for various purposes like live viewing,
recording, and analysis, while enabling the device to manage its encoding and
streaming resources.

## Key Components

1.  **`CameraAVStreamManagementCluster`:** The main class implementing the
    cluster logic, attribute storage, and command handling.
2.  **`CameraAVStreamManagementDelegate`:** A pure virtual interface that the
    application MUST implement. This delegate is responsible for interacting
    with the device's underlying camera, audio, and encoder hardware/software
    stack to fulfill stream requests and report status.

## Integration Guide

To use this cluster implementation in your application, follow the pattern in
`examples/camera-app/camera-common/src/camera-app.cpp`:

### 1. Implement the Delegate

Create a concrete class inheriting from `CameraAVStreamManagementDelegate` and
implement all its pure virtual methods. This class will bridge the cluster logic
with your device's specific media pipelines.

### 2. Instantiate and Initialize the Cluster

Within your application setup (e.g., in a function like
`CreateAndInitializeCameraAVStreamMgmt`):

    a.  **Determine Features and Optional Attributes:** Based on your device capabilities, populate `BitFlags<CameraAvStreamManagement::Feature>` and `BitFlags<CameraAvStreamManagement::OptionalAttribute>`. Use helper functions or interfaces (like `CameraHALInterface` in the example) to query device capabilities.

    b.  **Gather Configuration Parameters:** Collect all necessary parameters for the `CameraAVStreamManagementCluster` constructor. This includes:
        *   Max concurrent encoders
        *   Max encoded pixel rate
        *   Video sensor parameters
        *   Audio/Speaker capabilities
        *   Snapshot capabilities
        *   Supported stream usages, etc.
        Again, source these values from your device's hardware abstraction layer.

    c.  **Create Cluster Instance:** Instantiate `CameraAVStreamManagementCluster` on the heap or as a member variable, passing the delegate instance, endpoint ID, features, optional attributes, and all configuration parameters gathered above.

The example uses a `ServerCluster<CameraAVStreamManagementCluster>` member and a
`Create` method.

    d.  **Register Cluster:** Register the created cluster instance with the Matter data model registry for the desired endpoint.

    e.  **Set Initial Attribute Values:** After creation, set the initial values for writable attributes based on the device's current state. This is typically done by calling the cluster's setter methods (e.g., `SetHDRModeEnabled`, `SetViewport`, etc.).

    f.  **Call `Init()`:** Finally, call the `Init()` method on the cluster instance. This performs essential validation of the configuration.

Refer to the Matter specification, the header files
(`CameraAVStreamManagementCluster.h`, `CameraAVStreamManagementDelegate.h`), and
the `camera-app.cpp` example for full details.
