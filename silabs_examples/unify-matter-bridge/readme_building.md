# Building the Unify Matter Bridge

Pay attention to if the command should be executed on the host machine or inside
of the docker build environment. A `docker/<dir>$` means inside the docker and
docker left out means on your host machine.

# Download the uic repo

```bash
matter$ git clone ssh://git@stash.silabs.com/uic/uic.git  --recursive ../uic
```

# Build the docker container

```bash
matter$ docker build -t unify-matter silabs_examples/unify-matter-bridge/docker/
```

Starting the docker:

```bash
matter$ docker run -it -v $PWD:/matter -v $PWD/../uic:/uic unify-matter
```

# Buiding libunify

The Unify Matter Bridge is depending on the libunify library from the Unify
project.

This library must first be compiled for the target system

```bash
docker/uic$ cmake -DCMAKE_INSTALL_PREFIX=$PWD/stage -GNinja  -B unify_build/ -S components
docker/uic$ cmake --build unify_build
docker/uic$ cmake --install unify_build --prefix $PWD/stage
```

Setup pkg-config to look at the stage folder

```bash
docker/uic$ export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$PWD/stage/share/pkgconfig
```

# Setup the matter build environment

Now setup the matter environment.

Checking out submodules in the matter repo needs to be done on the host machine.

```bash
matter$ ./scripts/checkout_submodules.py --platform linux
```

After having all the necessary submodules source the environment of matter with
the below command. Be aware that it will show an error but disregard this.

```bash
docker$ ./scripts/build/gn_bootstrap.sh
```

Compile the Unify bridge

```bash
docker$ cd silabs_examples/unify-matter-bridge/linux/
docker$ gn gen out/host
docker$ ninja -C out/host
```
