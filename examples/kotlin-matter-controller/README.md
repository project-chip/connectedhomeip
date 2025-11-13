# Matter Controller Kotlin App Example

This is a Matter Controller Kotlin app that can be used to commission and
control Matter accessory devices.

<hr>

-   [Matter Controller Kotlin App Example](#matter-controller-kotlin-app-example)
    -   [Requirements for building](#requirements-for-building)
        -   [Linux](#linux)
    -   [Preparing for build](#preparing-for-build)
    -   [Building \& Running the app](#building--running-the-app)

<hr>

## Requirements for building

You need to have the following two software installed on your Ubuntu system:

1. Java Runtime Environment (JRE)
2. Java Development Kit (JDK)

openjdk version "11.0.20.1" or above is needed

First, check if we have the Java Runtime Environment and Java Development Kit
installed on our system or not.

```shell
java -version
```

If not, you can install the Java Runtime Environment and Java Development Kit on
your system through the following command as root:

```
sudo apt install default-jre
sudo apt install default-jdk
```

You also need to install kotlin compiler on your Ubuntu system:

kotlin compiler version 1.8.10 or above is needed to compile
kotlin-matter-controller, if you already have lower version kotlin compiler
installed on your Ubuntu from apt,  
you need to remove the Kotlin compiler package, run the following command:

```shell
sudo apt-get remove kotlin
```

Wait for the removal process to complete. Once it's done, the Kotlin compiler
will be removed from your system.

(Optional) If you want to remove any configuration files associated with Kotlin,
run the following command:

```shell
sudo apt-get purge kotlin
```

Install kotlin compiler 1.8.10 or above, such as
[kotlin-compiler-1.8.10-url](https://github.com/JetBrains/kotlin/releases/download/v1.8.10/kotlin-compiler-1.8.10.zip)

```shell
cd /usr/lib \
&& sudo wget -q [kotlin-compiler-1.8.10-url] \
&& sudo unzip kotlin-compiler-*.zip \
&& sudo rm kotlin-compiler-*.zip \
&& sudo rm -f kotlinc/bin/*.bat
```

Add a directory to PATH permanently by editing the `.bashrc` file located in the
Home directory. Follow these steps:

1. Open the `.bashrc` file using a text editor.
2. Go to the end of the file.
3. Paste the export syntax at the end of the file.

```shell
export PATH="/usr/lib/kotlinc/bin:$PATH"
```

### Linux

```shell
export JAVA_HOME=[JDK path]
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
./scripts/build/build_examples.py --target linux-x64-kotlin-matter-controller build
```

The Java executable file `kotlin-matter-controller` will be generated at
`out/linux-x64-kotlin-matter-controller/bin/`

Run the kotlin-matter-controller

```shell
java -Djava.library.path=../lib/jni -jar kotlin-matter-controller
```
