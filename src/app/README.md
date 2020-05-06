# Application layer for CHIP

## What is this?

The purpose of this folder is to contain functional ZCL Advanced Platform (ZAP)
plugins. A plugin is a discrete piece of Zigbee Cluster Library (ZCL)
functionality. Each plugin contains message processing code for a "Cluster"
within the Zigbee Cluster Library. Each plugin can be individually tested
through its own unit test code and Makefile.

## Usage

To build and run unit tests on these ZAP plugins run

| make test
