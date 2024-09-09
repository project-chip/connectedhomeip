# Matter Fabric Administrator Example

An example application that supports synchronization of devices across fabrics
fabrics.

---

-   [Building the Example Application](#building-the-example-application)

---

## Building the Example Application

For Linux host example:

```
source scripts/activate.sh
./scripts/build/build_examples.py --target linux-x64-fabric-admin-rpc build
```

For Raspberry Pi 4 example:

### Pull Docker Images

```
docker pull connectedhomeip/chip-build-vscode:latest
```

### Run docker

```
docker run -it -v ~/connectedhomeip:/var/connectedhomeip connectedhomeip/chip-build-vscode:latest /bin/bash
```

### Build

```
cd /var/connectedhomeip

git config --global --add safe.directory /var/connectedhomeip
git config --global --add safe.directory /var/connectedhomeip/third_party/pigweed/repo
git config --global --add safe.directory /var/connectedhomeip/examples/common/QRCode/repo

./scripts/run_in_build_env.sh \
 "./scripts/build/build_examples.py \
    --target linux-arm64-fabric-admin-clang-rpc \
    build"
```

### Transfer the fabric-admin binary to a Raspberry Pi

```
scp ./fabric-admin ubuntu@xxx.xxx.xxx.xxx:/home/ubuntu
```
