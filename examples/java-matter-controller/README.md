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

## Requirements for building

You need to have the following two software installed on your Ubuntu system:

1. Java Runtime Environment (JRE)
2. Java Development Kit (JDK)

```
java -version
```

This will ensure either Java Runtime Environment is already installed on your
system or not. In order to install the Java Runtime Environment on your system,
run the following command as root:

```
sudo apt install default-jre Install Java default JRE
```

After installing the JRE, let us check if we have the Java Development Kit
installed on our system or not.

```
javac -version
```

The above output shows that I need to install the Java compiler or the JDK on my
system. You can install it through the following command as root:

```
sudo apt install default-jdk
```

### Linux

```
export JAVA_PATH=[JDK path]
```

<hr>

## Preparing for build

Complete the following steps to prepare the Matter build:

1. Check out the Matter repository.

2. Run bootstrap (**only required first time**)

    ```shell
    source scripts/bootstrap.sh
    ```

## Building & Running the app

This is the simplest option. In the command line, run the following command from
the top Matter directory:

```shell
./scripts/build/build_examples.py --target linux-x64-java-matter-controller build
```

The Java executable file `java-matter-controller` will be generated at
`out/android-x86-java-matter-controller/bin/`

Run the java-matter-controller

```
java -Djava.library.path=../lib/jni -jar java-matter-controller
```
