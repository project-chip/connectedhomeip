# Programming with UniFlash

-   Download and install [UniFlash][uniflash].

-   Select the Device Detect option. Then select the device you wish to program.
    Or you can create a new configuration if you know the debugger serial number

    ![UniFlash step 1](images/uniflash-1.jpg)

-   Browse for your build image by clicking the `Browse` button. It will be at
    `${connectedhomeip}/examples/lock-app/cc13x2_26x2/out/debug/chip-cc13x2_26x2-lock-example.out`

    ![UniFlash step 2](images/uniflash-2.jpg)

-   Load the image by clicking the `Load Image` button. This will eventually
    result in a success message in the console below.

    ![UniFlash step 3](images/uniflash-3.jpg)

-   Open a terminal to the XDS110 User UART and interact with the example
    application.

[uniflash]: https://www.ti.com/tool/download/UNIFLASH
