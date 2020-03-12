## Build Documentation

Currently using simple GNU makefiles on Linux or MacOS (Tested on MacOS and Ubuntu 18.04).

Run:

```
$ make all
```

Assuming you have all the required tools installed, this will tidy, format, and run code
coverage on the entire tree.  Tests are built into the make system.

### Tool Prerequisites

To take advantage of all the current capabilities of the make system, you'll want:

* Bash 4.0 or greater
* GNU make
* C and C++ compilers
* clang-tidy
* clang-format
* gcov
