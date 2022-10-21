# Matter Controller Java App Example

This is a Matter Controller Java app that can be used to commission and control
Matter accessory devices. It offers the following features: - Commission a
Matter device - Send echo requests to the Matter echo server - Send on/off
cluster requests to a Matter device

<hr>

-   [Matter Controller Java App Example](#matter-controller-java-app-example)
    -   [Requirements for building](#requirements-for-building)
    -   [Preparing for build](#preparing-for-build)
    -   [Building & Running the app](#building--running-the-app)

<hr>

<a name="requirements"></a>

## Requirements for building

You need Android SDK 21 & NDK 21.4.7075529 downloaded to your machine. Set the
`$ANDROID_HOME` environment variable to where the SDK is downloaded and the
`$ANDROID_NDK_HOME` environment variable to point to where the NDK package is
downloaded.

1. Install [Android Studio](https://developer.android.com/studio)
2. Install NDK:
    1. Tools -> SDK Manager -> SDK Tools Tab
    2. Click [x] Show Package Details
    3. Select NDK (Side by Side) -> 21.4.7075529
    4. Apply
3. Install Command Line Tools:
    1. Tools -> SDK Manager -> SDK Tools Tab -> Android SDK Command Line Tools
       (latest)
    2. Apply
4. Install SDK 21:
    1. Tools -> SDK Manager -> SDK Platforms Tab -> Android 5.0 (Lollipop) SDK
       Level 21
    2. Apply
5. Install Emulator:
    1. Tools -> Device Manager -> Create device -> Pixel 5 -> Android S API 31
       -> Download

### Linux

```
export ANDROID_HOME=~/Android/Sdk
export ANDROID_NDK_HOME=~/Android/Sdk/ndk/21.4.7075529
```

### MacOS

```
export ANDROID_HOME=~/Library/Android/sdk
export ANDROID_NDK_HOME=~/Library/Android/sdk/ndk/21.4.7075529
```

<hr>

<a name="preparing"></a>

## Preparing for build

Complete the following steps to prepare the Matter build:

1. Check out the Matter repository.

2. Run bootstrap (**only required first time**)

    ```shell
    source scripts/bootstrap.sh
    ```

<a name="building-running"></a>

## Building & Running the app

This is the simplest option. In the command line, run the following command from
the top Matter directory:

```shell
./scripts/build/build_examples.py --target android-x86-java-matter-controller build
```

The Java executable file `java-matter-controller` will be generated at
`out/android-x86-java-matter-controller/bin/`
