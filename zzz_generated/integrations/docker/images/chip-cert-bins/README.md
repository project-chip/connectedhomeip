# Docker image for Matter Certification Test Harness

The Dockerfile here helps build multi-platform Docker images containing the
executable binaries necessary for the Matter Test Harness. It utilizes the
BuildKit toolkit and Buildx, included within Docker since version 18.06.

## Running

In order to properly run the binaries, avahi must be properly set up and passed
to the container.

Prerequisites:

-   Host must support and enable IPv6 and be on a network that has IPv6.
-   IPv6 must be enabled within avahi config on the host. `use-ipv6=yes` in
    avahi-daemon.conf
-   Sometimes there are stale avahi entries, so restarting avahi-daemon between
    runs may be necessary.

The host network and dbus must be exposed to the container for avahi to work
properly. So for an interactive prompt, use:

```
docker run -it --network host -v /var/run/dbus/system_bus_socket:/var/run/dbus/system_bus_socket chip-cert-bins
```

## Building

The Dockerfile requires building using the Buildx plugin, included within
docker. It is used to build for both the amd64 and arm64 architectures, so the
image may be cross-built and ran directly on a Raspberry Pi or other arm64 based
environment. If your docker installation does not have the Buildx plugin, please
update docker or install Buildx manually.

Prerequisites:

-   A recent docker installation.
-   Create a Buildx builder: `docker buildx create --use --name mybuild`
-   Install the Binfmt cross-platform Docker emulators:
    `docker run --privileged --rm tonistiigi/binfmt --install all`

### Example: Building for the host platform and loading into Docker

```
docker buildx build --load .
```

The above command will build the image and load them into your local Docker
instance.

### Example: Building for another platform and exporting to a tar

```
docker buildx build --platform linux/arm64 --output "dest=/full/path/to/dest/chipcertbins.tar,type=docker" .
```

The above command will build the image and export it to a tar file. You may copy
the tar file to a RaspberryPi and import the image by using:

```
docker load --input chipcertbins.tar
```

### Example: Creating a multi-platform image and pushing to the Docker registry

```
docker buildx build --platform linux/amd64,linux/arm64 --tag chip-cert-bins:tag1 --push .
```
