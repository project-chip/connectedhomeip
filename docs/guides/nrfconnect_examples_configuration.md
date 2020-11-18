# Configuring nRF Connect SDK examples

When you tried to build nRF Connect SDK example applications like [lock-app](../../examples/lock-app/nrfconnect/README.md), [lighting-app](../../examples/lighting-app/nrfconnect/README.md) or [pigweed-app](../../examples/pigweed-app/nrfconnect/README.md), you could notice that some default configuration is used then. Probably after testing an example solution you may want to change something in the application configuration or add new functionalities to build your own application based on sample. This page contains information about how to configure an application in different ways and also sheds some light on the configuration structure to allow you understand building process better.

<hr>

## Configuring application

Changing default application configuration might be done in the many ways and you should choose the one that suits you, depending on your needs. You are allowed to change configuration temporarily, what will be useful for testing impact of changes on the application behavior, but if you want to develop own application, probably you will be interested in making permanent changes. 

<hr>

### Temporary configuration

Temporary solution is basing on editing zephyr/.config, which is storing all configuration options from the whole application generated as a result of building process. After cleaning build and deleting this file, all changes are gone, so this is not possible to save changes permanently this way. 

First thing that needs to be done is bulding the application (see: [Building nRF Connect examples](TODO:) for more details), by typing following command in the example directory:

        # <board_name> should be replaced with the actual target board name
        $ west build -b <board_name>

Next, run the terminal-based interface called menuconfig, by typing command:

        $ west build -t menuconfig

You should be able to see menuconfig terminal window, where you can navigate using arrow keys and other keys, whose functions are described in the bottom of the window. You can make desired changes following terminal instructions and after that press `Q` to save and quit.

At this point configuration changes are applied to the output file and it can be flashed to the device. 

<hr>

### Permanent configuration

Permanent solution is basing on changing configuration Kconfig files used as component parts of the building process and that is a reason why changes are not disappearing after performing another builds. 

The simplest way to insert configuration changes is adding them to the main application configuration file called `prj.conf` and located in the example directory, what results in overriding existing configuration values. This is the best practice and for the majority of cases it should be enough, but if you are interested in understanding big picture of the configuration process see [Configuration structure overview](#configuration-structure-overview) section.

Assigning value to the config option is done by typing its name preceded by `CONIG_` prefix, `=` mark and the value. Please note that configuration options has different types and it is possible to assign them only values of proper type. Example on how to use config options was presented below:

    # assigning logical boolean true value to the option
    CONFIG_SAMPLE_BOOLEAN_OPTION=y
    # assigning numeric integer 1234 value to the option
    CONFIG_SAMPLE_INTEGER_OPTION=1234
    # assigning text string "some_text" value to the option
    CONFIG_SAMPLE_STRING_OPTION="some_text"

For more detailed information visit [Setting Kconfig values](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/guides/kconfig/setting.html#setting-configuration-values).

Please keep in mind that Kconfig files are used in building process, so after changing them you have rebuild your application (see: [Building nRF Connect examples](TODO:) for more details), by typing following command in the example directory:

        # <board_name> should be replaced with the actual target board name
        $ west build -b <board_name>

<hr>

<a name="configuration-structure-overview"></a>

## Configuration structure overview

Configuration of the application is specified using Kconfig files and typically there are many files having impact on the final configuration shape. Actually most of the components have their own configuration files, but don't worry, there is no need to modify each one of them separately. Below you can see what types of files you can meet in the project and which one of them may be important from the user's perspective:

- **Software libraries' configuration files**

    As it was previously written many libraries, components and modules have their own configuration files, but in the majority of cases you will be able to override their configuration values in the main application config file, so you will not have to modify their files unless you are interested in developing those libraries.

- **Application configuration file**

    Every example application has its main configuration file called `prj.conf` located in the example directory. It typically contains application specific configuration or most frequently changed options, but actually almost every configuration can be overrided in this file and probably this file is the most important for the User.

- **Overlays**

    Overlay files are usually used to extract configuration for some specific case or feature from the general application configuration.
    The main difference between them and the application `prj.conf` file is that they are not included automatically, so User may decide whether to build sample with or without them.

- **Board configuration files**

    These are hardware platform dependent configuration files, which are automatically included basing on the compilation target board name and they contain board and its peripherals configuration.

Visit [Kconfig](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/zephyr/guides/kconfig/index.html#kconfig) if you are interested in getting more advanced and detailed information about used configuration structure.