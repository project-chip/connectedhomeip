# Matter Camera Controller Example

This example application demonstrates the CHIP Camera Controller running on a
Linux platform and explains how to build and run the Camera Controller Example
on Linux.

In a typical setup, the Camera Controller app manages a CameraDevice app running
on a Raspberry Pi. The CameraDevice captures and encodes the video feed before
streaming it through a WebRTC track, while the CameraController receives this
video stream and displays it, creating a complete end-to-end camera solution.

---

-   [Building the Example Application](#building-the-example-application)

---

## Overview

The example consists of two primary applications:

Camera App (chip-camera-app): This application functions as the Matter
accessory. It captures and encodes a video feed, making it available for
streaming over a WebRTC connection. It is typically run on a device with a
camera, such as a Raspberry Pi.

Camera Controller (chip-camera-controller): This application acts as the client.
It commissions the Camera App onto the Matter network, discovers it, and
receives and displays the video stream.

## Building the Example Application

### 1. Prerequisites

Before building, you must install the necessary GStreamer libraries and
development packages, which are used for video processing and streaming.

```
sudo apt update
sudo apt install \
  gstreamer1.0-plugins-base \
  gstreamer1.0-plugins-good \
  gstreamer1.0-plugins-bad \
  gstreamer1.0-libav \
  libgstreamer1.0-dev \
  libgstreamer-plugins-base1.0-dev
```

### 2. Building the Applications

1. Initialize the Build Environment This command configures your shell with the
   necessary build tools. You only need to run this once per terminal session.

```
source scripts/activate.sh
```

2. Build the Camera App (Device)

Follow the build instructions from examples/camera-app/linux/README.md

3. Build the Camera Controller (Client)

You can either build the applications for a local Linux machine or cross-compile
them for a Raspberry Pi.

Option A: Build for a Local Linux (x86_64)

This is the simplest method for testing the camera pipeline on a single Linux
computer.

```
# Navigate to the examples directory
cd examples/camera-controller/

# Compile the Linux x86‑64 camera‑controller target
./scripts/build/build_examples.py \
    --target linux-x64-camera-controller \
    build
```

The resulting executable is placed in:

```
out/linux-x64-camera-controller/chip-camera-controller.
```

Option B: Cross-Compile for Raspberry Pi (arm64)

To run an application on a Raspberry Pi, you must cross-compile it from an
x86_64 host machine. The recommended method is to use the provided Docker build
environment to ensure all dependencies are correct.

1. Pull the Cross-Compilation Docker Image

```
docker pull ghcr.io/project-chip/chip-build-crosscompile:162
```

2. Run the Docker Container This command starts an interactive shell inside the
   container and mounts your local connectedhomeip repository into the
   container's /var/connectedhomeip directory.

```
docker run -it -v ~/connectedhomeip:/var/connectedhomeip ghcr.io/project-chip/chip-build-crosscompile:162 /bin/bash
```

3. Build Inside the Container From within the Docker container's shell, execute
   the build script.

```
cd /var/connectedhomeip

# Required to fix git repository ownership issues inside Docker
git config --global --add safe.directory /var/connectedhomeip
git config --global --add safe.directory /var/connectedhomeip/third_party/pigweed/repo

# Run the build script for the Raspberry Pi (arm64) target
./scripts/run_in_build_env.sh \
  "./scripts/build/build_examples.py \
    --target linux-arm64-camera-controller-clang \
    build"
```

4. Transfer the Binary to the Raspberry Pi After the build completes, exit the
   Docker container. The compiled binary will be available on your host machine
   in the out/ directory. Use scp to copy it to your Raspberry Pi.

```
# The binary path on your host machine
# out/linux-arm64-camera-controller-clang/chip-camera-controller

# SCP command to transfer the file
scp ./out/linux-arm64-camera-controller-clang/chip-camera-controller ubuntu@<RASPBERRY_PI_IP_ADDRESS>:/home/ubuntu
```

### 3. Running the Live View Demo

After building the applications using Option A, you can run the full end-to-end
example on your Linux machine. You will need two separate terminal windows.

Terminal 1: Start the Camera App (Device)

1. Launch the chip-camera-app binary.

Clean up any existing configurations (first-time pairing only):

```
sudo rm -rf /tmp/chip_*
```

```
./out/linux-x64-camera/chip-camera-app
```

Terminal 2: Launch and Use the Camera Controller (Client)

1. Launch the Controller Start the interactive controller shell.

```
./out/linux-x64-camera-controller/chip-camera-controller
```

2. Commission the Device At the controller's > prompt, use the pairing command
   to securely commission the Camera App onto the Matter network.

onnetwork: Specifies pairing over the local IP network.

```
pairing onnetwork 1 20202021
```

Wait for the command to complete and confirm that commissioning was successful.

3. To start a live video stream from your camera, use the `liveview start`
   command followed by the nodeID you set during pairing. For example, if your
   nodeID is 1, you can request a stream with a minimum resolution of 800x600
   pixels and a minimum frame rate of 30 frames per second using the command
   below.

```
liveview start 1 --min-res-width 800 --min-res-height 600 --min-framerate 30
```

To see what video formats and resolutions your camera supports, first list the
available video devices, then check the formats for your specific device:

```
# List all available video devices
v4l2-ctl --list-devices

# Check formats for a specific device (replace /dev/video0 with your device)
v4l2-ctl -d /dev/video0 --list-formats-ext
```

Wave your hand in front of the camera to trigger live view; a video window will
appear, confirming that the stream is active.

### 4. Running the Video Recording Upload Demo

The Push AV Server acts as the recording destination (like a cloud service) for
video clips. The demo shows how a Matter Camera Controller tells a Matter Camera
to record a clip and upload it to that server.

The server itself doesn't control the camera; it's a passive service that
securely receives and stores media. The relationship is established by the
controller.

#### The Server's Role: The Recording Destination

The Push AV Server runs in the background, waiting for authenticated devices to
push video content to it. The key piece of information it provides is its ingest
URL, which, for example, looks like this: `https://localhost:1234/streams/1`

Think of this server as a secure, private YouTube or Dropbox, but just for your
camera's video clips.

Here is the command to set up a local Push AV Server, using a virtual
environment. (recommended)

```
# Create a virtual environment
python3 -m venv pavs_env

# Activate it
source pavs_env/bin/activate

# Install dependencies
pip install uvicorn fastapi jinja2 cryptography zeroconf pydantic

# Run the server
python server.py --working-directory ~/.pavstest
```

#### The Controller's Role: The Orchestrator

The `chip-camera-controller` acts as the "brain" of the operation. It's what you
use to configure the system. It tells the camera where to send its recordings.

This happens in this specific command:

-   Build & run camera application

```
./scripts/examples/gn_build_example.sh examples/camera-app/linux out/debug
./out/debug/chip-camera-app
```

-   Build & run `chip-camera-controller` steps for clip recording & uploading

```
./scripts/build/build_examples.py --target linux-x64-camera-controller build
./out/linux-x64-camera-controller/chip-camera-controller
```

-   Pair Camera

```
pairing code 1 34970112332
```

-   Video Stream Allocation

```
cameraavstreammanagement video-stream-allocate 3 0 30 30 '{ "width":640, "height":480}' '{ "width":640, "height":480}' 10000 10000 1 10 1 1 --WatermarkEnabled 1 --OSDEnabled 1
```

-   Audio Stream Allocation

```
cameraavstreammanagement audio-stream-allocate 3 0 2 48000 96000 16 1 1
```

-   Push AV Transport allocation

```
pushavstreamtransport allocate-push-transport '{"streamUsage":0, "videoStreamID":1, "audioStreamID":2, "endpointID":1, "url":"https://localhost:1234/streams/1", "triggerOptions":{"triggerType":0, "maxPreRollLen":1, "motionTimeControl":{"initialDuration":20, "augmentationDuration":5,"maxDuration":40, "blindDuration":5}}, "ingestMethod":0, "containerOptions":{"containerType":0, "CMAFContainerOptions": {"chunkDuration": 4, "CMAFInterface": 0, "segmentDuration": 6, "sessionGroup": 1, "trackName": "main"}}}' 1 1
```

Here, the controller is sending the server's URL to the camera. It's essentially
saying, "When I tell you to record a clip, this is the address you need to send
it to."

#### The Camera's Role: The Content Producer

The `chip-camera-app` is the device that does the actual work of recording and
sending the video. After it receives the configuration from the controller, it
waits for a trigger.

This command from the controller is the trigger:

```
pushavstreamtransport manually-trigger-transport 1 0 1 1
```

This command means, "Start recording now and upload the clip to the URL I gave
you earlier." The camera then connects to the push_av_server at
`https://localhost:1234/streams/1` and pushes the video clip.
