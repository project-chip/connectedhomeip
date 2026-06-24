### Image description

This image adds cross-compilation support files (a suitable sysroot) on top of
the regular linux chip-build image.

The build pulls a pre-packaged sysroot directory named
`ubuntu-24.04-aarch64-sysroot` directly from CIPD.

---

### Creating a Sysroot (Python script)

We provide a fully automated Python script (`create_sysroot.py`) to generate
clean cross-compilation sysroots directly on the host machine.

This tool is designed to work fast compared to full emulation by completely
**avoiding the need for full system emulation or virtualization** (which is
traditionally slow and brittle in restricted environments)

#### Prerequisites

Ensure `debootstrap` is installed on your host:

```bash
sudo apt-get install debootstrap
```

The script requires Python 3.11+ and depends on `click`, `coloredlogs`, and
`tqdm`.

You have two ways to run it:

1. **Using `uv` (Recommended)**: Install `uv` from Astral (e.g.,
   `curl -LsSf https://astral.sh/uv/install.sh | sh`). Running the script with
   `uv` will automatically fetch the required dependencies in an isolated
   environment.
2. **Using standard `python3`**: Ensure you have installed the required
   dependencies in your environment (e.g.,
   `pip install click coloredlogs tqdm`).

#### How to Run

You must specify the architecture (`arm64` or `armhf`) via the `-a` flag.

If running via `uv` (takes advantage of the shebang):

```bash
./create_sysroot.py -a arm64
```

If running directly via `python3`:

```bash
python3 create_sysroot.py -a arm64
```

This performs a full run and generates two directories:

1. `ubuntu-24.04-aarch64-full`: The complete minimal rootfs created by
   `debootstrap`.
2. `ubuntu-24.04-aarch64-sysroot`: The lean, final sysroot optimized for
   cross-compilation.

#### Fast Iteration (Skipping Stage 1)

The initial download of base packages takes time. Once you have run the script
once (creating the `-full` directory), you can iterate rapidly on the sysroot
generation phase by passing the **`-s`** flag to skip the slow Stage 1:

```bash
./create_sysroot.py -a arm64 -s
```

#### Packaging and Uploading

Once the sysroot is verified locally, package and upload it to CIPD using the
provided shell script:

```bash
# Upload to CIPD
./upload-cidp-package.sh -a arm64

# Or just generate the cipd.yaml locally without uploading
./upload-cidp-package.sh -a arm64 -c
```

---

### How the Python Script Works (Under the Hood)

Generating a cross-compilation sysroot cleanly without full emulation involves
solving several complex system challenges. Here is exactly what the script does
and why:

#### 1. Stage 1: Dependency Resolution (`debootstrap --foreign`)

The script executes `debootstrap` in foreign mode. This forces Ubuntu's package
tools to resolve all dependencies for the target architecture (including extra
libraries requested from `main` and `universe`) and download the correct `.deb`
binaries into an internal cache (`var/cache/apt/archives/`). Because it runs in
foreign mode, it only unpacks core base packages and skips the slow execution of
installation scripts inside emulation.

#### 2. Stage 2: Non-Destructive Manual Extraction

To install the remaining development headers, libraries, and `pkg-config` files,
the script avoids booting an emulated `chroot`. Instead, it iterates over the
downloaded `.deb` files, unpacks each one into a **clean temporary staging
directory** on the host, and safely merges the contents into the sysroot using
`rsync -a --keep-dirlinks`. This completely avoids destructive extraction
conflicts (such as `tar` unlinking system directories when packages contain
overlapping structure).

#### 3. Merged-Usr Pre-Creation

Modern Ubuntu relies on a merged-usr structure where `/bin`, `/sbin`, and `/lib`
are symlinks pointing to their corresponding directories in `/usr`. The script
pre-creates this exact structure before extraction begins. This ensures
extracted files merge flawlessly into `/usr` and preserves standard system
links.

#### 4. Absolute Symlink Fixups

Many libraries install symbolic links that point to absolute paths (e.g.,
`libfoo.so -> /usr/lib/libfoo.so.1`). When cross-compiling on a host, the
compiler prepends the sysroot path, making these absolute links invalid
(pointing to the host's `/usr/lib` instead of the sysroot `/usr/lib`). The
script traverses the entire sysroot, detects all absolute symlinks, and natively
converts them into **relative symlinks** (safe for sysroot relocation). It
explicitly skips special dynamic filesystems (`/proc`, `/sys`, `/dev`).

#### 5. Size Cleanup

A raw rootfs contains massive directories completely unneeded for user-space
application compilation, such as graphics drivers (`dri`), kernel modules
(`modules`), and firmware. The script explicitly deletes these massive folders,
reducing the final sysroot size from over 2.4GB down to a highly optimized
**~1.5GB**.

#### Creating a Sysroot

Start with a fresh Raspberry PI image:

-   Use the Raspberry pi imager from <https://www.raspberrypi.org/software/>
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

<https://chrome-infra-packages.appspot.com/p/experimental/matter/sysroot/ubuntu-22.04-aarch64/+/>

and can be downloaded using the `cipd` script from
[depot_tools](https://dev.chromium.org/developers/how-tos/depottools):

```
echo 'experimental/matter/sysroot/ubuntu-22.04-aarch64 latest' > ensure_file.txt
cipd ensure -ensure-file ensure_file.txt -root ./
```
