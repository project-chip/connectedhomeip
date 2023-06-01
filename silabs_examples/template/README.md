# Matter EFR32 Template App

<hr>

-   [Matter EFR32 Template App](#matter-efr32-template-app)
    -   [Introduction](#introduction)
    -   [Overview of the application](#overview-of-the-application)
        -   [Starting point](#starting-point)
        -   [Receiving Matter commands](#receiving-matter-commands)
        -   [Using Matter with Clusters](#using-matter-with-clusters)
        -   [Using ZAP to configure clusters](#using-zap-to-configure-clusters)
        -   [The GN Build system](#the-gn-build-system)
    -   [Running the application](#running-the-application)

<hr>

## Introduction

This is a template application. It is a good starting point to build a custom
app.

Find below more information about _how a Matter application works_. See
[How to Create a custom application guide](HOW_TO_CREATE_A_CUSTOM_APP.md) for
more information about building your own application starting from the template
app.

## Overview of the application

### Starting point

The program starts in [the main() function](src/main.cpp). However, the
application's behaviour is mainly defined in the function
[AppTaskMain()](src/AppTask.cpp). AppTaskMain() listens to incoming _events_ and
sends them to the corresponding _handler_.

### Receiving Matter commands

All Matter commands reach the application through the intermediate of the
function [MatterPostAttributeChangeCallback()](src/ZclCallbacks.cpp). Modify
this function to forward the commands it receives to a _function_ of your
choosing. This _function_ should implement the behavior you desire in response
to these commands.

### Using Matter with Clusters

In Matter, commands can only be issued by using a **cluster**. A **cluster** is
a set of _attributes_ and _commands_ which are grouped together under a relevant
theme.

_Attributes_ store values (think of them as variables). _Commands_ are used to
modify the value of _attributes_.

For example, the "On/Off" **cluster** has an _attribute_ named "OnOff" of type
boolean. The value of this _attribute_ can be set to "1" by sending an "On"
_command_ or it can be set to "0" by sending an "Off" _command_.

See "appclusters.pdf" for a list of all available **clusters** or to find useful
documentation about their behavior. The C++ implementation of these **clusters**
is located in [the clusters directory][2]. Note that you can also create your
own custom **cluster**.

### Using ZAP to configure clusters

ZAP is a tool which is used to define the **cluster** configuration of a Matter
application. Using ZAP, you can choose which **cluster** to enable for your
application.

For example, if you enable the "level control" **cluster** for your application,
you application will be able to receive "level control" commands.

ZAP also allows you to modify the settings of a **cluster**. For example, you
can use ZAP to set what the initial value of the "OnOff" _attribute_ will be at
the start of the application.

The clusters configuration of an app are stored in a ".zap" file. For the
template application, the template.zap file is located in
[template_DataModel_config][3]. You can use ZAP to modify this file.

Once the modifications are saved, the "file.zap" can be fed to the ZAP tool to
_generate_ a C implementation of these clusters. The C files generated must be
stored in the [zzz-generated folder](../../../zzz_generated).

### The GN Build system

For the Matter project, GN is used for the building process. The build
configuration is stored inside the [BUILD.gn](BUILD.gn) files and complemented
by other .gn files.

## Running the application

The Lighting App for EFR32 contains useful information about building, flashing
and monitoring a lighting app using an EFR32 device. You can find this
information in its [README file][1].

Color Lighting App also has useful information about setting up a Matter EFR32
app. Refer to its [README file][4] for more information.

###### Last modified August 5th, 2022.

[1]: ../../../examples/lighting-app/efr32/README.md
[2]: ../../../src/app/clusters
[3]: ../template_DataModel_config
[4]: ../../sl-newLight/efr32/README.md
