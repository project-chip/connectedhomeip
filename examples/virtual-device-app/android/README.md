# Matter Android Virtual Device App Example

This is a Matter Android Device app that can be used as a commissionee device.

This app offers the following features:

-   Work as a commissionee device on Android environment
-   Make custom payload information and display Matter QR code
-   Provide UI for the user to change the cluster attribute values and create
    the cluster event

<hr>

-   [Matter Android Virtual Device App Example](#matter-android-virtual-device-app-example)
    -   [Requirements for building](#requirements-for-building)
    -   [Preparing for build](#preparing-for-build)
    -   [Building & Installing the app](#building--installing-the-app)

<hr>

<a name="requirements"></a>

## Requirements for building

For information about how to build the application, see the
[Building Android](../../../docs/guides/android_building.md) guide.

## Preparing for build

Complete the following steps to prepare the Matter build:

1. Check out the Matter repository.

2. Run bootstrap (**only required first time**)

    ```shell
    source scripts/bootstrap.sh
    ```

<a name="building-installing"></a>

## Building & Installing the app

This is the simplest option. In the command line, run the following command from
the top Matter directory:

```shell
./scripts/build/build_examples.py --target android-arm64-virtual-device-app build
```

See the table above for other values of `TARGET_CPU`.

The debug Android package `app-debug.apk` will be generated at
`out/android-$TARGET_CPU-virtual-device-app/VirtualDeviceApp/app/outputs/apk/debug/`,
and can be installed with

```shell
adb install out/android-$TARGET_CPU-virtual-device-app/VirtualDeviceApp/app/outputs/apk/debug/app-debug.apk
```

You can use Android Studio to edit the Android app itself and run it after
build_examples.py, but you will not be able to edit Matter Android code from
`src/controller/java`, or other Matter C++ code within Android Studio.
