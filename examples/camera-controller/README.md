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
docker pull ghcr.io/project-chip/chip-build-crosscompile:140
```

2. Run the Docker Container This command starts an interactive shell inside the
   container and mounts your local connectedhomeip repository into the
   container's /var/connectedhomeip directory.

```
docker run -it -v ~/connectedhomeip:/var/connectedhomeip ghcr.io/project-chip/chip-build-crosscompile:140 /bin/bash
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

### 3. Running the Local Demonstration

After building the applications using Option A, you can run the full end-to-end
example on your Linux machine. You will need two separate terminal windows.

Terminal 1: Start the Camera App (Device)

1. Launch the chip-camera-app binary. The --camera-deferred-offer flag prepares
   the camera to stream upon request from the controller.

Clean up any existing configurations (first-time pairing only):

```
sudo rm -rf /tmp/chip_*
```

```
./out/linux-x64-camera/chip-camera-app --camera-deferred-offer
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

3. Start the Live Stream Still in the controller shell, use the Live View
   command with the nodeID you assigned during pairing to request a video
   stream.

```
liveview start 1
```

Wave your hand in front of the camera to trigger live view; a video window will
appear, confirming that the stream is active.
