# Camera App Architecture and Design

This document provides a detailed overview of the design and architecture of the
Matter camera application, focusing on the Linux implementation.

## High-Level Architecture

The camera application is designed with a clear separation between the generic
Matter cluster logic and the platform-specific hardware abstraction. This is
achieved through the use of a `CameraDeviceInterface`, which defines the
contract that any platform-specific camera implementation must adhere to.

The core components are:

-   **Camera App (`CameraApp`):** Responsible for initializing and managing the
    Matter clusters related to camera functionality. It is platform-agnostic and
    interacts with the hardware through the `CameraDeviceInterface`.
-   **Camera Device (`CameraDevice`):** The Linux-specific implementation of the
    `CameraDeviceInterface`. It manages the camera hardware, using V4L2 and
    GStreamer, and provides the necessary delegates for the Matter clusters.
-   **Media Controller (`DefaultMediaController`):** The central hub for media
    data distribution. It receives encoded media frames from the `CameraDevice`
    and distributes them to the various transport managers.
-   **Transport Managers (`WebRTCProviderManager`,
    `PushAvStreamTransportManager`):** These classes manage the specific
    transport protocols for streaming media to clients.
-   **Matter Cluster Managers:** The various Matter cluster managers that expose
    the camera's functionality to the Matter network (e.g.,
    `CameraAVStreamManagement`, `WebRTCTransportProvider`).

This layered architecture allows for easy porting of the camera application to
other platforms by simply providing a new implementation of the
`CameraDeviceInterface`.

## Component Breakdown

### `CameraApp`

-   **Responsibilities:**
    -   Initializes and configures the Matter clusters for camera functionality
        (e.g., `CameraAVStreamManagementCluster`,
        `WebRTCTransportProviderCluster`).
    -   Fetches camera capabilities and settings from the
        `CameraDeviceInterface` to configure the clusters.
    -   Manages the lifecycle of the cluster servers.
-   **Key Interactions:**
    -   Takes a `CameraDeviceInterface` pointer in its constructor.
    -   Calls methods on the `CameraDeviceInterface` to get delegates and
        hardware information.

### `CameraDeviceInterface`

-   **Responsibilities:**
    -   Defines the abstract interface for a camera device.
    -   Declares methods for accessing the various delegates required by the
        Matter clusters.
    -   Defines the `CameraHALInterface`, an inner interface that abstracts the
        hardware-specific operations (e.g., starting/stopping streams, taking
        snapshots).

### `CameraDevice`

-   **Responsibilities:**
    -   Implements the `CameraDeviceInterface` and
        `CameraDeviceInterface::CameraHALInterface` for the Linux platform.
    -   Manages the video device using V4L2.
    -   Uses GStreamer to create and manage pipelines for video and audio
        streaming, as well as snapshots.
    -   Instantiates and manages the various manager classes that implement the
        cluster delegate logic (e.g., `CameraAVStreamManager`,
        `WebRTCProviderManager`).
    -   Instantiates the `DefaultMediaController` and passes the encoded media
        frames to it.
    -   Maintains the state of the camera device (e.g., pan, tilt, zoom, privacy
        modes).
    -   Provides the `CameraHALInterface` implementation to the
        `CameraAVStreamManager`.
-   **Key Interactions:**
    -   Is instantiated in `main.cpp` and passed to `CameraAppInit`.
    -   The manager classes it contains are returned by the `Get...Delegate()`
        methods to the `CameraApp`.
    -   Its `CameraHALInterface` methods (like `StartVideoStream`,
        `StopVideoStream`) are called by `CameraAVStreamManager`.

### `DefaultMediaController`

-   **Responsibilities:**
    -   Acts as the central distribution point for all encoded media data (video
        and audio).
    -   Receives media frames from the `CameraDevice`'s GStreamer pipeline
        callbacks.
    -   Maintains a pre-roll buffer (`PushAVPreRollBuffer`) that stores a
        configurable duration of recent media frames. This is crucial for
        event-based recording, as it allows the recording to include footage
        from before the event occurred.
    -   Manages a list of registered transports (e.g., `WebRTCTransport`,
        `PushAVTransport`).
    -   When a new media frame is received, it is pushed to the pre-roll buffer,
        which then multicasts the frame to all interested and registered
        transports.
-   **Key Interactions:**
    -   Is owned by the `CameraDevice`.
    -   The `WebRTCProviderManager` and `PushAvStreamTransportManager` register
        their transport instances with the `MediaController`.

