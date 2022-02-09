# Android TV App

This directory contains the Android Studio project for CHIPTVServer, an Android
application for commissioning and controlling CHIP accessories.

CHIPTVServer offers the following features:

-   Scan a CHIP QR code and display payload information to the user
-   Read the NFC tag containing CHIP onboarding information
-   Commission a CHIP device
-   Send echo requests to the CHIP echo server
-   Send on/off cluster requests to a CHIP device

> :warning: Connection to the CHIP device's soft AP will be a manual step until
> pairing is implemented.

For information about how to build the application, see the
[Building Android CHIPTVServer](../../../docs/guides/android_building.md) guide.


## Building 

./scripts/examples/gn_android_example.sh examples/tv-app/android out/debug is_debug=true

