# Building the Unify Matter Bridge

This build guide cross-compiles for arm64 architecture to be run on Unify's reference platform - a Raspberry Pi 4 (RPi4) with the 64-bit version of Debian Bullseye.

> **Note:**
> In the following subsections the commands should either be run on your local development machine or inside a running Docker container, as distinguished by the structure of the example.
>
> - _some-command_ should be executed on your local machine.
>   - _`dev-machine:~$ some-command`_
> - _some-other-command_ should be executed inside the Docker container.
>   - _`root@docker:/<dir>$ some-other-command`_

## Check Out Submodules

> 🔴 Assuming you have cloned the matter repo in `~/matter` 

Check out the necessary submodules with the following command.

```bash
dev-machine:~/matter$ ./scripts/checkout_submodules.py --platform linux
```

## Clone and Stage the Unify SDK Repository 

> 🔴 Assuming you have cloned the matter repo in `~/matter` 

```bash
dev-machine:~/matter$ git clone --depth 1 https://github.com/SiliconLabs/UnifySDK.git --recursive ../uic-matter
```

## Build the Docker Container (arm64 compilation)

```bash
dev-machine:~/matter$ docker build -t unify-matter silabs_examples/unify-matter-bridge/docker/
```

## Run the docker container  (arm64 compilation)

> 🔴
> Make sure the directory structure is like follows where Unify repo `uic-matter/` and matter repo `matter/` are at same directory level
> 
> ```shell
> .
> ├── matter
> └── uic-matter
> ```

Start the docker from `matter/` directory where you cloned the matter repo: Here we asuume `matter/` is in `~`
```bash
dev-machine:~$ cd matter/
dev-machine:~/matter$ docker run -it -v $PWD:/matter -v $PWD/../uic-matter:/uic unify-matter
```

## Build libunify

The Unify Matter Bridge depends on the libunify library from the Unify project.

This library must first be compiled for the target system, by changing directory to the `/uic` directory and running the following:

```bash
root@docker:/uic$ cmake -DCMAKE_INSTALL_PREFIX=$PWD/stage -GNinja -DCMAKE_TOOLCHAIN_FILE=../cmake/arm64_debian.cmake  -B build_unify_arm64/ -S components -DBUILD_TESTING=OFF
root@docker:/uic$ cmake --build build_unify_arm64
root@docker:/uic$ cmake --install build_unify_arm64 --prefix $PWD/stage
```

After cross-compiling the Unify library, set two paths in the PKG_CONFIG_PATH.
The first path is for the staged Unify library and the second is for cross compiling to arm64.

```bash
root@docker:/uic$ export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$PWD/stage/share/pkgconfig
root@docker:/uic$ export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/lib/aarch64-linux-gnu/pkgconfig
```

If you want to be able to use Zap to generate code from Unify XML files you need to export UCL_XML_PATH as well.

```bash
root@docker:/uic$ export UCL_XML_PATH=$PWD/stage/share/uic/ucl
```

## Run activate in matter 

Once you have all the necessary submodules, source the Matter environment with the following command. This loads a number of build tools and makes sure the correct toolchains and compilers are used for compiling the Unify Matter Bridge.

Make sure you are in `matter/` directory

```bash
root@docker:/matter$ source ./scripts/activate.sh
```

## Compile the Unify Bridge

```bash
root@docker:/matter$ cd silabs_examples/unify-matter-bridge/linux/

root@docker:/matter/silabs_examples/unify-matter-bridge/linux$ gn gen out/arm64 --args='target_cpu="arm64"'

root@docker:/matter/silabs_examples/unify-matter-bridge/linux$ ninja -C out/arm64
```

> 🔴 After building, the `unify-matter-bridge` binary is located at `/matter/silabs_examples/unify-matter-bridge/linux/out/arm64/obj/bin/unify-matter-bridge`.

## Compile the chip-tool

The `chip-tool` is a CLI tool that can be used to commission the bridge and to control end devices.

```bash
root@docker:/matter$ cd examples/chip-tool

root@docker:/matter/examples/chip-tool$ gn gen out/arm64 --args='target_cpu="arm64"'

root@docker:/matter/examples/chip-tool$ ninja -C out/arm64
```

> 🔴 After building, the chip-tool binary is located at `/matter/examples/chip-tool/out/arm64/obj/bin/chip-tool`.

## Unit Testing

Unit testing is always a good idea for quality software. Documentation on writing unit tests for the Matter Unify Bridge is in the
[README.md](https://github.com/SiliconLabs/matter/blob/latest/silabs_examples/unify-matter-bridge/linux/src/tests/README.md).

## Troubleshooting

1. If you do not source the `matter/scripts/activate.sh` as described above in [Set Up the Matter Build Environment](../general/SOFTWARE_REQUIREMENTS.md), `gn` and other common
   build tools will not be found.
2. If you do not export the `pkgconfig` for the `aarch64-linux-gnu` toolchain as described above in [Build libunify](#build-libunify)
   you will get errors such as `G_STATIC_ASSERT(sizeof (unsigned long long) == sizeof (guint64));`
3. If you are compiling unit tests, do not try to compile the Unify Matter Bridge at
   the same time. This will not work as when compiling unit tests you are also
   compiling unit tests for all other sub-components.
4. If you encounter errors linking to `libunify`, try redoing the [`libunify` compile steps](#build-libunify).
5. Encountering problems with the submodules can be due to trying to check out
   the submodules inside the docker container.
6. If the Unify Matter Bridge gets stuck while booting. Try to pass `--args="chip_config_network_layer_ble=false"` to `gn gen` command while building
