# Matter Python REPL

The Matter Python REPL is a native IPython shell environment loaded with a
Python-wrapped version of the C++ Matter stack to permit interacting as a
controller to other Matter-compliant devices.

You can interact with the REPL in a one of three ways:

1. Through an IPython shell built from source
2. Through a locally built 'REPL Playground', which is a Jupyter Lab environment
   that supports launching both a native IPython shell as well as Guide
   Notebooks.
3. Through a cloud-hosted REPL Playground that has pre-built versions of the
   REPL that can be interacted with through a browser.

This guide provides instructions on how to utilize its various features.

###

## Source files

You can find source files of the Python CHIP Controller tool in the
`src/controller/python` directory.

The tool uses the generic CHIP Device Controller library, available in the
`src/controller` directory.

## Building

Please follow the instructions
[here](./python_chip_controller_building.md#building-and-installing) to build
the Python virtual environment.

### Building for `arm64` e.g. for Raspberry Pi

Matter code relies on code generation for cluster-specific data types and
callbacks. A subset of code generation is done at compile time by `zap-cli`. ZAP
is generally installed as a third-party tool via CIPD during the build
environment bootstrap. However, zap packages are currently NOT available for
`arm64` (like when compiling on Raspberry PI.). In this case, you have 2
choices.

1. You could check out zap from source as described in
   [Code Generation - Installing zap and environment variables](https://github.com/project-chip/connectedhomeip/blob/master/docs/code_generation.md#Installing-zap-and-environment-variables)
   and proceed with the
   [instructions](./python_chip_controller_building.md#building-and-installing)
   to build the Python virtual environment.

2. When compile-time code generation is not desirable, then pre-generated output
   code can be used. To understand about code generation and pre-generating
   matter code see.
   [Code generation - Pre-generation](https://github.com/project-chip/connectedhomeip/blob/master/docs/code_generation.md#Pre-generation).
   To build and install the Python CHIP controller with pre-generated files use
   the -z argument that points to the directory of pre-generated code:

    ```shell
    scripts/build_python.sh -m platform -i out/python_env -z "/some/pregen/dir"
    ```

    > Note: To get more details about available build configurations, run the
    > following command: `scripts/build_python.sh --help`

## Launching the REPL

1. Activate the Python virtual environment:

    ```shell
    source out/python_env/bin/activate
    ```

2. Launch the REPL.

    ```shell
    sudo out/python_env/bin/chip-repl
    ```

> By default, the REPL points to `/tmp/repl-storage.json` for persistent
> storage. You can over-ride that location by passing in `--storage-path <path>`
> to the above invocation.

## REPL Playground

The REPL playground is a Jupyter Lab instance that allows you to interact with
the REPL from a web browser (or a Jupyter Notebook client of your choice!). It
contains the entire REPL encapsulated as an IPython kernel.

### Locally Hosted

The locally hosted version requires you to follow the build instructions below
to initially setup your Python environment.

Then:

1. Install
   [Jupyter Lab](https://jupyterlab.readthedocs.io/en/stable/getting_started/installation.html)
   (not within the virtualenv!)

```
pip3 install jupyterlab ipykernel
```

2. Install the [Python LSP](https://github.com/jupyter-lsp/jupyterlab-lsp)
   extension for better code autocompletion in the playground.

```
pip3 install jupyterlab-lsp
pip3 install python-lsp-server
```

3. Every virtual env needs to be installed as a 'kernel' in Jupyter Lab. To do
   so, activate the virtual env and run:

```
python -m ipykernel install <name-for-your-kernel>
```

4. Navigate to the SDK root folder and launch Jupyter Lab (not from within
   virtual env!)

```
jupyter-lab
```

This will automatically launch the playground on your browser.

5. (Optional) To enable live code completions, in the Jupyter Lab Interface, go
   to "Settings" → "Advanced Settings Editor" → "Code Completion". In the "User
   Preferences" section, add the following:

```
{
    "continuousHinting": true,
    "showDocumentation": true,
    "theme": 'material'
}
```

Now, when you type, it should auto complete functions/objects/etc.

For more details, go to the
[Python LSP](https://github.com/jupyter-lsp/jupyterlab-lsp) page.

### Cloud Hosted

A pre-built version of the REPL playground is made available through the cloud.
This is ideal if you're new to the REPL and want to try it out without having to
follow the build and launch instructions below. You can also use this to
prototype various bits of logic in Python as well as interact with
all-clusters-app from a browser.

The playground can be accessed [here](http://35.236.121.59/).

> **NOTE:** You have to create a user ID when accessing the above for the first
> time (password can be blank). That creates a sandboxed environment for you to
> play in. There-after, you'll always be re-directed straight to the Jupyter Lab
> landing page.

> **NOTE:** The sandbox is temporary. After an hour of inactivity, the sandbox
> is deleted and your saved contents will be lost.

For more information on Jupyter Lab, check out
[these](https://jupyterlab.readthedocs.io/en/stable/user/interface.html) docs.

### IPython REPL

Going through the above isn't terribly useful, since all you'll be able to do is
launch the REPL environment itself through the IPython shell.

To launch the IPython REPL, launch "matter-env" from the "Console" tab in the
Launcher.

### Guide Notebooks

A number of Jupyter Notebooks have been written that serve as both guides for
interacting with the REPL _as well as_ being launchable directly into the
cloud-hosted playground.

The following icon is present at the top of applicable guides that can be
launched into the playground:

![Launch playground icon](https://i.ibb.co/hR3yWsC/launch-playground.png")
<br></br>

## Guides

[REPL Basics](https://deepnote.com/viewer/github/project-chip/connectedhomeip/blob/master/docs/guides/repl/Matter_REPL_Intro.ipynb)

[Using the IM](https://deepnote.com/viewer/github/project-chip/connectedhomeip/blob/master/docs/guides/repl/Matter_Basic_Interactions.ipynb)

[Multi Fabric Commissioning](https://deepnote.com/viewer/github/project-chip/connectedhomeip/blob/master/docs/guides/repl/Matter_Multi_Fabric_Commissioning.ipynb)

[Access Control](https://deepnote.com/viewer/github/project-chip/connectedhomeip/blob/master/docs/guides/repl/Matter_Access_Control.ipynb)

## Testing

We also provide `mobile-device-test.py` for testing your accessories, you can
run it manually or using a wrapper script.

### Usage

mobile-device-test.py provides the following options for running the tests:

```
  --controller-nodeid INTEGER     NodeId of the controller.
  --device-nodeid INTEGER         NodeId of the device.
  -a, --address TEXT              Skip commissionee discovery, commission the
                                  device with the IP directly.

  -t, --timeout INTEGER           The program will return with timeout after
                                  specified seconds.

  --discriminator INTEGER         Discriminator of the device.
  --setup-pin INTEGER             Setup pincode of the device.
  --enable-test TEXT              The tests to be executed. By default, all
                                  tests will be executed, use this option to
                                  run a specific set of tests. Use --print-
                                  test-list for a list of applicable tests.

  --disable-test TEXT             The tests to be excluded from the set of
                                  enabled tests. Use --print-test-list for a
                                  list of applicable tests.

  --log-level [ERROR|WARN|INFO|DEBUG]
                                  The log level of the test.
  --log-format TEXT               Override logging format
  --print-test-list               Print a list of test cases and test sets
                                  that can be toggled via --enable-test and
                                  --disable-test, then exit

  --help                          Show this message and exit.
```

By default, all tests will be executed, however, you can exclude one or more
tests or only include a few tests if you want.

For example, if you are working for commissioning, then you may want to exclude
the data model test cases by adding `--disable-test datamodel` to disable all
data model tests.

Some tests provides the option to exclude them. For example, you can use
`--disable-test ClusterObjectTests.TestTimedRequestTimeout` to exclude the
"TestTimedRequestTimeout" test case.

It is recommended to use the test wrapper to run mobile-device-test.py, for
example, you can run:

```shell
./scripts/tests/run_python_test.py --app chip-all-clusters-app --factory-reset
```

It provides some extra options, for example:

```
optional arguments:
  -h, --help                show this help message and exit
  --app APP                 Path to local application to use, omit to use external apps.
  --factory-reset           Remove app config and repl configs (/tmp/chip* and /tmp/repl*) before running the tests.
  --app-args APP_ARGS       The extra parameters passed to the device side app.
  --script SCRIPT           Path to the test script to use, omit to use the default test script (mobile-device-test.py).
  --script-args SCRIPT_ARGS Arguments for the REPL test script
```

You can pass your own flags for mobile-device-test.py by appending them to the
command line with two dashes, for example:

```shell
./scripts/tests/run_python_test.py --app out/linux-x64-all-clusters-no-ble-no-wifi-tsan-clang/chip-all-clusters-app --factory-reset --script-args "-t 90 --disable-test ClusterObjectTests.TestTimedRequestTimeout"
```

will pass `-t 90 --disable-test ClusterObjectTests.TestTimedRequestTimeout` to
`mobile-device-test.py`