### Manager Classes and SDK Interaction

The manager classes in the camera-app are concrete implementations of the
delegate interfaces defined in the Matter SDK. They act as a bridge between the
generic cluster logic in the SDK and the specific hardware implementation in the
camera-app.

-   **`CameraAVStreamManager`:**

    -   **SDK Interface:**
        `chip::app::Clusters::CameraAvStreamManagement::CameraAVStreamManagementDelegate`
    -   **Responsibilities:** Implements the application-specific logic for the
        Camera AV Stream Management cluster. This includes:
        -   Handling requests to allocate, deallocate, and modify video, audio,
            and snapshot streams (`VideoStreamAllocate`, `AudioStreamAllocate`,
            etc.).
        -   Validating stream parameters against camera capabilities obtained
            from `CameraDeviceHAL`.
        -   Checking for resource availability (e.g., encoder slots).
        -   Interacting with the `CameraDevice` (specifically its
            `CameraHALInterface` implementation) to start and stop the GStreamer
            pipelines for the various streams (e.g., calling `StartVideoStream`,
            `StopVideoStream`).
        -   Notifying the SDK cluster of allocation/deallocation results.
    -   **Interaction with SDK:** The `CameraAVStreamManagementCluster` in the
        SDK calls the methods of the `CameraAVStreamManager` to handle the
        commands it receives from the Matter network. For example, when a
        `VideoStreamAllocate` command is received, the
        `CameraAVStreamManagementCluster` calls the `VideoStreamAllocate` method
        on the `CameraAVStreamManager`.
    -   **Interaction with `CameraDevice`:** It holds a
        `CameraDeviceInterface * mCameraDeviceHAL` pointer, set via
        `SetCameraDeviceHAL`. When a stream needs to be started, stopped, or
        modified, `CameraAVStreamManager` calls the appropriate method on
        `mCameraDeviceHAL->GetCameraHALInterface()`, e.g.,
        `mCameraDeviceHAL->GetCameraHALInterface().StartVideoStream(allocatedStream)`.

-   **`WebRTCProviderManager`:**

    -   **SDK Interface:**
        `chip::app::Clusters::WebRTCTransportProvider::Delegate`
    -   **Responsibilities:** Manages the lifecycle of WebRTC sessions for live
        streaming.
    -   **Session Initiation:**
        -   `HandleSolicitOffer`: When a client wants the camera to initiate the
            WebRTC handshake, this method creates a `WebrtcTransport`, generates
            an SDP Offer, and sends it to the client.
        -   `HandleProvideOffer`: When a client initiates the handshake, this
            method processes the received SDP Offer, creates a
            `WebrtcTransport`, generates an SDP Answer, and sends it back.
    -   **Handshake:** Manages the exchange of SDP messages and ICE candidates
        between the camera and the WebRTC client. It uses callbacks like
        `OnLocalDescription` to send the locally generated SDP, and
        `HandleProvideICECandidates` to process candidates from the client.
    -   **Media Flow:** Once the WebRTC connection is established
        (`OnConnectionStateChanged(Connected)`), it registers the
        `WebrtcTransport` with the `DefaultMediaController` to receive and send
        audio/video frames.
    -   **Stream Management:** Acquires and releases audio/video stream
        resources from `CameraAVStreamManager` using `AcquireAudioVideoStreams`
        and `ReleaseAudioVideoStreams`.
    -   **Privacy:** Handles `LiveStreamPrivacyModeChanged` to end sessions if
        live stream privacy is enabled.

-   **`PushAvStreamTransportManager`:**

    -   **SDK Interface:**
        `chip::app::Clusters::PushAvStreamTransport::Delegate`
    -   **Responsibilities:** Manages push-based AV streaming for event-based
        recording (e.g., CMAF).
    -   **Allocation:** `AllocatePushTransport` creates a `PushAVTransport`
        object for a given client request. This object is configured with
        details like container type (CMAF), segment duration, and target
        streams.
    -   **Registration:** The created `PushAVTransport` is registered with the
        `DefaultMediaController` to access media data, including the pre-roll
        buffer.
    -   **Triggering:**
        -   `ManuallyTriggerTransport`: Allows a client to force a recording.
        -   `HandleZoneTrigger`: Called by `CameraDevice` when a motion zone
            alarm is raised. This checks which `PushAVTransport` instances are
            configured for that zone and initiates the recording and upload
            process.
    -   **Bandwidth Management:** Validates that new or modified transport
        configurations do not exceed the camera's maximum network bandwidth
        (`ValidateBandwidthLimit`).
    -   **Session Management:** Monitors active recording sessions and can
        restart them to limit maximum session duration, generating new session
        IDs.

