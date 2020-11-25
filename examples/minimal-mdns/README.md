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

The file -client.cpp` contains an example of a mdns client. By default it
queries for **\_services.\_dns-sd.\_udp.local** (DNS-SD list services) and uses
unicast queries.

Example run:

```sh
./out/minimal_mdns/minimal-mdns-client -4
```

which is likely to list a lot of answers.

You can customize the queries run:

```sh
/out/minimal_mdns/minimal-mdns-client -4 -q chip-mdns-demo._chip._tcp.local
```

see

```sh
./out/minimal_mdns/minimal-mdns-client --help
```

for full command line details.

## Testing with dns-sd

If you have a mac computer (or are able to install dns-sd via opkg), here are
some helpful examples on usage. More details are available in the CHIP spec on
discovery.

### Advertising to test client listings

#### Simulated uncommisioned node

```sh
dns-sd -R DD200C20D25AE5F7 _chipc._udp,S052,L0840,V123 . 11111 D=0840 VP=123+456
```

Will create the following records

```
_chipc._udp.local.                   PTR   DD200C20D25AE5F7._chipc._udp.local.
S052._sub._chipc._udp.local.         PTR   DD200C20D25AE5F7._chipc._udp.local.
L0840._sub._chipc._udp.local.        PTR   DD200C20D25AE5F7._chipc._udp.local.
V123._sub._chipc._udp.local.         PTR   DD200C20D25AE5F7._chipc._udp.local.
DD200C20D25AE5F7._chipc._udp.local.  TXT   "D=0840" "VP=123+456"
DD200C20D25AE5F7._chipc._udp.local.  SRV   0 0 11111 B75AFB458ECD.local.
B75AFB458ECD.local.                  AAAA  ba2a:b311:742e:b44c:f515:576f:9783:3f30
```

#### Simulated commisionable node

```sh
dns-sd -R DD200C20D25AE5F7 _chipd._udp,S052,L0840,V123 . 11111 D=0840 VP=123+456 PH=3
```

Will create the following records:

```
_chipd._udp.local.              PTR   DD200C20D25AE5F7._chipd._udp.local.
S052._sub._chipd._udp.local.    PTR   DD200C20D25AE5F7._chipd._udp.local.
V123._sub._chipd._udp.local.    PTR   DD200C20D25AE5F7._chipd._udp.local.
D0840._sub._chipd._udp.local.   PTR   DD200C20D25AE5F7._chipd._udp.local.
DD200C20D25AE5F7._chipd._udp.local.  TXT   "D=0840" "VP=123+456" "PH=3"
DD200C20D25AE5F7._chipd._udp.local.  SRV   0 0 11111 B75AFB458ECD.local.
B75AFB458ECD.local.             AAAA  ba2a:b311:742e:b44c:f515:576f:9783:3f30
```

#### Simulated commisioned node

```sh
dns-sd -R 2906C908D115D362-8FC7772401CD0696 _chip._tcp . 22222
```

### Discovery commands

Nodes:

```sh
dns-sd -B _chipc._udp          # Nodes awaiting commisioning
dns-sd -B _chipc._udp,S052     # Nodes awaiting commisioning with short discriminator 052
dns-sd -B _chipc._udp,L0840    # Nodes awaiting commisioning with long discriminator 0840
dns-sd -B _chipc._udp,V123     # Nodes awaiting commisioning with vendor id 123

dns-sd -B _chipd._udp          # Commisionable nodes
dns-sd -B _chipd._udp,S052     # Commisionable nodes with short discriminator 052
dns-sd -B _chipd._udp,L0840    # Commisionable nodes with long discriminator 0840
dns-sd -B _chipd._udp,V123     # Commisionable nodes with vendor id 123
```

IP Address:

```sh
dns-sd -L 2906C908D115D362-8FC7772401CD0696 _chip._tcp  # find server address
dns-sd -Gv6 B75AFB458ECD.local                          # get IPv6 address
```
