# Generating PyBind with Binder

## Install Binder
- Installation instructions can be found [here](https://cppbinder.readthedocs.io/en/latest/install.html).


## Binder Command
binder --config `./CHIPDeviceController.config` --suppress-errors --root-module PyChip --prefix `connectedhomeip/src/pybindings/pycontroller/GeneratedFiles` --flat `all_header.hpp` -- -DCHIP_HAVE_CONFIG_H=1 -DCHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK=1 -target `x86_64-apple-macos10.15` -x c++ -std=c++11 `-I/Library/Developer/CommandLineTools/SDKs/MacOSX11.3.sdk/usr/include/` -Iconnectedhomeip/src -Iconnectedhomeip/src/lib -Iconnectedhomeip/out/python_lib/gen/include -Iconnectedhomeip/config/standalone -Iconnectedhomeip/third_party/pigweed/repo/pw_minimal_cpp_stdlib/public -Iconnectedhomeip/third_party/pigweed/repo/pw_polyfill/standard_library_public -Iconnectedhomeip/src/include/

- Include paths may vary and `highlighted` text may also vary.

## Binder Output
- Binder will output several files with prefix PyChip and suffix _#
- The output will be based on the config and hpp file passed in.
- Since binder is new to this project, manual conversion is necessary.
- Example of the output below:

![alt text](https://raw.githubusercontent.com/krypton36/images/master/ss3.png)


## Binder Conversions
- If the file contains everythin you want then the file can be copied over and added.
- If there is just a class and it can be added to the existing, the class binding can be added.

Example of conversion of Logging:
- Search for the module:

   <img src="https://raw.githubusercontent.com/krypton36/images/master/ss16.png " alt="alt text" width="250" height="400">

- Combined these two to a single Logging file

   <img src="https://raw.githubusercontent.com/krypton36/images/master/ss17.png " alt="alt text" width="353" height="147">  <font size="+3">-></font> <img src="https://raw.githubusercontent.com/krypton36/images/master/ss27.png " alt="alt text" width="150" height="100">

- Rename function to bind_PyChip_Logging:

   <img src="https://raw.githubusercontent.com/krypton36/images/master/ss19.png " alt="alt text" width="665" height="25">
- Add the logging binding to Main
 
   <img src="https://raw.githubusercontent.com/krypton36/images/master/ss20.png " alt="alt text" width="265" height="20">
   
   <img src="https://raw.githubusercontent.com/krypton36/images/master/ss23.png " alt="alt text" width="265" height="100"><font size="+3">-></font> <img src="https://raw.githubusercontent.com/krypton36/images/master/ss24.png " alt="alt text" width="165" height="100"><font size="+3">-></font> <img src="https://raw.githubusercontent.com/krypton36/images/master/ss18.png " alt="alt text" width="100" height="180"><font size="+3">-></font><img src="https://raw.githubusercontent.com/krypton36/images/master/ss26.png " alt="alt text" width="300" height="50">

- Add to BUILD GN file:

<img src="https://raw.githubusercontent.com/krypton36/images/master/ss25.png " alt="alt text" width="500" height="200">

- Lastly Test:

   <img src="https://raw.githubusercontent.com/krypton36/images/master/TestGif.gif" alt="alt text" width="500" height="200">