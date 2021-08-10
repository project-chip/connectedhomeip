# CHIP TV Casting App Example

This is a CHIP TV Casting app that can be used to cast content to a TV. This app
discovers TVs on the local network that act as commissioners, lets the user
select one, sends the TV a User Directed Commissioning request, enters
commisioning mode, advertises itself as a Commissionable Node and gets
commissioned. Then it allows the user to send CHIP ContentLauncher commands to
the TV.

<hr>

-   [CHIP TV Casting App Example](#chip-tv-casting-app-example)
    -   [Building](#building)
    -   [Running the Complete Example on Linux](#running-the-complete-example-on-linux)

<hr>

<a name="building"></a>

## Building

-   Install tool chain

          $ sudo apt-get install git gcc g++ python pkg-config libssl-dev libdbus-1-dev libglib2.0-dev ninja-build python3-venv python3-dev unzip

-   Build the example application:

          $ cd ~/connectedhomeip/examples/tv-casting-app/linux
          $ git submodule update --init
          $ source third_party/connectedhomeip/scripts/activate.sh
          $ gn gen out/debug
          $ ninja -C out/debug

-   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip/examples/tv-casting-app/linux
          $ rm -rf out/

<a name="running-the-complete-example-on-linux"></a>

## Running the Complete Example on Linux

-   Pre-requisite: Build and run the tv-app

        $ cd ~/connectedhomeip/examples/tv-app/linux
        $ out/debug/chip-tv-app

-   Run the tv-casting-app

        $ cd ~/connectedhomeip/examples/tv-casting-app/linux
        $ out/debug/chip-tv-casting-app

    Follow the on-screen prompts on the tv-casting-app console
