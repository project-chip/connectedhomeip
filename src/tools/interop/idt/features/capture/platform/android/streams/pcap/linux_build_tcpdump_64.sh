#!/bin/bash
#
#    Copyright (c) 2023 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

set -e
export TCPDUMP=4.99.4
export LIBPCAP=1.10.4

wget https://www.tcpdump.org/release/tcpdump-"$TCPDUMP".tar.gz
wget https://www.tcpdump.org/release/libpcap-"$LIBPCAP".tar.gz

tar zxvf tcpdump-"$TCPDUMP".tar.gz
tar zxvf libpcap-"$LIBPCAP".tar.gz
export CC=aarch64-linux-gnu-gcc
cd libpcap-"$LIBPCAP"
./configure --host=arm-linux --with-pcap=linux
make
cd ..

cd tcpdump-"$TCPDUMP"
export ac_cv_linux_vers=2
export CFLAGS=-static
export CPPFLAGS=-static
export LDFLAGS=-static

./configure --host=arm-linux
make

aarch64-linux-gnu-strip tcpdump
cp tcpdump ..
cd ..
rm -R libpcap-"$LIBPCAP"
rm -R tcpdump-"$TCPDUMP"
rm libpcap-"$LIBPCAP".tar.gz
rm tcpdump-"$TCPDUMP".tar.gz
