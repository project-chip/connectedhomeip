# CHIP Linux Camera Example App

An example application showing the use of a CHIP Camera on the Linux platform.
The document will describe how to build and run the CHIP Linux Camera Example on
Linux.

---

-   [Building the Example Application](#building-the-example-application)

---

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

### 2. Building

1. Initialize the Build Environment This command configures your shell with the
   necessary build tools. You only need to run this once per terminal session.

```
source scripts/activate.sh
```

2. Build the Camera App (Device)

You can either build the applications for a local Linux machine or cross-compile
them for a Raspberry Pi.

Option A: Build for a Local Linux (x86_64)

This is the simplest method for testing the camera pipeline on a single Linux
computer.

```
# Navigate to the examples directory
cd examples/camera-app/

# Compile the Linux x86‑64 camera‑app target
./scripts/build/build_examples.py \
    --target linux-x64-camera \
    build
```

The resulting executable is placed in:

```
out/linux-x64-camera/chip-camera-app.
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
    --target linux-arm64-camera-clang \
    build"
```

4. Transfer the Binary to the Raspberry Pi After the build completes, exit the
   Docker container. The compiled binary will be available on your host machine
   in the out/ directory. Use scp to copy it to your Raspberry Pi.

```
# The binary path on your host machine
# out/linux-arm64-camera-clang/chip-camera-app

# SCP command to transfer the file
scp ./out/linux-arm64-camera-clang/chip-camera-app ubuntu@<RASPBERRY_PI_IP_ADDRESS>:/home/ubuntu
```
