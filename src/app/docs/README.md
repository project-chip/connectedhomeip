# CHIP Zigbee Cluster Library (ZCL) Documentation

## What is this?

Documentation for the CHIP Zigbee Cluster Library (ZCL) implementation. This document provides information regarding how the different parts of the CHIP Application layer implementation of the Zigbee Cluster Library fit together.

## Design

The CHIP ZCL implementation is broken down into "plugins." Plugins are located in the plugin directory located at <code>src/app/plugin</code>. A plugin represents a discrete piece of functionality implemented within the CHIP ZCL. The collection of all of the CHIP ZCL plugins is sometimes called the *CHIP ZCL Application Framework*. 

### Core Message Dispatch
Messages enter into and are handled by the CHIP ZCL Application Framework through the <code>core-message-dispatch</code> plugin. The <code>core-message-dispatch</code> plugin is responsible for handling incoming messages and dispatching them off for processing by various other plugins. <code>core-message-dispatch</code> is also responsible for transmitting any synchronous response to an incoming message if one is required. For instance if a message arrives and it requires a ZCL Default Response, the <code>core-message-dispatch</code> plugin is responsible for generating any response value <code>ZCL_ERROR</code> or <code>ZCL_SUCCESS</code> based on how the incoming message is handled by the application framework and putting the response into the CHIP outgoing message queue.

Messages arriving in the <code>core-message-dispatch</code> are assumed to have had their "APS" Header decoded completely such that they arrive in the form of a pointer to a <code>ChipZclCommandContext_t</code> which is defined in <code>src/app/api/chip-zcl.h</code>. Values that are normally part of the Zigbee APS header such as <code>endpointId</code>, <code>clusterId</code> and <code>commandId</code> are used by the <code>core-message-dispatch</code> to determine how to process the incoming message.

For more information on the handling of plugins please see the diagram below:


![CHIP ZCL Message Flow](chip-zcl-msg-flow.png)