### Image description

This image adds cross-compilation support files (a suitable sysroot) on top of
the regular linux chip-build image.

The build assumes a file named `ubuntu-22.04-aarch64-sysroot.tar.xz` exists in
the current directory. This can generally be manually built using an existing
Raspberry Pi device (or equivalent qemu) and a convenience copy was created in
CIPD

#### Creating a Sysroot (qemu virtual emulator)

NOTE: this approach is slower due to emulation usage, however has the advantage
of not requiring separate hardware.

Ensure `qemu` and `virt-install` prerequisites are met:

```
apt-get install qemu-system-arm virtinst libvirt-daemon
```

Start up the sysroot virtual machine. This will be named `sysrootsrv`:

```
./start-sysroot-vm.sh
```

Connect to a console to monitor progress. The VM is configured to pre-install
packages based on CHIP BUILDING.md documentation plus any additional packages.

```
virsh console sysrootsrv
```

Once installation completed, you can also login as `ubuntu/1234` or use
`ssh ubuntu@localhost -p 5555`

The current image is based on Ubuntu 24.04, so you can create an image via:

```
rsync -avL -e 'ssh -p 5555' ubuntu@localhost:/lib ubuntu-24.04-aarch64-sysroot
rsync -avL -e 'ssh -p 5555' ubuntu@localhost:/usr/lib ubuntu-24.04-aarch64-sysroot/usr
rsync -avL -e 'ssh -p 5555' ubuntu@localhost:/usr/include ubuntu-24.04-aarch64-sysroot/usr
```

At this point you have a sysroot in `ubuntu-24.04-aarch64-sysroot`

To create/upload a CIPD package tagged with the current date, a script is
provided:

```
upload-cipd-package.sh
```

#### Creating a Sysroot

Start with a fresh Raspberry PI image:

-   Use the Raspberry pi imager from https://www.raspberrypi.org/software/
-   Follow installation instructions from the CHIP BUILDING.md document.
    Generally this includes:
    -   Use Ubuntu 22.04 server 64 bit server image
    -   Install required compile dependencies via `apt-get`. You may skip
        installing git and python3 dependencies to save some image size

Generate a sysroot. You can do this locally (slow due to SD performance):

```
rsync -avL /lib ubuntu-22.04-aarch64-sysroot/
rsync -avL /usr/lib ubuntu-22.04-aarch64-sysroot/usr
rsync -avL /usr/include ubuntu-22.04-aarch64-sysroot/usr
```

or you can copy directly to your host machine via

```
export PI="ip-of-raspberry-pi"
ssh-copy-id ubuntu@$PI

rsync -avL ubuntu@$PI:/lib ubuntu-22.04-aarch64-sysroot
rsync -avL ubuntu@$PI:/usr/lib ubuntu-22.04-aarch64-sysroot/usr
rsync -avL ubuntu@$PI:/usr/include ubuntu-22.04-aarch64-sysroot/usr
```

NOTE: in the future, if creating a 32-bit image (not covered by this docker
image yet), the following symlinks are required:

-   `usr/lib/arm-linux-gnueabihf` to `usr/lib/armv7-linux-gnueabihf`
-   `usr/include/arm-linux-gnueabihf` to `usr/lib/armv7-linux-gnueabihf`

Once the sysroot is on the host machine, create the corresponding `tar.xz` file:

```
tar cvfJ ubuntu-22.04-aarch64-sysroot.tar.xz ubuntu-22.04-aarch64-sysroot
```

#### CIPD image

CIPD image is provided as a convenience to avoid the need to spend time
rebuilding a sysroot. It is located at:

https://chrome-infra-packages.appspot.com/p/experimental/matter/sysroot/ubuntu-22.04-aarch64/+/

and can be downloaded using the `cipd` script from
[depot_tools](https://dev.chromium.org/developers/how-tos/depottools):

```
echo 'experimental/matter/sysroot/ubuntu-22.04-aarch64 latest' > ensure_file.txt
cipd ensure -ensure-file ensure_file.txt -root ./
```
