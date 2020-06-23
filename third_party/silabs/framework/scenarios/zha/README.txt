ZigBee Home Automation Sample Applications

These sample applications demonstrate basic ZigBee Home Automation
functionality between a light, a sleepy switch, and a gateway. The gateway
will form a network with the light & sleepy switch joined to it. The light
periodically reports the On/Off Cluster "on/off" status to the gateway.
The switch controls the light. All bindings, required for On/Off attribute
reporting & light/switch control via unicast, are created through Ez-Mode
Commissioning.

The gateway will perform network operations in reponse to press button 0 & 1.
Pressing button 0 will cause it to perform an energy scan in order to find an
available channel and form a network. Once network is formed, pressing button
0 will cause the light to permit joining on its network for 60 seconds.
Pressing button 1 will put the current device in Ez-Mode Server Commissioning
mode. This will allow the light to form a binding to the gateway for On/Off
Cluster attribute reporting.

The light will perform network operations in response to pressing button 0 & 1.
If the light is not joined to a network, pressing button 0 will cause it to
search for a joinable network. Once it has joined a network, pressing button
0 will cause the light to enter Ez-Mode Client Commissioning mode. This is
used to form binding from the light to the gateway as required by the
Reporting plugin. Pressing button 1 will put the device in Ez-Mode Server
Commissioning Mode. This is used to form binding from the switch to the light.

The switch will sleep until a button is pressed.  If the switch is not joined
to a network, pressing button 0 will cause the switch to wake up and search for a
joinable network.  If joined, pressing button 0 will cause the switch to wake up
and send an unicast on/off command to all existing bindings. To create
bindings, pressing button 1 will put the device in Ez-Mode client mode. If the
switch has slept long enough to have aged out of its parent's child table,
the switch will automatically attempt to rejoin the network prior to
broadcasting the on/off command. Once an action completes, the switch will
return to deep sleep.

All devices provide feedback during network operations by playing a tune. 
A rising two-tone tune indicates a successful operation while
a falling two-tone tune indicates a failure. A brief tone indicates that the
device has performed a long-running action and is waiting for a result.

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
