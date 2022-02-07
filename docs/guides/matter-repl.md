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
[here](./python_chip_controller_building.md#building) to build the Python
virtual environment.

## Launching the REPL

1. Activate the Python virtual environment:

    ```
    source out/python_env/bin/activate
    ```

2. Launch the REPL.

    ```
    sudo out/python_env/bin/chip-repl
    ```

> By default, the REPL points to `/tmp/repl-storage.json` for persistent
> storage. You can over-ride that location by passing in `--storagepath <path>`
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

<a href="https://www.w3schools.com">
<img src="https://i.ibb.co/hR3yWsC/launch-playground.png" alt="drawing" width="130"/>
</a>
<br></br>

## Guides

[REPL Basics](https://deepnote.com/viewer/github/project-chip/connectedhomeip/blob/master/docs/guides/repl/Matter%20-%20REPL%20Intro.ipynb)

[Using the IM](https://deepnote.com/viewer/github/project-chip/connectedhomeip/blob/master/docs/guides/repl/Matter%20-%20Basic%20Interactions.ipynb)

[Multi Fabric Commissioning](https://deepnote.com/viewer/github/project-chip/connectedhomeip/blob/master/docs/guides/repl/Matter%20-%20Multi%20Fabric%20Commissioning.ipynb)

[Access Control](https://deepnote.com/viewer/github/project-chip/connectedhomeip/blob/master/docs/guides/repl/Matter%20-%20Access%20Control.ipynb)
