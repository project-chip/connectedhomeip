### Image description

This image adds cross-compilation support files (a suitable sysroot) on top of
the regular linux chip-build image.

The build assumes a file named `ubuntu-21.04-aarch64-sysroot.tar.xz` exists in
the current directory. This can generally be manually built using an existing
Raspberry Pi device (or equivalent qemu) and a convenience copy was created in
CIPD

#### Creating a Sysroot

Start with a fresh Raspberry PI image:

-   Use the Raspberry pi imager from https://www.raspberrypi.org/software/
-   Follow installation instructions from the CHIP BUILDING.md document.
    Generally this includes:
    -   Use Ubuntu 21.04 server 64 bit server image
    -   Install required compile dependencies via `apt-get`. You may skip
        installing git and python3 dependencies to save some image size

Generate a sysroot. You can do this locally (slow due to SD performance):

```
rsync -avL /lib ubuntu-21.04-aarch64-sysroot/
rsync -avL /usr/lib ubuntu-21.04-aarch64-sysroot/usr
rsync -avL /usr/include ubuntu-21.04-aarch64-sysroot/usr
```

or you can copy directly to your host machine via

```
export PI="ip-of-raspberry-pi"
ssh-copy-id ubuntu@$PI

rsync -avL ubuntu@$PI:/lib ubuntu-21.04-aarch64-sysroot
rsync -avL ubuntu@$PI:/usr/lib ubuntu-21.04-aarch64-sysroot/usr
rsync -avL ubuntu@$PI:/include/lib ubuntu-21.04-aarch64-sysroot/usr
```

NOTE: in the future, if creating a 32-bit image (not covered by this docker
image yet), the following symlinks are required:

-   `usr/lib/arm-linux-gnueabihf` to `usr/lib/armv7-linux-gnueabihf`
-   `usr/include/arm-linux-gnueabihf` to `usr/lib/armv7-linux-gnueabihf`

Once the sysroot is on the host machine, create the corresponding `tar.xz` file:

```
tar cvfJ ubuntu-21.04-aarch64-sysroot.tar.xz ubuntu-21.04-aarch64-sysroot
```

#### CIPD image

CIPD image is provided as a convenience to avoid the need to spend time
rebuilding a sysroot. It is located at:

https://chrome-infra-packages.appspot.com/p/experimental/matter/sysroot/ubuntu-21.04-aarch64/+/

and can be downloaded using the `cipd` script from
[depot_tools](https://dev.chromium.org/developers/how-tos/depottools):

```
echo 'experimental/matter/sysroot/ubuntu-21.04-aarch64 latest' > ensure_file.txt
cipd ensure -ensure-file ensure_file.txt -root ./
```
