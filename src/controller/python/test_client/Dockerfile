FROM ubuntu:latest

# To override
ENV TZ=America/Los_Angeles
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Executed during Image build
USER root
RUN apt-get update

# Install Dependencies for setting up CHIP environment
RUN apt-get install -y apt-utils \
                       gcc \
                       g++ \
                       python \
                       pkg-config \
                       libssl-dev \
                       libdbus-1-dev \
                       libglib2.0-dev \
                       libavahi-client-dev \
                       libgirepository1.0-dev \
                       ninja-build \
                       python3-venv \
                       python3-dev \
                       python3-pip \
                       unzip

# Additional Dependencies for Chip device controller
RUN apt-get install -y autotools-dev \
                       automake \
                       libusb-dev \
                       libudev-dev \
                       libical-dev \
                       libreadline-dev \
                       libdbus-glib-1-dev \
                       libcairo2-dev \
                       libtool \
                       bluez \
                       bluetooth \
                       python3-gi \
                       python-is-python3 \
                       python3 \
                       pi-bluetooth \
                       avahi-daemon \
                       libavahi-client-dev \
                       build-essential \
                       protobuf-compiler \
                       wpasupplicant \
                       wireless-tools \
                       rfkill \
                       libcairo2-dev \
                       python3-widgetsnbextension \
                       python3-testresources \
                       isc-dhcp-client \
                       avahi-utils \
                       iproute2 \
                       iputils-ping

ADD requirements.txt .
RUN pip3 install -r requirements.txt --no-binary :all

# Wheel needs to be in the same dir as the dockerfile and have the following name
ADD chip-0.0-cp37-abi3-linux_aarch64.whl .
RUN pip3 install chip-0.0-cp37-abi3-linux_aarch64.whl
WORKDIR /chip_tool
 
# Switching to a non-root user, please refer to https://aka.ms/vscode-docker-python-user-rights
RUN useradd appuser && chown -R appuser /chip_tool
USER appuser

# Set PYTHONPATH
ENV PYTHONPATH="/usr/bin/python3"

# Executed when you start/launch a container
CMD ["chip-device-ctrl"]
