# Matter Controller Java App Example

This is a Matter Controller Java app that can be used to uses Matter to send
messages to a Matter server.

<hr>

-   [Matter Controller Java App Example](#matter-controller-java-app-example)
    -   [Requirements for building](#requirements-for-building)
        -   [Gradle & JDK Version](#gradle--jdk-version)
    -   [Preparing for build](#preparing-for-build)
    -   [Building & Running the app](#building--running-the-app)

<hr>

<a name="requirements"></a>

## Requirements for building

You need Android SDK 21 & NDK downloaded to your machine. Set the
`$ANDROID_HOME` environment variable to where the SDK is downloaded and the
`$ANDROID_NDK_HOME` environment variable to point to where the NDK package is
downloaded.

<a name="jdk"></a>

### Gradle & JDK Version

We are using Gradle 7.1.1 for all android project which does not support Java 17
(https://docs.gradle.org/current/userguide/compatibility.html) while the default
JDK version on MacOS for Apple Silicon is 'openjdk 17.0.1' or above.

Using JDK bundled with Android Studio will help with that.

```shell
export JAVA_HOME=/Applications/Android\ Studio.app/Contents/jre/Contents/Home/
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
