FROM debian:bookworm

# deps
RUN apt-get update
RUN apt-get install -y apt-utils git wget
# python
RUN apt-get install -y python3.11 python3-pip python3.11-venv
# pybluez
RUN apt-get install -y pkg-config glib-2.0 libboost-python-dev libboost-thread-dev libbluetooth-dev libglib2.0-dev

# adb
RUN apt-get install -y usbutils adb
RUN usermod -aG plugdev $(whoami)
# for pcap
RUN apt-get install -y net-tools pciutils kmod
RUN apt-get install -y aircrack-ng
RUN apt-get install -y tcpdump
RUN echo "wireshark-common wireshark-common/install-setuid boolean true" | debconf-set-selections
RUN DEBIAN_FRONTEND=noninteractive apt-get -y install wireshark
# otbr
WORKDIR /
RUN git clone https://github.com/openthread/ot-br-posix

# setup
WORKDIR /
# otbr (not used right yet)
# cd /ot-br-posix && ./script/bootstrap && INFRA_IF_NAME=eth0 ./script/setup
# python deps
RUN python3 -m venv /env
ADD requirements.txt /
RUN /bin/bash -c "source /env/bin/activate && pip install -r requirements.txt"
RUN wget https://github.com/pybluez/pybluez/archive/master.tar.gz -O master.tar.gz
RUN tar -xvf master.tar.gz
WORKDIR pybluez-master
RUN /bin/bash -c "source /env/bin/activate && python setup.py install"
RUN /bin/bash -c "source /env/bin/activate && pip install -e .[ble]"
WORKDIR /

# env
RUN echo "source /env/bin/activate; pip -V; source /idt/scripts/alias.sh;" >> /root/.bashrc
ENTRYPOINT ["/bin/bash"]