-   **`ZoneManager`:**

    -   **SDK Interface:** `chip::app::Clusters::ZoneManagement::Delegate`
    -   **Responsibilities:**
        -   Manages the creation, update, and removal of 2D Cartesian zones.
        -   Handles the creation and management of triggers associated with
            zones (e.g., motion detection).
        -   Receives zone event notifications from the `CameraDevice` HAL (e.g.,
            `OnZoneTriggeredEvent`).
        -   Emits `ZoneTriggered` and `ZoneStopped` Matter events to
            subscribers.
        -   Manages trigger logic, including initial duration, augmentation
            duration, max duration, and blind duration using an internal timer.
    -   **Key Interactions:**
        -   `CreateTrigger`, `UpdateTrigger`, `RemoveTrigger` commands delegate
            to the `CameraHALInterface`.
        -   `CameraDevice` calls `OnZoneTriggeredEvent` on this manager when the
            HAL detects activity in a zone.
        -   `PushAvStreamTransportManager` is notified of zone triggers to start
            recordings.

-   **`CameraAVSettingsUserLevelManager`:**

    -   **SDK Interface:**
        `chip::app::Clusters::CameraAvSettingsUserLevelManagement::Delegate`
    -   **Responsibilities:** Handles user-level settings, including Pan, Tilt,
        Zoom.
    -   **Mechanical PTZ (MPTZ):**
        -   Commands like `MPTZSetPosition`, `MPTZRelativeMove`, and
            `MPTZMoveToPreset` are received from the SDK cluster.
        -   These commands are delegated to the `CameraHALInterface`
            (`CameraDevice`) to interact with the physical hardware (simulated
            in this app).
        -   The manager simulates the time taken for physical movement using a
            timer before responding to the command.
    -   **Digital PTZ (DPTZ):**
        -   `DPTZSetViewport`: Sets the digital viewport for a _specific
            allocated video stream ID_. It validates the requested viewport
            against the stream's resolution, aspect ratio, and the camera
            sensor's capabilities. The change is applied via
            `CameraHALInterface::SetViewport`.
        -   `DPTZRelativeMove`: Adjusts the current viewport of a specific video
            stream by a delta. Calculations are done to keep the viewport within
            bounds and maintain the aspect ratio.

## Interaction Diagrams

### Component Relationships

```mermaid
graph TD
    subgraph "Platform Agnostic"
        CameraApp
        CameraDeviceInterface
    end

    subgraph "Linux Platform"
        CameraDevice -- implements --> CameraDeviceInterface
        CameraDevice -- owns --> GStreamer
        CameraDevice -- uses --> V4L2
        CameraDevice -- owns --> DefaultMediaController
        CameraDevice -- owns --> CameraAVStreamManager
        CameraDevice -- owns --> WebRTCProviderManager
        CameraDevice -- owns --> PushAvStreamTransportManager
        CameraDevice -- owns --> ZoneManager

        WebRTCProviderManager --> DefaultMediaController
        PushAvStreamTransportManager --> DefaultMediaController
    end

    subgraph "Matter Clusters (SDK)"
        CameraAVStreamManagementCluster
        WebRTCTransportProviderCluster
        PushAvStreamTransportServer
        ZoneMgmtServer
    end

    Main --> CameraDevice
    Main --> CameraApp
    CameraApp -- aggregates --> CameraDeviceInterface

    CameraApp --> CameraAVStreamManagementCluster
    CameraApp --> WebRTCTransportProviderCluster
    CameraApp --> PushAvStreamTransportServer
    CameraApp --> ZoneMgmtServer

    CameraAVStreamManagementCluster -- uses delegate --> CameraAVStreamManager
    WebRTCTransportProviderCluster -- uses delegate --> WebRTCProviderManager
    PushAvStreamTransportServer -- uses delegate --> PushAvStreamTransportManager
    ZoneMgmtServer -- uses delegate --> ZoneManager

    CameraAVStreamManager -- calls HAL --> CameraDevice
    ZoneManager -- calls HAL --> CameraDevice
```

### Video Stream Allocation Sequence

