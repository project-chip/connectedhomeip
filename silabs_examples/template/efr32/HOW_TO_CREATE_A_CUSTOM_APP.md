# How to Create an EFR32 Matter Application

<hr>

-   [How to Create an EFR32 Matter Application](#how-to-create-an-efr32-matter-application)
    -   [Introduction](#introduction)
    -   [Create your app](#create-your-app)
        -   [Create a new directory](#create-a-new-directory)
        -   [Modify the application](#modify-the-application)
            -   [Using ZAP to configure clusters](#using-zap-to-configure-clusters)
                -   [Create a Data Model](#create-a-data-model)
                -   [Cluster Configuration and Generation](#cluster-configuration-and-generation)
            -   [Modifying the GN Build files](#modifying-the-gn-build-files)
            -   [Modifying the source code](#modifying-the-source-code)
        -   [Running your app](#running-your-app)

<hr>

## Introduction

This guide will walk you through the process of creating your own application,
starting from the [template application][1].

Please read the template application’s [README.md file](README.md) for a summary
about the functioning of a Matter EFR32 application.

## Create your app

Follow the steps below to create a new application named **your_app**.

### Create a new directory

1. Copy the [template folder][1], paste it in the [silabs_examples][2]
   directory. Rename the folder as **your_app**.

### Modify the application

Before modifying the source code, follow the steps below to make sure that
**your_app** is a fully functional custom application.

#### Using ZAP to configure clusters

_Clusters_ determine which commands **your_app** can receive. To configure
_clusters_ for your app, you need a data model. For more information about
clusters, see [README.md](README.md).

##### Create a Data Model

Before we can modify the cluster configuration, we need to create a new data
model configuration for **your_app**.

If you followed step 1, in **your_app** folder, you should have two directories.
One of them should have a name similar to “template_DataModel_config”. We will
modify this folder to create a custom Data Model configuration named
**your_data_model_config**.

2. Rename the folder “template_DataModel_config” to
   “**your_data_model_config**”.

Inside **your_data_model_config**, you will find a file named “sl_template.zap”.
This file contains your cluster configuration.

3. Rename “sl_template.zap” to “**your_app.zap**”.

Inside **your_data_model_config** you should also find a BUILD.gn file. This
file needs to be modified as well, but we will come back to it later.

When you feed “**your_app**.zap” to Zaptool, ZAP will translate the ".zap" file
into a C implementation. This process is called _Generation_. We need to create
a directory for **your_app** where Zap tool will output the C files. This
directory needs to be located inside [zzz_generated][3].

4. Naviguate to [zzz_generated][3]. Inside, create a new directory named
   “**your_app**”. Inside the folder you just created, create another folder
   named “zap-generated”.

We want Zaptool to generate the C files in the directory you just created,
namely: [zzz_generated][3]/**your_app**/zap-generated. To do so, go back to
**your_data_model_config** and make the following modifications.

5. Naviguate to the file “BUILD.gn” located in **your_data_model_config**.
   Inside, change the following lines of code:

```gn
	# Change this line of code
	chip_data_model("sl_template") {
	# to this:
	chip_data_model("your_data_model_config") {
```

```gn
	# Change the following line of code
	zap_file = "sl_template.zap"
	# to:
	zap_file = "your_app.zap"
```

```gn
	# Finally, modify the following line
	zap_pregenerated_dir = "${chip_root}/zzz_generated/sl_template/zap-generated"
	# to become:
	zap_pregenerated_dir = "${chip_root}/zzz_generated/your_app/zap-generated"
```

Now, we have our own configurable data model. We can now modify the cluster
configuration and _Generate_ with ZAP.

##### Cluster Configuration and Generation

Follow the steps below to modify the cluster configuration stored in
“**your_app**.zap”. To do that, you need to run ZAP. See [this guide][4] to
learn how to run zap (it can take some time). Once you have Zap up and running,
proceed with the steps below:

6. Open a terminal. Navigate to the [root directory][5] of your Matter
   repository.

7. Run ZapTool. Enter this bash command in your terminal:

    ```bash
    # While in the root directory, run this command:
    ./scripts/tools/zap/run_zaptool.sh
    ```

An application running on “Electron” should appear on your screen after a
moment.

8. Now open your file.zap in the ZapTool. For Mac, follow these steps

    1. Go to the App Menus (top bar).
    2. Select `Electron > Open File...`
    3. Modify the cluster configuration using the UI of Zaptool.
    4. When finished modifying the file, select `Electron > Save`
    5. Close Zaptool.

9. To _Generate_ the corresponding C files, execute the following command:

    ```bash
    # While in the root directory, run this command:
    ./scripts/tools/zap/generate.py silabs_examples/your_app/your_data_model_config/your_app.zap -o zzz_generated/your_app/zap-generated
    # The first argument is the path to the "generate script". It should not be changed.
    # The second argument is the path to **your_app**.zap. Zap will use it to *Generate*.
    # The third argument "-o" should not be modified.
    # The last argument is the directory where the C files will be *Generated*.
    ```

If everything worked as expected, you should now have a file named
"**your_app**.matter" inside the folder **your_data_model_config**.

Finally, there is a file which ZAP does not generate. It is called
"af-gen-event.h" and it is necessary to run a Matter application.

10. Navigate to one of the zzz-generated output directory (one example is the
    [lighting-app output directory]). Copy the "af-gen-event.h" file and paste
    it inside "zzz-generated/**your_app**/zap-generated". This step needs to be
    done only once for each new app.

Now, you have successfully modified and generated a Matter cluster
configuration.

**Important** The process is not finished! You must update the GN files to point
to **your_data_model_config**. Read the section [Modifying the GN Build
files][6] to learn more.

#### Modifying the GN Build files

You need to add the **your_data_model_config** to the build configuration so
that the compiler will compile the C files you generated in the previous steps.

To do so, you need to modify [BUILD.gn](BUILD.gn) which is the main build file
of your application.

11. Naviguate to the main [BUILD.gn](BUILD.gn) file, perform this modification:

    ```gn
    # Find the "efr32_executable" section of the file. Inside it you will find a sub-section called
    # deps = [
    # Here, there is list of dependencies listed.
    # Remove any DataModel dependencies from this section. They will look like this:
    # "${chip_root}/silabs_examples/template/template_DataModel_config",
    # or like this:
    # "${chip_root}/examples/lighting-app/lighting-common",
    # Remove them all, we want our app to use your_data_model_config only.
    # Now, add your_data_model_config as a dependency.
    # After you are done, "deps" should look something like this:
    deps = [
        ":sdk",
        "${chip_root}/silabs_examples/your_app/your_data_model_config",
        "${chip_root}/src/lib",
        "${chip_root}/src/setup_payload",
    ]
    ```

Now, you have a configurable custom Matter application. You can proceed to
modifying the source code of **your_app**.

#### Modifying the source code

As mentioned in the [README](README.md) of this app, Matter commands are
received in the function [MatterPostAttributeChangeCallback()][7]. You can
modify this function to forward Matter commands to a custom function of your
choice and handle them there. Refer to [Color Lighting][8] to see how this can
be done.

Note: If you add .cpp files to **your_app**, modify [BUILD.gn](BUILD.gn) to take
account of it. To do so, navigate to the section "efr32_executable" in the file,
locate the "sources" subsection and add your new "file.cpp" there.

Note: Both in the source files and in the [BUILD.gn](BUILD.gn), you might have
to rename certain variables. Simply renaming any instance of "sl_template" or
"template" to "**your_app**" should be enough.

### Running your app

Color Light app's [README][9] contains useful information about building,
flashing and running a Matter EFR32 app. A more complete guide can be found in
Lighting-app's [README](../../../examples/lighting-app/efr32/README.md).

###### Last modified August 5th, 2022.

[1]: ../../template/
[2]: ../../../silabs_examples/
[3]: ../../../zzz_generated/
[4]: ../../../third_party/zap/repo/README.md
[5]: ../../../
[6]: #Modifying-the-GN-Build-files
[7]: src/ZclCallbacks.cpp
[8]: ../../sl-newLight/efr32/src/
[9]: ../../sl-newLight/efr32/README.md
