# Visual Studio Code Development

[Visual Studio Code](https://code.visualstudio.com/) is a great and simple IDE that can be used to build & develop with for CHIP. 

CHIP supports the docker / remote container workflow that Visual Studio Code, and has an container environment setup automatically. You can read more about this workflow [here](https://code.visualstudio.com/docs/remote/containers).

Tested on:
* MacOS

## Setup Steps

1. Install [Docker](https://www.docker.com/) for your operating system of choice from here: <https://docs.docker.com/install>
1. Install [Visual Studio Code](https://code.visualstudio.com/) for your operating system of choice here: <https://code.visualstudio.com/Download>
1. Install [Git](https://git-scm.com/) if you haven't already
1. Git clone the main CHIP repository here: <https://github.com/project-chip/connectedhomeip>
1. Launch Visual Studio Code, and open the cloned folder from
1. Install the [Remote - Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) extension for Visual Studio Code, this extension allows you to use docker containers as a development backend.
1. Once this is installed, you'll be prompted to reload Visual Studio Code, do so
1. At the bottom right of your Visual Studio Code window you should have a new box prompting you to re-open the window as a container. Hit yes.
1. Now your local machine is building a docker image that has all the tools necessary to build and test CHIP. This can take some time, but will eventually complete and open up the source tree

## Bootstrapping your source tree (one time)
1. Under the "Terminal" menu (or using another shortcut to the same tool), select "Run Task..."
1. Select the "Bootstrap" task

## Building the Source Tree
1. Under the "Terminal" menu select "Run Build Task..."

## Tasks

Located in the [tasks json](../.vscode/tasks.json) file you'll find a list of tasks that can be run from the "Run Task..." command.
Example tasks are "Clean", "Run Pretty Check"

