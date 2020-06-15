DynamicMultiprotocolDemoSwitch:

The Dynamic Multiprotocol Demo Switch application help demonstrate the dynamic 
multiprotocol feature from Silabs. This application only uses zigbee and is not
a dynamic multiprotocol application itself. This application is to be used with
the Dynamic Multiprotocol Demo Light sample to demonstrate the control of a 
dynamic multiprotocol device from a zigbee network.

The Demo is setup to run on the WSTK6000B Mesh Development Kit. The Demo makes
use of the LCD display, LEDs and Buttons on the WSTK. 

The Switch is configured to join a zigbee network as a router and once joined it
find and bind to an on/off server cluster such that after joining button presses 
on the WSTK will send the on/off cluster's Toggle command to the bound device.
The Switch will also setup reporting on the bound device to report any change in 
state of the on/off cluster. The state of the on/off attribute is then shown on 
LCD display by the bulb icon.

Upon startup a help screen is shown on the LCD display to assist the user to 
join and leave network and to control the Light. 

All application code is contained in the _callbacks.c files within each
application directory.

To use each application:

   1. Load the included application configuration file (i.e., the .isc file)
      into Simplicity Studio.

   2. Enter a new name for the application in the pop-up window.

   3. Generate the application's header and project/workspace files by
      clicking on the "Generate" button in Simplicity Studio.  The application
      files will be generated in the app/builder/<name> directory in the stack
      installation location.

   4. Load the generated project file into the appropriate compiler and build
      the project.

   5. Load the binary image onto a device using Simplicity Studio.


DynamicMultiprotocolDemoLight :
The Dynamic Multiprotocol Demo Light application demonstrate the dynamic 
multiprotocol feature from Silabs. In particular, these applications 
demonstrate the ZigBee and BLE stacks running concurrently as MicriumOS tasks.

The Demo is setup to run on the WSTK6000B Mesh Development Kit. The Demo makes
use of the LCD display, LEDs and Buttons on the WSTK.

The Light is configured to host a zigbee network as a coordinator and allow 
other devices to join. A help screen is shown on the LCD display to assist 
the user to form, permit join and control the Light. A complementary 
application named DynamicMultiprotocolDemoSwitchSoc is available to run on a 
WSTK board and join the network formed by the Light and control the state of 
Light via buttons on the WSTK running the Switch application.

On the BLE interface the Light is setup to advertise itself using Eddystone and
iBeacon in an alternate manner. The Light advertises itself as available for a 
connection by a smartphone running Wireless Gecko application. Once connected
the user can control the Light (represented by LEDs on the WSTK board) using 
Wireless Gecko smartphone application. 

A change in the Light state initiated by the either the smartphone over the BLE
connection or via the zigbee only Switch is propagated to connected device over
both the zigbee and BLE interface.

All application code is contained in the _callbacks.c files within each
application directory.

To use each application:

   1. Load the included application configuration file (i.e., the .isc file)
      into Simplicity Studio.

   2. Enter a new name for the application in the pop-up window.

   3. Generate the application's header and project/workspace files by
      clicking on the "Generate" button in Simplicity Studio.  The application
      files will be generated in the app/builder/<name> directory in the stack
      installation location.

   4. Load the generated project file into the appropriate compiler and build
      the project.

   5. Load the binary image onto a device using Simplicity Studio.

DynamicMultiprotocolDemoLightSED :
The Dynamic Multiprotocol Demo Light (SED) application demonstrates the dynamic 
multiprotocol feature from Silabs on a sleepy device. In particular, these 
applications demonstrate the ZigBee and BLE stacks running concurrently as
MicriumOS tasks.

The Demo is setup to run on the WSTK6000B Mesh Development Kit. The Demo makes
use of the LCD display, LEDs and Buttons on the WSTK.

The sleepy Light is configured to join a zigbee network as a SED. Once on the 
network it will start to identify itself. A help screen is shown on the LCD display 
to assist the user to join and control the Light. A complementary application named 
DynamicMultiprotocolDemoSwitchSoc is available to run on a WSTK board and join the 
network formed by the Light and control the state of Light via buttons on the WSTK 
running the Switch application.

On the BLE interface the Light is setup to advertise itself using Eddystone and
iBeacon in an alternate manner. The Light advertises itself as available for a 
connection by a smartphone running Wireless Gecko application. Once connected
the user can control the Light (represented by LEDs on the WSTK board) using 
Wireless Gecko smartphone application. 

A change in the Light state initiated by the either the smartphone over the BLE
connection or via the zigbee only Switch is propagated to connected device over
both the zigbee and BLE interface.

All application code is contained in the _callbacks.c files within each
application directory.

To use each application:

   1. Load the included application configuration file (i.e., the .isc file)
      into Simplicity Studio.

   2. Enter a new name for the application in the pop-up window.

   3. Generate the application's header and project/workspace files by
      clicking on the "Generate" button in Simplicity Studio.  The application
      files will be generated in the app/builder/<name> directory in the stack
      installation location.

   4. Load the generated project file into the appropriate compiler and build
      the project.

   5. Load the binary image onto a device using Simplicity Studio.

