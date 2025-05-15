# Matter Joint Fabric Control Example

An example application that acts as a commissioner and control application for
jf-admin-app (Joint Fabric Administrator Application).

It is also used for initial commissioning of the jf-admin-app and also as a PKI
provider.

<hr>

-   [Matter Joint Fabric Control Example](#matter-joint-fabric-control-example)
    -   [Building the example Application on Linux](#building-the-example-application-on-linux)

<hr>

## Building the Example Application on Linux

-   Pull the required submodules

          $ scripts/checkout_submodules.py --shallow --platform linux

-   Build the example application:

          $ cd ~/connectedhomeip/examples/jf-control-app
          $ source third_party/connectedhomeip/scripts/activate.sh
          $ gn gen out/debug
          $ ninja -C out/debug

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip/examples/jf-control-app
          $ rm -rf out/
