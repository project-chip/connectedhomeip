# Introduction

This section provides an overview of the Matter 15.4 setup and operation. A typical simple Matter 15.4 (Thread) network setup is shown in the following
image. 

**Matter 15.4 Setup** ![Overview](./images/thread_overview.png)

The setup consists of the following four elements:

1.  A **Controller** such as an app running on a phone or the ChipTool running
    on a Linux box or Raspberry Pi.
2.  An Open Thread Border Router (**OTBR**) running on a Linux box or Raspberry
    Pi.
3.  A Radio Co-Processor (**RCP**), which the OTBR uses to communicate with
    Thread network (not shown). This is attached to the Raspberry Pi.
4.  An End Device such as a light or switch, which is the Matter Accessory
    Device (**MAD**).

The flow of the setup described above is as follows:

1. The controller commissions the End Device directly over Bluetooth – this
   makes the End Device join the Thread network and the CHIP fabric.
2. After commissioning, the Bluetooth connection is terminated and all further
   communication is done over Matter.
3. The controller sends ZCL commands, such as the OnOff Toggle, and the End
   Device performs the corresponding action, such as turning the End Device's
   LED on or off

<br>

A Matter network can be built in a number of ways using a
combination of Silicon Labs hardware, a Raspberry Pi, and any external
controller (Macbook, Ubuntu, Android, etc.)

The suggested method involves using a Raspberry Pi to function as both the
controller and the OTBR, with a Silicon Labs device as the MAD.

An alternate configuration is to use a Macbook as the controller, a
Raspberry Pi as the OTBR, and a Silicon Labs Device as the MAD. This requires
additional routing between the controller and OTBR.

----
[Table of Contents](../README.md) | [Thread Demo](./DEMO_OVERVIEW.md) | [Wi-Fi Demo](../wifi/DEMO_OVERVIEW.md)