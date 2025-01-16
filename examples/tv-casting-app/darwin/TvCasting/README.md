# Matter TV Casting iOS App Example

This is a Matter TV Casting iOS app that can be used to cast content to a TV.
This app discovers TVs on the local network that act as commissioners, lets the
user select one, sends the TV a User Directed Commissioning request, enters
commissioning mode, advertises itself as a Commissionable Node and gets
commissioned. Then it allows the user to send Matter ContentLauncher commands to
the TV.

Refer to the
[Matter Casting APIs documentation](https://project-chip.github.io/connectedhomeip-doc/examples/tv-casting-app/APIs.html)
to build the Matter “Casting Client” into your consumer-facing mobile app.

---

-   [Matter TV Casting iOS App Example](#matter-tv-casting-ios-app-example)
    -   [Building the Application](#building-the-application)
        -   [Building through command line](#building-through-command-line)
        -   [Compilation Fixes](#compilation-fixes)
    -   [Installing the Application](#installing-the-application)
    -   [Debugging](#debugging)
    -   [Running the Application](#running-the-application)

---

## Building the Application

Matter TV Casting iOS App Example can be built with the latest Xcode releases.

-   Open the `TvCastingDarwin.xcworkspace` file located in
    `examples/tv-casting-app/darwin` with Xcode.

-   Then select the `TvCasting` scheme at the top.

-   Running the `TvCasting` scheme in Xcode (select the scheme and then hit the
    "play button" to "run" the scheme) will attempt to compile the application
    and then install it to the connected iOS device selected in the scheme.

Because we do not share a developer group, directly running this scheme will
fail with missing signing configuration errors.

You need to update the Project configuration for TvCasting app to use your
Personal Developer account and a unique Bundle ID.

### Building through command line

To build the app through the command line, use the xcodebuild app.

```shell
xcodebuild -workspace TvCastingDarwin.xcworkspace -scheme TvCasting -sdk <SDK_TARGET>
```

The list of available SDKs can be found by using the showsdks flag.

```shell
xcodebuild -showsdks
```

### Compilation Fixes

Before you can run the `TvCasting` scheme, you need to amend the
`project.pbxproj` file for the TvCasting app project. The file is located here
`examples/tv-casting-app/darwin/TvCasting/TvCasting.xcodeproj/project.pbxproj`.

You can choose to amend this file via Xcode directly or manually with any text
editor. This Readme will focus on doing it via Xcode.

-   With the `TvCastingDarwin.xcworkspace` loaded, you should see 1 project on
    the left side pane in Xcode. `TvCasting`

-   Selecting `TvCasting` should bring up the project configuration view in
    Xcode. Next, select the `Signing & Capabilities` tab.

-   Perform the following steps to enable building TvCasting app:

    1. Ensure "Automatically manage signing" is checked

    2. Select your Personal Team in the "Team" dropdown.

    3. Change the bundle identifier from `com.matter.TvCasting` to something
       unique, like `com.matter.TvCasting-username`. These bundle IDs get
       reserved for a short amount of time and so it's best to use something
       only you might think of to avoid conflicts (you'll see that the default
       bundle ID does not work if you skip this step).

    4. Confirm that the Signing Certificate now says "Apple Development:
       <your personal account>"

Now you can install TvCasting app to your connected iOS device by clicking on
the "Play"/Run icon.

## Installing the Application

The first time you install this application to your iOS device, Xcode will not
be able to launch it. This is because iOS prevents arbitrary developer apps from
running prior to user consent. To give this application consent, navigate to
`Settings->General->VPN & Device Management` and give TvCasting app permission
to run.

Now you can launch the application from the Home screen or from Xcode by hitting
the run button once more.

## Debugging

Use the "TvCasting" scheme when building to enable debugging. If you wish to
build the app without any debugging symbols, use the "TvCasting Release" scheme.

## Running the Application

This example Matter TV Casting iOS application can be tested with the following
video players:

1. With the
   [example Matter tv-app](https://github.com/project-chip/connectedhomeip/tree/master/examples/tv-app)
   running on a Raspberry Pi - works out of the box.
2. With a FireTV device - requires your Amazon Customer ID to be allow-listed
   first.