```mermaid
sequenceDiagram
    participant Client
    participant CameraAVStreamManagementCluster
    participant CameraAVStreamManager as Delegate
    participant CameraDevice as HAL

    Client ->> CameraAVStreamManagementCluster: VideoStreamAllocate Request
    CameraAVStreamManagementCluster ->> Delegate: VideoStreamAllocate()
    Delegate ->> HAL: GetAvailableVideoStreams()
    HAL -->> Delegate: List of streams
    Delegate ->> Delegate: Find compatible stream
    Delegate ->> HAL: IsResourceAvailable()
    HAL -->> Delegate: Yes/No
    alt Resources Available
        Delegate -->> CameraAVStreamManagementCluster: Success, streamID
        CameraAVStreamManagementCluster ->> Delegate: OnVideoStreamAllocated()
        Delegate ->> HAL: StartVideoStream(streamID)
        HAL ->> HAL: Configure & Start GStreamer Pipeline
    else Resources NOT Available
        Delegate -->> CameraAVStreamManagementCluster: ResourceExhausted
    end
    CameraAVStreamManagementCluster -->> Client: VideoStreamAllocate Response
```

### Push AV Transport Allocation Sequence

```mermaid
sequenceDiagram
    participant Client
    participant PushAVStreamTransportServer as SDK Cluster
    participant PushAvStreamTransportManager as Delegate
    participant MediaController
    participant CameraAVStreamManager

    Client ->> SDK Cluster: AllocatePushTransport Request
    SDK Cluster ->> Delegate: AllocatePushTransport()
    Delegate ->> CameraAVStreamManager: GetBandwidthForStreams()
    CameraAVStreamManager -->> Delegate: Bandwidth
    Delegate ->> Delegate: ValidateBandwidthLimit()
    alt Bandwidth OK
        Delegate ->> Delegate: Create PushAVTransport instance
        Delegate ->> MediaController: RegisterTransport(PushAVTransport, videoID, audioID)
        MediaController ->> MediaController: Add to transport list
        Delegate ->> MediaController: SetPreRollLength()
        Delegate -->> SDK Cluster: Success
    else Bandwidth Exceeded
        Delegate -->> SDK Cluster: ResourceExhausted
    end
    SDK Cluster -->> Client: AllocatePushTransport Response
```

### WebRTC Livestream Setup Sequence (Client Offer)

```mermaid
sequenceDiagram
    participant Client
    participant WebRTCTransportProviderCluster as SDK Cluster
    participant WebRTCProviderManager as Delegate
    participant WebrtcTransport
    participant MediaController
    participant CameraAVStreamManager

    Client ->> SDK Cluster: ProvideOffer Request (SDP Offer)
    SDK Cluster ->> Delegate: HandleProvideOffer()
    Delegate ->> Delegate: Create WebrtcTransport
    Delegate ->> WebrtcTransport: SetRemoteDescription(Offer)
    Delegate ->> CameraAVStreamManager: AcquireAudioVideoStreams()
    CameraAVStreamManager -->> Delegate: Success
    Delegate ->> WebrtcTransport: CreateAnswer()
    WebrtcTransport -->> Delegate: OnLocalDescription(SDP Answer)
    Delegate ->> Delegate: ScheduleAnswerSend()
    SDK Cluster -->> Client: ProvideOffer Response
    Delegate -->> Client: Answer Command (SDP Answer)

    Client ->> SDK Cluster: ProvideICECandidates Request
    SDK Cluster ->> Delegate: HandleProvideICECandidates()
    Delegate ->> WebrtcTransport: AddRemoteCandidate()
    Note right of Delegate: Meanwhile...
    WebrtcTransport -->> Delegate: OnICECandidate (Local Candidates)
    Delegate ->> Delegate: ScheduleICECandidatesSend()
    Delegate -->> Client: ICECandidates Command

    Note over Client, WebrtcTransport: ICE Connectivity Establishment
    WebrtcTransport -->> Delegate: OnConnectionStateChanged(Connected)
    Delegate ->> MediaController: RegisterTransport(WebrtcTransport, videoID, audioID)
    Note over Client, MediaController: Live Stream Starts
```

### High-Level Data Flow

