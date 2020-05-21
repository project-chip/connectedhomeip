# Application layer for CHIP Documentation

## What is this?

Documentation for the CHIP Application layer. This docuemnt provides information regarding how the different parts of the CHIP Application layer implementation of the Zigbee Cluster Library fits together.

## Design

The CHIP ZCL implementation is broken down into "plugins." Plugins are located in the plugin directory located at src/app/plugin. A plugin represents a discrete piece of functionality implemented within the CHIP ZCL.

Messages enter into and are handled by the CHIP ZCL through the core-message-dispatch plugin. The core-message-dispatch plugin is responsible for handling incoming messages and dispatching them off for processing by various other plugins. For more information on the handling of plugins please see the diagram below:


![CHIP ZCL Message Flow](chip-zcl-msg-flow.png)