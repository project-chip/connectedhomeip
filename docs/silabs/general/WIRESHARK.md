# Using Wireshark to Capture Network Traffic in Matter

When developing a wireless application it is often useful to be able to
visualize the network traffic. [Wireshark](https://www.wireshark.org/) is a
great tool for this, but you can't use Wireshark alone. You first have to
capture the network traffic off a wireless network interface. Fortunately
Silicon Labs has provided an open source project for capturing network traffic
off its devices called the
[Java Packet Trace Library or Silabs PTI .jar](https://github.com/SiliconLabs/java_packet_trace_library)

Here are the following steps for capturing and visualizing network traffic with
Wireshark and the Java Packet Trace Library:

## 1. Clone and Build Silabs-PTI.jar Out of the Java Packet Trace Library

The
[Java Packet Trace Library](https://github.com/SiliconLabs/java_packet_trace_library)
can be built locally for your development platform. First clone the repository
and then build the library according to the
[instructions](https://github.com/SiliconLabs/java_packet_trace_library/blob/master/README.md)

## 2. Download Wireshark

If you don't have Wireshark, you can
[download Wireshark for your development platform here](https://www.wireshark.org/).

## 3. Follow Instructions for Wireshark Integration

To capture from a Silicon Labs device like a WSTK use
the Silabs-PTI.jar utility you built in the previous step. You will further need
to integrate the execution of the utility into Wireshark through Wireshark's
`excap` interface. A complete guide to
[Wireshark Integration is provided here](https://github.com/SiliconLabs/java_packet_trace_library/blob/master/doc/wireshark.md).
You integrate the Silabs-PTI.jar utility into Wirehshark by adding a small
script into Wireshark's `excap` directory. Make sure that you make the script
executable using something like

```shell
$ chmod 777 <myexcapscript>
```

This will make it so that Wireshark can execute the script and integrate the
WSTK interfaces into its capture functionality.

## 4. Run Wireshark and Discover and Capture using Silabs-PTI.jar

In order to capture from an adapter such as a WSTK using the utility
Silabs-PTI.jar that you built in step 2, your adapter must be connected to the
network via Ethernet. If your adapter is not connected via Ethernet and only via
USB you will need to use the `silink` utility to make the adapter show up as a
localhost.

Once your adapter is connected, you can test out the visibility of your WSTK on
the network by running Silabs-PTI.jar from the command line using the following
command:

```shell
$ java -jar silabs-pti-<VERSION>.jar -discover
```
