# Minimal mDNS example

This example demonstrates the multicast DNS (mDNS) protocol functionality in
Matter.

## Example server

The file `server.cpp` contains an example of a mdns server that can listen on
both IPv4 and IPv6 interfaces or mDNS queries.

To run, you can use:

```sh
./out/minimal_mdns/minimal-mdns-server -4
```

which will listen on both IPv4 and IPv6 addresses, on port 5353.

See

```sh
./out/minimal_mdns/minimal-mdns-server --help
```

for supported options.

The server will print out any queries as well as any advertisements it sees on
the network while running.

## Example client

The file `client.cpp` contains an example of a mdns client. By default it
queries for **\_services.\_dns-sd.\_udp.local** (DNS-SD list services) and uses
unicast queries.

Example run:

```sh
./out/minimal_mdns/minimal-mdns-client
```

which is likely to list a lot of answers.

You can customize the queries run:

```sh
./out/minimal_mdns/minimal-mdns-client -q chip-mdns-demo._matter._tcp.local
```

see

```sh
./out/minimal_mdns/minimal-mdns-client --help
```

for full command line details.

## Example advertiser

This example uses the chip built-in advertiser, implemented in the file
`advertiser.cpp`.

Example runs:

```sh
./out/minimal_mdns/mdns-advertiser -4 -m operational
```

```sh
./out/minimal_mdns/mdns-advertiser -4 -m commissionable-node --vendor-id 123 --product-id 456
```

```sh
./out/minimal_mdns/mdns-advertiser -4 -m commissioner --vendor-id 123 --product-id 456 --device-type 35
```

see

```sh
./out/minimal_mdns/mdns-advertiser --help
```

for full command line details.

## Testing with dns-sd

If you have a mac computer (or are able to install dns-sd via opkg), here are
some helpful examples on usage. More details are available in the CHIP spec on
discovery.

### Advertising to test client listings

#### Simulated uncommissioned node

```sh
dns-sd -R DD200C20D25AE5F7 _matterc._udp,S52,L840,V123 . 11111 D=840 VP=123+456
```

Will create the following records

```
_matterc._udp.local.                   PTR   DD200C20D25AE5F7._matterc._udp.local.
S52._sub._matterc._udp.local.          PTR   DD200C20D25AE5F7._matterc._udp.local.
L840._sub._matterc._udp.local.         PTR   DD200C20D25AE5F7._matterc._udp.local.
V123._sub._matterc._udp.local.         PTR   DD200C20D25AE5F7._matterc._udp.local.
DD200C20D25AE5F7._matterc._udp.local.  TXT   "D=840" "VP=123+456"
DD200C20D25AE5F7._matterc._udp.local.  SRV   0 0 11111 B75AFB458ECD.local.
B75AFB458ECD.local.                  AAAA  ba2a:b311:742e:b44c:f515:576f:9783:3f30
```

#### Simulated commissioning node

```sh
dns-sd -R DD200C20D25AE5F7 _matterd._udp,S52,L840,V123 . 11111 D=840 VP=123+456 PH=3
```

Will create the following records:

```
_matterd._udp.local.                   PTR   DD200C20D25AE5F7._matterd._udp.local.
S52._sub._matterd._udp.local.          PTR   DD200C20D25AE5F7._matterd._udp.local.
V123._sub._matterd._udp.local.         PTR   DD200C20D25AE5F7._matterd._udp.local.
D840._sub._matterd._udp.local.         PTR   DD200C20D25AE5F7._matterd._udp.local.
DD200C20D25AE5F7._matterd._udp.local.  TXT   "D=840" "VP=123+456" "PH=3"
DD200C20D25AE5F7._matterd._udp.local.  SRV   0 0 11111 B75AFB458ECD.local.
B75AFB458ECD.local.                    AAAA  ba2a:b311:742e:b44c:f515:576f:9783:3f30
```

#### Simulated commissioned node

```sh
dns-sd -R 2906C908D115D362-8FC7772401CD0696 _matter._tcp . 22222
```

### Discovery commands

Nodes:

```sh
dns-sd -B _matterc._udp         # Nodes awaiting commissioning
dns-sd -B _matterc._udp,S52     # Nodes awaiting commissioning with short discriminator 52
dns-sd -B _matterc._udp,L840    # Nodes awaiting commissioning with long discriminator 840
dns-sd -B _matterc._udp,V123    # Nodes awaiting commissioning with vendor id 123

dns-sd -B _matterd._udp         # Commissionable nodes
dns-sd -B _matterd._udp,S52     # Commissionable nodes with short discriminator 52
dns-sd -B _matterd._udp,L840    # Commissionable nodes with long discriminator 840
dns-sd -B _matterd._udp,V123    # Commissionable nodes with vendor id 123
```

IP Address:

```sh
dns-sd -L 2906C908D115D362-8FC7772401CD0696 _matter._tcp  # find server address
dns-sd -Gv6 B75AFB458ECD.local                            # get IPv6 address
```
