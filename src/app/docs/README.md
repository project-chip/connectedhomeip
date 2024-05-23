# CHIP Zigbee Cluster Library (ZCL) Documentation

## What is this?

Documentation for the CHIP Zigbee Cluster Library (ZCL) implementation also
called the CHIP ZCL Application Framework. This document provides information on
how the different parts of the CHIP Application layer implementation of the
Zigbee Cluster Library fit together.

**_<code>THIS DOCUMENT IS A WORK IN PROGRESS AS THE DESIGN OF THE CHIP
APPLICATION FRAMEWORK IS STILL IN FLUX</code>_**

## Directory Structure

### <code>/src/app/util</code>

This directory contains various utilities and some of the external API headers
for interacting with the data model layer. In particular the various data type
definitions and the public APIs to the data model are declared in the headers
here.

### <code>/src/app/docs</code>

This directory contains all the documentation for the CHIP ZCL Application
Framework including the file you are reading right now.

### <code>/src/app/clusters</code>

This directory contains all of the cluster implementations included in the CHIP
ZCL Application Framework.

### <code>/src/app/zap-templates</code>

This directory contains all of the templates and helpers specific to CHIP for
the ZAP tool.

## Public APIs

### Attribute changes

When a ZCL attribute is updated in the data model, the framework will call the
<code>postAttributeChangeCallback</code>, if this callback is implemented by the
device it will be informed of the attribute change. The device may react to the
attribute change. For instance, if the on/off attribute in the On-Off Cluster
changes, the application which implements the
<code>postAttributeChangeCallback</code> may reflect this by toggling the actual
pin tied to an LED.
