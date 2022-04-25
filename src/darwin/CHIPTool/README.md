# CHIP Tool iOS Sample Commissioner App

A prototype application that demonstrates device commissioning and cluster
control.

---

-   [CHIP Tool iOS Sample Commissioner App](#chip-tool-ios-sample-commissioner-app)
    -   [Building the Application](#building-the-application)
        -   [Compilation Fixes](#compilation-fixes)
    -   [Installing the Application](#installing-the-application)
    -   [Pairing an Accessory](#pairing-an-accessory)

---

## Prerequisites

-   Requires building and running the app to a physical device on iOS >= 15.4.

## Building the Application

CHIPTool iOS can be built with the latest Xcode releases.

-   Open the `Darwin.xcworkspace` file located in `src/darwin` with Xcode.

-   Then select the `CHIP Tool App` scheme at the top.

-   Running the `CHIP Tool App` scheme in Xcode (select the scheme and then hit
    the "play button" to "run" the scheme) will attempt to compile the
    application and then install it to the connected iOS device selected in the
    scheme.

Because we do not share a developer group, directly running this scheme will
fail with missing signing configuration errors.

You need to update the Project configuration for CHIPTool to use your Personal
Developer account and a unique Bundle ID.

### Compilation Fixes

Before you can run the `CHIP Tool App` scheme, you need to amend the
`project.pbxproj` file for the CHIPTool project. The file is located here
`src/darwin/CHIPTool/CHIPTool.xcodeproj/project.pbxproj`.

You can choose to amend this file via Xcode directly or manually with any text
editor. This Readme will focus on doing it via Xcode.

-   With the `Darwin.xcworkspace` loaded, you should see 2 projects on the left
    side pane in Xcode. `CHIPTool` and `CHIP`.

-   Selecting `CHIPTool` should bring up the project configuration view in
    Xcode. Next, select the `Signing & Capabilities` tab.

-   Perform the following steps to enable building CHIPTool:

    1. Ensure "Automatically manage signing" is checked

    2. Select your Personal Team in the "Team" dropdown.

    3. Change the bundle identifier from `com.chip.CHIPTool` to something
       unique, like `com.chip.CHIPTool-username`. These bundle IDs get reserved
       for a short amount of time and so it's best to use something only you
       might think of to avoid conflicts (you'll see that the default bundle ID
       does not work if you skip this step).

    4. Confirm that the Signing Certificate now says "Apple Development:
       <your personal account>"

Now you can install CHIPTool to your connected iOS device by clicking on the
"Play"/Run icon.

## Installing the Application

The first time you install this application to your iOS device, Xcode will not
be able to launch it. This is because iOS prevents arbitrary developer apps from
running prior to user consent. To give this application consent, navigate to
`Settings->General->VPN & Device Management` and give CHIPTool permission to
run.

Now you can launch the application from the Home screen or from Xcode by hitting
the run button once more.

## Pairing an Accessory

Once you have CHIPTool up and running, to pair an accessory simply:

-   Click on the `QRCode Scanner` item in the list on screen
-   Then the Camera icon on the top right corner to launch the scanner
-   Then scan the QRCode of the accessory to begin commissioning it. CHIPTool
    will prompt you for permission to use Bluetooth as well as to scan your
    local network. Grant these requests. It will also prompt you for the WiFi
    password for network the accessory should join. Note - Thread is not
    supported by this application at this time.

Look for `Commissioning complete.` and `Sigma3` in the logs to know when the
device is commissioned and ready for use.

Note - CHIPTool also supports pairing via the Matter manual codes. Instead of
scanning the QRCode you can also type in the manual code. The rest of the steps
remain the same.
