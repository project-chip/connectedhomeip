# Building the Unify Matter Bridge for Debian Bullseye (Or similar Ubunutu) x86_64 

> **Note:**
> In the following subsections the commands should either be run on your local development machine or inside a running Docker container, as distinguished by the structure of the example.
>
> - _some-command_ should be executed on your local machine.
>   - _`dev-machine:~$ some-command`_
> - _some-other-command_ should be executed inside the Docker container.
>   - _`root@docker:/<dir>$ some-other-command`_

## Set Up the Matter Build Environment

Once you have all the necessary submodules, source the Matter environment with the following command. This loads a number of build tools and makes sure the correct toolchains and compilers are used for compiling the Unify Matter Bridge.

## Check Out Submodules

Check out the necessary submodules with the following command.

```bash
dev-machine:~/matter$ ./scripts/checkout_submodules.py --platform linux
```

## Clone and Stage the Unify SDK Repository 

> 🔴 Assuming you have cloned the matter repo in `~/matter` 

```bash
dev-machine:~/matter$ git clone --depth 1 https://github.com/SiliconLabs/UnifySDK.git --recursive ../uic-matter
```

## Build the docker container (host compilation)

```bash
dev-machine:~/matter$ docker build -t unify-matter-host --build-arg ARCH=amd64 silabs_examples/unify-matter-bridge/docker/
```
## Run the docker container (host compilation)

> 🔴
> Make sure the directory structure is like follows where Unify repo `uic-matter/` and matter repo `matter/` are at same directory level
> 
> ```
> .
> ├── matter
> └── uic-matter
> ```

Starting the docker from `matter/` directory where you cloned the matter repo: Here we asuume `matter/` is in `~`

```bash
dev-machine:~$ cd matter 
dev-machine:~/matter$ docker run -it -v $PWD:/matter -v $PWD/../uic-matter:/uic unify-matter-host
```

Compile and install libunify for host.

```bash
root@docker:/uic$ cmake -DCMAKE_INSTALL_PREFIX=$PWD/stage -GNinja -B build_unify_amd64/ -S components
root@docker:/uic$ cmake --build build_unify_amd64
root@docker:/uic$ cmake --install build_unify_amd64 --prefix $PWD/stage
root@docker:/uic$ export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$PWD/stage/share/pkgconfig
```

Activate Matter development environment.

```bash
root@docker:/matter$ source ./scripts/activate.sh
```

## Compile the Unify Matter Bridge 


```bash
root@docker:/matter$ cd silabs_examples/unify-matter-bridge/linux/
root@docker:/matter/silabs_examples/unify-matter-bridge/linux$ gn gen out/host
root@docker:/matter/silabs_examples/unify-matter-bridge/linux$ ninja -C out/host
```

> 🔴 After building, the `unify-matter-bridge` binary is located at `/matter/silabs_examples/unify-matter-bridge/linux/out/host/obj/bin/unify-matter-bridge`.

## Compile the chip-tool

The `chip-tool` is a CLI tool that can be used to commission the bridge and to control end devices.

```bash
root@docker:/matter$ cd examples/chip-tool

root@docker:/matter/examples/chip-tool$ gn gen out/amd64

root@docker:/matter/examples/chip-tool$ ninja -C out/amd64
```

> 🔴 After building, the chip-tool binary is located at `/matter/examples/chip-tool/out/amd64/obj/bin/chip-tool`.

## Troubleshooting

Refer to Troubleshooting section in [readme_building.md](readme_building.md) 