```mermaid
graph TD
    subgraph "Camera Hardware"
        CameraSensor
    end

    subgraph "Linux Kernel"
        V4L2_Driver
    end

    subgraph "Userspace (Camera App)"
        GStreamer_Pipeline
        CameraDevice
        DefaultMediaController
        WebRTCTransport
        PushAVTransport
    end

    subgraph "Matter Network"
        ClientDevice
    end

    CameraSensor --> V4L2_Driver
    V4L2_Driver --> GStreamer_Pipeline
    GStreamer_Pipeline -- Encoded Data --> CameraDevice
    CameraDevice -- Encoded Data (H.264/Opus) --> DefaultMediaController
    DefaultMediaController --> WebRTCTransport
    DefaultMediaController --> PushAVTransport
    WebRTCTransport --> ClientDevice
    PushAVTransport --> ClientDevice
```

## Data Flow

### Video Streaming

1.  A Matter client requests a video stream from the `CameraAVStreamManagement`
    cluster.
2.  The `CameraAVStreamManagementCluster` (in the SDK) receives the request and
    calls the `VideoStreamAllocate` method on its delegate, the
    `CameraAVStreamManager`.
3.  The `CameraAVStreamManager` validates the request, checks for compatible
    stream configurations and available resources by querying the `CameraDevice`
    (HAL).
4.  If a stream can be allocated, `CameraAVStreamManager` updates the stream
    state.
5.  The SDK server informs the `CameraAVStreamManager` via
    `OnVideoStreamAllocated`.
6.  `CameraAVStreamManager` calls `StartVideoStream` on the `CameraDevice`'s
    `CameraHALInterface`.
7.  The `CameraDevice` creates (if not already running) the shared camera
    capture pipeline (`CreateCameraCapturePipeline`) ending in a `tee` element
    (`stream_tee`), creates a per-stream video branch bin
    (`CreateVideoBranchBin`), and dynamically attaches it via
    `AttachStreamBranch`:
    -   Shared capture pipeline: captures raw or MJPEG frames from `v4l2src` or
        `libcamerasrc` (decoding via `decodebin` on hardware sources) or raw
        frames from `videotestsrc`, applies source caps, converts to `I420`
        (`videoconvert`), and fans out frames through `stream_tee`.
    -   Per-stream video branch bin: buffers and converts to the stream's target
        dimensions and frame rate (`queue` -> `videoscale` -> `videorate` ->
        `capsfilter`), applies privacy-zone `I420` masks on the encoder sink pad
        probe, encodes to H.264 (`x264enc`), and delivers encoded samples to an
        `appsink`.
8.  The `appsink` has a callback function (`OnNewVideoSampleFromAppSink`) that
    is called for each new frame.
9.  In the callback, the encoded H.264 data is retrieved and passed to the
    `DefaultMediaController` (via `DistributeVideoIfCurrentGeneration`).
10. The `DefaultMediaController` pushes the frame to its pre-roll buffer, which
    then distributes it to all registered transports.
11. For a live stream, the `WebRTCTransport` receives the frame and sends it
    over the established WebRTC connection to the client.
12. For an event-based recording, the `PushAVTransport` receives the frame and
    includes it in the recording that is pushed to the client.

### Snapshot

1.  A Matter client allocates a snapshot stream via `SnapshotStreamAllocate`
    (which attempts to start and attach a `CreateSnapshotBranchBin` branch to
    the shared capture `stream_tee`) and requests a snapshot via
    `CaptureSnapshot` on the `CameraAVStreamManagement` cluster.
2.  The `CameraAVStreamManagementCluster` receives the request and calls the
    `CaptureSnapshot` method on the `CameraAVStreamManager`.
3.  `CameraAVStreamManager` delegates the call to
    `CameraDevice::CaptureSnapshot`.
4.  If the matched allocated snapshot stream's branch is not yet active,
    `CameraDevice::CaptureSnapshot` starts it on demand via
    `StartSnapshotStream`. Each snapshot branch bin scales and rate-limits raw
    `I420` frames from `stream_tee`, applies privacy-zone masks on the `jpegenc`
    sink pad probe, encodes JPEG frames (`jpegenc`), and retains the latest
    sample in an `appsink` (`max-buffers=1`, `drop=TRUE`).
5.  `CameraDevice::CaptureSnapshot` pulls a current-generation JPEG `GstSample`
    directly from the branch's `appsink` in memory and returns the encoded bytes
    in `CaptureSnapshotResponse`.

## GStreamer Integration

