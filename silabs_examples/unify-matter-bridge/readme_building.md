# Unify Matter Bridge

The Unify Matter Bridge bridges the Matter ecosystem with the Unify ecosystem.

## Building the Unify Matter Bridge

This building guide cross compiles for armhf architecture.

Pay attention to if the command should be executed on the host machine or inside
of the docker build environment. A `docker/<dir>$` means inside the docker and
docker left out means on your host machine.


### Checkout submodules

For the Unify bridge we checkout the necessary submodules with the below
command.

```bash
matter$ ./scripts/checkout_submodules.py --platform linux
```

### Downloading and staging the uic repo

```bash
matter$ git clone --depth 1 --branch ver_1.2.1-103-g34db9516-unify-matter-bridge ssh://git@stash.silabs.com/uic/uic.git  --recursive ../uic
```

#### Build the docker container (armhf compilation)

```bash
matter$ docker build -t unify-matter silabs_examples/unify-matter-bridge/docker/
```

Starting the docker:

```bash
matter$ docker run -it -v $PWD:/matter -v $PWD/../uic:/uic unify-matter
```

#### Building libunify

The Unify Matter Bridge is depending on the libunify library from the Unify
project.

This library must first be compiled for the target system

```bash
docker/uic$ cmake -DCMAKE_INSTALL_PREFIX=$PWD/stage -GNinja -DCMAKE_TOOLCHAIN_FILE=../cmake/armhf_debian.cmake  -B build_unify_armhf/ -S components
docker/uic$ cmake --build build_unify_armhf
docker/uic$ cmake --install build_unify_armhf --prefix $PWD/stage
```

After having cross compiled unify library we set two paths in the
PKG_CONFIG_PATH. The first path is for the staged unify library and the second
one is for cross compiling to armhf.

```bash
docker/uic$ export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$PWD/stage/share/pkgconfig
docker/uic$ export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/lib/arm-linux-gnueabihf/pkgconfig
```

### Setup the matter build environment

Now setup the matter environment.

After having all the necessary submodules source the environment of matter with
the below command. The source command will load a number of build tools, and
will make sure the correct toolchains and compilers are used for compiling the
unify matter bridge.

```bash
docker$ source ./scripts/activate.sh
```

Compile the Unify bridge

```bash
docker/matter$ cd silabs_examples/unify-matter-bridge/linux/
docker/silabs_examples/unify-matter-bridge/linux$ gn gen out/armhf --args='target_cpu="arm"'
docker/silabs_examples/unify-matter-bridge/linux$ ninja -C out/armhf
```

After building the unify-matter-bridge binary is located at
`out/armhf/obj/bin/unify-matter-bridge`.

### Unit testing

It is always preferable to write unit testing for quality software. So we
encourage to do this when developing for the Unify Matter Bridge.

Documentation on unit testing can be found within the
[README.md](linux/src/tests/README.md) of the `linux/src/tests` folder.

### Trouble shooting

Common errors one might encounter:

1. If you did not source the `matter/scripts/activate.sh` `gn` and other common
   build tools won't be found.
2. If you did not export the `pkgconfig` for the `arm-linux-gnueabihf` toolchain
   you will get errors such as `G_STATIC_ASSERT(sizeof (unsigned long long) == sizeof (guint64));`
3. If you are compiling unit tests do not try to compile the matter bridge at
   the same time. This will not work as when compiling unit tests you are also
   compiling unit tests for all other sub components.
4. If you are encountering errors linking to `libunify` try to redo the compile
   steps of the `libunify`.
5. Encountering problems with the submodules can be due to trying to checkout
   the submodules inside of the docker container.
