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

For Linux host example:

```
source scripts/activate.sh
./scripts/build/build_examples.py --target linux-x64-camera-controller build
```
