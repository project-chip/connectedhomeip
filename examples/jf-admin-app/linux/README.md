# Matter Joint Fabric Admin Example

An example application that acts as a Joint Fabric Administrator.

<hr>

-   [Matter Joint Fabric Admin Example](#matter-joint-fabric-admin-example)
    -   [Building the example Application on Linux](#building-the-example-application-on-linux)

<hr>

## Building the Example Application on Linux

-   Pull the required submodules

          $ scripts/checkout_submodules.py --shallow --platform linux

-   Build the example application:

          $ cd ~/connectedhomeip/examples/jf-admin-app/linux
          $ source ../third_party/connectedhomeip/scripts/activate.sh
          $ gn gen out/debug
          $ ninja -C out/debug

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip/examples/jf-admin-app/linux
          $ rm -rf out/
