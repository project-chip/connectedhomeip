# CHIP Zigbee Cluster Library (ZCL) Documentation

## What is this?

Documentation for the CHIP Zigbee Cluster Library (ZCL) implementation also
called the CHIP ZCL Application Framework. This document provides information on
how the different parts of the CHIP Application layer implementation of the
Zigbee Cluster Library fit together.

**_<code>THIS DOCUMENT IS A WORK IN PROGRESS AS THE DESIGN OF THE CHIP
APPLICATION FRAMEWORK IS STILL IN FLUX</code>_**

## Directory Structure

### <code>/src/app/api</code>

This directory provides the external interface for the CHIP ZCL Application
Framework. In here you will find APIs and declarations needed to interact with
the CHIP ZCL implementation.

### <code>/src/app/docs</code>

This directory contains all the documentation for the CHIP ZCL Application
Framework including the file you are reading right now.

### <code>/src/app/gen</code>

This directory contains an example of generated code used by the CHIP ZCL
Application Framework. This is not all the generated code used by a given
application. It is merely the code used by the test framework within the CHIP
ZCL Application Framework. Under orderinary circumstances, code would be
generated into the <code>gen</code> directory associated with a given
application target or project. The application target's <code>gen</code>
directory would live with the given application and not with the core CHIP ZCL
Application Framework. In this way there can be many application configurations
each with its own <code>gen</code> directory.

### <code>/src/app/plugin</code>

This directory contains all of the plugins or sets of functionality included in
the CHIP ZCL Application Framework. For more information on the plugin directory
see the design document below.

## Design

The CHIP ZCL implementation is broken down into "plugins." Plugins are located
in the plugin directory located at <code>src/app/plugin</code>. A plugin
represents a discrete piece of functionality implemented within the CHIP ZCL.
Everything in the CHIP ZCL Application Framework is encapsulated in a plugin.

### Handlers and Callbacks

There are two types of functions called from within the CHIP ZCL Application
Framework, there are command "handlers" and "callbacks". These two types of
functions are distinguished by the last word in their function signature.

#### Command Handlers

The command handler literally handles ZCL commands as they arrive over the air.
the command handler has a very specific signature containing just two arguments,
a pointer to the incoming <code>ChipZclCommandContext_t</code> and a pointer to
the handlers associated command struct. The <code>ChipZclCommandContext_t</code>
contains all of the raw information about the incoming command such as its
<code>endpointId</code>, <code>clusterId</code> and <code>commandId</code> among
other things. The command handler's command struct contains all of the arguments
associated with a given command.

Command handler can be implemented anywhere in the application code, but
generally they are implemented by the plugin associated with the ZCL Cluster to
which the given command belongs. For example the
<code>chipZclClusterOnOffServerCommandOnRequestHandler</code> is implemented by
the <code>cluster-server-on-off</code> plugin. This way whenever an "on" command
is received over the air by the device, it will be handed off to the
<code>cluster-server-on-off</code> plugin which will in turn decide what to do
with it.

#### Callbacks

Callbacks are called when one portion of the CHIP ZCL Application Framework
needs to interact with another. They are used to tie different pieces of
functionality within the CHIP ZCL Application Framework together. For instance
the <code>core-data-model</code> calls the
<code>postAttributeChangeCallback</code> when an attribute's value changes
within the data model. By implementing the
<code>postAttributeChangeCallback</code> the application is able to listen for,
and react to changes in the <code>core-data-model</code>.

Any plugin may define a callback for its own purposes. The
<code>core-data-model</code> defines callbacks associated with the data model.
The <code>cluster-server-identify</code> plugin defines two callbacks, the
<code>chipZclIdentifyServerStartIdentifyingCallback</code> and the
<code>chipZclIdentifyServerStopIdentifyingCallback</code>. By implementing these
callbacks, the application can be informed when it should start or stop
identifying itself. In this manner, the "normative behavior" of the application
can be encapsulated inside the <code>cluster-server-identify</code> plugin and
the application logic implemented by the developer need only implement a
callback so that it knows when to start and stop interaction with the hardware
needed to identify itself (toggling a gpio).

Callbacks provide a nice separation between the application logic (interaction
with the hardware) and normative logic encapsulated within the CHIP ZCL
Applicaton Framework.

## Plugins

I do not attempt to document all of the plugins in the CHIP ZCL Application
Framework here, this would be a never ending tasks since they are being added to
all the time. This is merely documentation for the critical plugins associated
with the framework such as <code>core-message-dispatch</code> and
<code>core-data-model</code>.

### <code>core-message-dispatch</code>

Messages enter into and are handled by the CHIP ZCL Application Framework
through the <code>core-message-dispatch</code> plugin. The
<code>core-message-dispatch</code> plugin is responsible for handling incoming
messages and dispatching them off for processing by various other plugins.
<code>core-message-dispatch</code> is also responsible for transmitting any
synchronous response to an incoming message if one is required. For instance if
a message arrives and it requires a ZCL Default Response, the
<code>core-message-dispatch</code> plugin is responsible for generating any
response value <code>ZCL_ERROR</code> or <code>ZCL_SUCCESS</code> based on how
the incoming message is handled by the application framework and putting the
response into the CHIP outgoing message queue.

Messages arriving in the <code>core-message-dispatch</code> are assumed to have
had their "APS" Header decoded completely such that they arrive in the form of a
pointer to a <code>ChipZclCommandContext_t</code> which is defined in
<code>src/app/api/chip-zcl.h</code>. Values that are normally part of the Zigbee
APS header such as <code>endpointId</code>, <code>clusterId</code> and
<code>commandId</code> are used by the <code>core-message-dispatch</code> to
determine how to process the incoming message.

<code>core-message-dispatch</code> uses information parsed from the "APS" header
to retrieve a "request spec" and command handler pointer from the
<code>core-data-model</code>. The "request spec" contains all the information
necessary to decode the incoming request. The "request spec" and a pointer to
the appropriate "request struct" are passed by the
<code>core-message-dispatch</code> to the appropriate codec. The codec decodes
the incoming message payload and populates the associated "request struct" which
can then be passed on throughout the system for actual command processing and
device actuation etc...

For more information and an example on the handling of incoming On/Off Command
messages by the <code>core-message-dispatch</code> and other plugins please see
the diagram below:

![CHIP ZCL Message Flow](chip-zcl-msg-flow.png)

### <code>core-data-model</code>

The <code>core-data-model</code> plugin is responsible for the storage and
interaction with the data model associated with a device. This includes all ZCL
attributes and command handlers. For instance, when an attribute needs to be
updated, the application interacts with the <code>core-data-model</code>. The
data model itself is a generated piece of code located in the <code>gen</code>
directory associated with a given project. There is a <code>gen</code> directory
located at <code>/src/app/gen</code>. This is not the gen directory for an
application. It is merely an example of generated code used for unit testing the
CHIP ZCL Application Framework.

When a ZCL attribute is updated in the data model, the
<code>core-data-model</code> will call the
<code>postAttributeChangeCallback</code>, if this callback is implemented by the
device it will be informed of the attribute change. The device may react to the
attribute change. For instance, if the on/off attribute in the On-Off Cluster
changes, the application which implements the
<code>postAttributeChangeCallback</code> may reflect this by toggling the actual
pin tied to an LED.
