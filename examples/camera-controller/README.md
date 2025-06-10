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

## Building the Example Application

Building and Running the Camera Example (Linux)

1. Install build‑time and runtime dependencies

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

2. Build the camera‑controller binary

```
# Initialise the CHIP build environment
source scripts/activate.sh

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

3. Build the app binary

```
# Initialise the CHIP build environment
source scripts/activate.sh

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

4. Start the camera accessory Open a new terminal and run:

```
./out/linux-x64-camera-app/chip-camera-app --camera-deferred-offer
```

5. Launch the camera controller In a second terminal:

```
./out/linux-x64-camera-controller/chip-camera-controller
```

6. Commission the camera device At the controller prompt, pair over the local
   network using the default setup PIN 20202021 and an arbitrary nodeID 1:

```
pairing onnetwork 1 20202021
```

Wait until commissioning succeeds.

7. Start a live‑view stream Still in the controller shell, request a live view
   from the camera you just paired:

```
liveview start 1
```

Wave your hand in front of the camera to trigger live view; a video window will
appear, confirming that the stream is active.
