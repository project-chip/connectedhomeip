# Visual Studio Code Development

[Visual Studio Code](https://code.visualstudio.com/) is a great and simple IDE
that can be used to build & develop with for Matter.

Matter supports the docker / remote container workflow in Visual Studio Code,
and has a container environment setup automatically. You can read more about
this workflow [here](https://code.visualstudio.com/docs/remote/containers).

Tested on:

-   macOS 10.5
-   Windows 10 Pro + WSL + Ubuntu 18 LTS
-   Linux - Fedora Core 35 distribution

## Setup Steps

1. _Windows Only_ Enable the Windows Subsystem for Linux (WSL) following
   instructions here:
   <https://docs.microsoft.com/en-us/windows/wsl/install-win10>
1. _Windows Only_ Install Ubuntu from the Windows App Store here:
   <https://www.microsoft.com/en-us/p/ubuntu-1804-lts/9n9tngvndl3q>
1. Install [Docker](https://www.docker.com/) for your operating system of choice
   from here: <https://docs.docker.com/engine/install>
1. Install [Visual Studio Code](https://code.visualstudio.com/) for your
   operating system of choice here: <https://code.visualstudio.com/Download>
1. Install [Git](https://git-scm.com/) if you haven't already
1. _Windows Only_ Enable git to use LF instead of CLRF by default:
   `git config --global core.autocrlf false`
1. Git clone the main Matter repository here:
   <https://github.com/project-chip/connectedhomeip>
1. Launch Visual Studio Code, and open the cloned folder from
1. Install the
   [Remote - Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)
   extension for Visual Studio Code, this extension allows you to use docker
   containers as a development backend.
1. Once this is installed, you'll be prompted to reload Visual Studio Code, do
   so
1. At the bottom right of your Visual Studio Code window you should have a new
   box prompting you to re-open the window as a container. Hit yes.
1. _Windows Only_ Update your Visual Studio Code settings as documented here:
   https://code.visualstudio.com/docs/editor/integrated-terminal#_configuration
   to use Bash on Ubuntu (on Windows) eg:
   `"terminal.integrated.shell.windows": "C:\\Windows\\System32\\bash.exe"`
1. Now your local machine is building a docker image that has all the tools
   necessary to build and test Matter. This can take some time, but will
   eventually complete and open up the source tree

## Bootstrapping your source tree (one time)

1. Under the "Terminal" menu (or using another shortcut to the same tool),
   select "Run Task..."
1. Select the "Bootstrap" task

## Building the Source Tree

1. Under the "Terminal" menu select "Run Build Task..."

## Tasks

Located in the [tasks json](../.vscode/tasks.json) file you'll find a list of
tasks that can be run from the "Run Task..." command. Example tasks are "Clean",
"Run Pretty Check"

Developers are encouraged to add tasks to the
[tasks json](../.vscode/tasks.json) over time to make sure everyone is using the
same base configuration and build.

### Current base tasks are listed here

-   Main Build - Build the default configuration (i.e., Linux OpenSSL)
-   Run Unit and Functional Tests - Test the default configuration
-   Build & Test (all) - Build & Test various configurations (Linux variants,
    Android, EFR32)
-   Update compilation database - Update the database used by IntelliSense
    (needed for cross references, completion)
-   Bootstrap - On a clean tree, pull in the third party dependencies required
-   Clean Output - Remove build artifacts
-   Clean Tree - Full (and destructive) git clean of the tree

## Launch Tasks

Located in the [launch json](../.vscode/launch.json) file you'll find a list of
build & run jobs that can be run from the "Run" tab and start a run or debug
session.

Developers are encouraged to add tasks to the
[launch json](../.vscode/launch.json) over time to make sure everyone is using
the same base debugging setup.

## Submitting a Pull Request - Practical Advice

### Before submitting a PR, make sure these commands run and succeed

-   Run task: "Build & Test (all)"

## Common Issues

-   [Missing Git credential](https://code.visualstudio.com/docs/remote/containers#_sharing-git-credentials-with-your-container)
-   [Missing Git SSH keys](https://code.visualstudio.com/docs/remote/containers#_sharing-git-credentials-with-your-container)
-   [Using GPG signing keys](https://github.com/microsoft/vscode-remote-release/issues/72)

## Docker FAQ

### DNS problem: can't resolve archive.ubuntu.com

A common problem encountered is that a container can't resolve
`archive.ubuntu.com` and can't install anything via `apt-get`, during the
creation of the container image, resulting in an error like:

```
E: Package 'locales' has no installation candidate
The command '/bin/sh -c apt-get install -y locales && localedef -i en_US -c -f UTF-8 -A /usr/share/locale/locale.alias en_US.UTF-8' returned a non-zero code: 100
```

Most common reason for this is that the DNS for docker daemon has not been set
up correctly and is simply using a default: 8.8.8.8, which in many corporate or
more secure environments is not accessible. A typical solution for this is to
put a following key/value into your system-wide docker `daemon.json` (Typically
located under `/etc/docker/daemon.json` on a Linux system):

```
"dns": ["<<IP ADDRESS OF YOUR NAMESERVER>>", "8.8.8.8"]
```

You can obtain the address you should put into that line of your nameserver by
running:

```
nmcli dev show | grep 'IP4.DNS'
```

After you update the dns, you should restart docker, specific to your system. On
a typical Linux workstation, you do this via:

```
sudo service docker restart
```

Alternatively, you can also pass the `--dns` argument to your docker daemon, but
creating a `daemon.json` and following the above method will solve the problem
system-wide.

## Visual Studio Code FAQ

-   _Highly_ recommend you read through
    [this page](https://code.visualstudio.com/docs/getstarted/settings) to learn
    how to configure Visual Studio Code to suit your style:
    <https://code.visualstudio.com/docs/getstarted/settings>
-   Great primer set of videos
    [here](https://code.visualstudio.com/docs/getstarted/introvideos)
    <https://code.visualstudio.com/docs/getstarted/introvideos>

## Visual Studio Code Recommended Settings

-   Configure the editor to format on save, in your Visual Studio Code Settings:
    `"editor.formatOnSave": true`
-   Configure the clang-format extension `@ext:xaver.clang-format`, it is
    installed in the docker container. Make sure all languages are enabled