GStreamer is used extensively in the `CameraDevice` to handle all media
processing. Video and snapshot streams share a single camera capture pipeline
(`CreateCameraCapturePipeline`) fanned out through a `tee` element
(`stream_tee`) into dynamically attached per-stream `GstBin` branches
(`CreateVideoBranchBin`, `CreateSnapshotBranchBin`), allowing concurrent video
streaming and snapshot capture from a single physical V4L2/CSI camera device.
Audio capture and playback use dedicated pipelines (`CreateAudioPipeline`,
`CreateAudioPlaybackPipeline`).

-   **Shared Camera Capture (`CreateCameraCapturePipeline`):**

    -   **Source:** `v4l2src` (USB/V4L2), `libcamerasrc` (CSI), or
        `videotestsrc` (when `--camera-test-videosrc` is enabled).
    -   **Caps & Format Conversion:** `source_caps` directly after the source to
        constrain sensor capture bounds, followed by `videoconvert` and
        `format_caps` (`video/x-raw, format=I420`) feeding `stream_tee`.
    -   **Error Handling & Lifecycle:** Started when the first video or snapshot
        branch attaches (`AttachStreamBranch`), monitored via a bus sync handler
        for runtime device errors, and stopped automatically when the last
        active branch detaches (`DetachStreamBranch` /
        `StopCameraCapturePipeline`).

-   **Video Stream Branch (`CreateVideoBranchBin`):**

    -   **Scaling & Rate Control:** `queue` -> `videoscale` -> `videorate` ->
        `capsfilter` (`I420` at the allocated stream resolution and frame rate).
    -   **Encoding:** `x264enc` (`tune=zerolatency`, `speed-preset=ultrafast`)
        with privacy-zone pad probes.
    -   **Sink:** `appsink` (`sync=FALSE`, `async=FALSE`) invoking
        `OnNewVideoSampleFromAppSink` to forward encoded H.264 frames to
        `DefaultMediaController`.
    -   **Lifecycle:** Attached by `CameraDevice::StartVideoStream`, detached
        via an `IDLE` tee pad probe by `CameraDevice::StopVideoStream`.

-   **Audio Streaming (`CreateAudioPipeline`):**

    -   **Source:** `pulsesrc` (or `audiotestsrc` for testing).
    -   **Caps Negotiation:** `capsfilter` to set sample rate, channels.
    -   **Format Conversion:** `audioconvert` and `audioresample`.
    -   **Encoding:** `opusenc` for Opus encoding.
    -   **Sink:** `appsink` with the `OnNewAudioSampleFromAppSink` callback,
        which passes encoded Opus buffers to
        `DefaultMediaController::DistributeAudio`.
    -   **Lifecycle:** Started by `CameraDevice::StartAudioStream`, stopped by
        `CameraDevice::StopAudioStream`.

-   **Snapshot Stream Branch (`CreateSnapshotBranchBin`):**

    -   **Scaling & Rate Control:** `queue` -> `videoscale` -> `videorate` ->
        `capsfilter` (`I420` at the snapshot stream resolution, capped to a low
        continuous rate to minimize idle CPU).
    -   **Encoding:** `jpegenc` with privacy-zone pad probes.
    -   **Sink:** `appsink` (`max-buffers=1`, `drop=TRUE`, `sync=FALSE`,
        `async=FALSE`) from which `CameraDevice::CaptureSnapshot` pulls JPEG
        samples directly in memory.
    -   **Lifecycle:** Attached by `CameraDevice::StartSnapshotStream` (at
        allocation or on demand) and detached by
        `CameraDevice::StopSnapshotStream`.

-   **Audio Playback (`CreateAudioPlaybackPipeline`):**
    -   **Source:** `udpsrc` to receive RTP Opus packets from the network (e.g.,
        from a WebRTC session).
    -   **Jitter Buffer:** `rtpjitterbuffer` to handle network jitter.
    -   **Depacketization:** `rtpopusdepay` to extract Opus frames from RTP.
    -   **Decoding:** `opusdec` to decode Opus audio.
    -   **Output:** `audioconvert`, `audioresample`, and `autoaudiosink` to play
        the audio on the device speakers.
    -   **Lifecycle:** Started by `CameraDevice::StartAudioPlaybackStream`,
        stopped by `CameraDevice::StopAudioPlaybackStream`.

The state of these pipelines (e.g., NULL, READY, PLAYING) is managed using
`gst_element_set_state`. Callbacks on `appsink` elements are crucial for linking
GStreamer's data flow to the Matter application logic.
