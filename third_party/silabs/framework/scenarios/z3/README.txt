ZigBee 3.0 Sample Applications

ZigBee 3.0 provides a foundation of commissioning and network management
mechanisms to be used in all ZigBee applications. The sample scenario
presented here demonstrates the flexibility that the ZigBee 3.0 specification
provides to applications. They also act as an excellent starting point for users
wishing to build their own ZigBee 3.0 applications.

These applications can take on three possible roles.
  1. The gateway can form a centralized network, and the light and the switch
     can join the centralized network by performing network steering.
  2. The light, acting as a router, can form a distributed network, and the
     switch, acting as an end device, can join the distributed network.
  3. The light, acting as a touchlink target, can touchlink with the
     switch, acting as a touchlink initiator.

The gateway provides a robust application interface to the user. Pressing
button 0 or 1 on the application when the gateway is not on a network will
form a centralized network. If the gateway interface does not have buttons
available, then the CLI command "plugin network-creator start 1" can be issued
in order to form a centralized network (just as the buttons would do). The
gateway application can then be triggered to allow other devices onto the
network using button 0. If buttons do not exist on the gateway, then one can
use the CLI command "plugin network-creator-security open-network" to do so.
Devices can then join the network using the ZigBeeAlliance09 link key, or by
manually entering the install code derived link key into the gateway using the
CLI command "plugin network-creator-security set-joining-link-key". Pressing
button 1 will no longer allow devices onto the gateway's network. If the host
does not have a button interface, then the CLI command
"plugin network-creator-security close-network" will close the network. When the
gateway has formed a network, it will set its COMMISSIONING_STATUS_LED. When the
network is open for joining, the LED will blink.

The light provides a very simplistic user interface. On power up, the light will
perform network steering automatically. If the light does not find a suitable
network to join, it forms its own distributed network. The light will not open
its network for joining as soon as the network comes up, so this must be done
manually. Once a distributed network is formed and opened, the switch may join
its network by performing network steering itself or through touchlink
commissioning.  Once the light is on a network, it will set its
COMMISSIONING_STATUS_LED. When the light starts identifying as a find and bind
target, it will blink its COMMISSIONING_STATUS_LED. The light will start
identifying as a find and bind target automatically when it brings its network
up, but users can reopen the find and bind target window by pushing either button
on the light at any time when it is on a network.

The switch provides a simple application interface to the user. When the switch
is not on a network, it can initiate network steering to join a network using
button 0. The switch can also initiate touchlink commissioning using button 1.
After the switch has successfully joining a network, it will perform the finding
and binding procedure for an initiator. After this takes place, the switch
should have at least two bindings to the light in its binding table: one for the
On/Off cluster and one for the Level Control cluster. While the switch is
performing its network commissioning, it will blink its
COMMISSIONING_STATUS_LED. When the switch is active on the network, it will
set its COMMISSIONING_STATUS_LED. Once the switch has finished finding and binding,
users can use buttons 0 and 1 to send On/Off Toggle and Level Control Move
to Level commands to the light, respectively.

The current debug printing settings in these applications are only for the
purpose of aiding users in understanding and debugging this sample scenario.
Debug printing should be turned off in order to save code size on a final
product.
