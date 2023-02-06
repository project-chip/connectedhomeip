# Matter Shell - App Server module

The all-clusters-app server may be invoked and managed via the Matter Shell CLI.

## Command List

-   [help](#help)
-   [clusters](#clusters)
-   [endpoints](#endpoints)
-   [port](#port)
-   [sessions](#sessions)
-   [start](#start)
-   [stop](#stop)
-   [udcport](#udcport)

## Command Details

### help

List the Server CLI commands.

```bash
> server help
  help            Usage: server <subcommand>
  start           Start the ZCL application server.
  stop            Stop the ZCL application server.
  port            Get/Set operational port of server.
  udcport         Get/Set commissioning port of server.
  sessions        Manage active sessions on the server.
  exchanges       Manage active exchanges on the server.
  endpoints       Display endpoints on the server.
  clusters        Display clusters on the server.
Done
```

### clusters

Displays all clusters in endpoint hierarchy.

```bash
> server clusters
Endpoint 0:
  - Cluster 0x0003
  - Cluster 0x0004
  - Cluster 0x001D
  - Cluster 0x001E
  - Cluster 0x001F
  - Cluster 0x0028
  - Cluster 0x0029
  - Cluster 0x002A
  - Cluster 0x002E
  - Cluster 0x0030
  - Cluster 0x0031
  - Cluster 0x0032
  - Cluster 0x0033
  - Cluster 0x0034
  - Cluster 0x0035
  - Cluster 0x0036
  - Cluster 0x0037
  - Cluster 0x003C
  - Cluster 0x003E
  - Cluster 0x003F
  - Cluster 0x0040
  - Cluster 0x0041
  - Cluster 0x0405
Endpoint 1:
  - Cluster 0x0003
  - Cluster 0x0004
  - Cluster 0x0005
  - Cluster 0x0006
  - Cluster 0x0007
  - Cluster 0x0008
  - Cluster 0x000F
  - Cluster 0x001D
  - Cluster 0x001E
  - Cluster 0x0025
  - Cluster 0x002F
  - Cluster 0x0039
  - Cluster 0x003B
  - Cluster 0x0040
  - Cluster 0x0041
  - Cluster 0x0045
  - Cluster 0x0050
  - Cluster 0x0101
  - Cluster 0x0102
  - Cluster 0x0103
  - Cluster 0x0200
  - Cluster 0x0201
  - Cluster 0x0204
  - Cluster 0x0300
  - Cluster 0x0400
  - Cluster 0x0402
  - Cluster 0x0403
  - Cluster 0x0404
  - Cluster 0x0405
  - Cluster 0x0406
  - Cluster 0x0500
  - Cluster 0x0503
  - Cluster 0x0504
  - Cluster 0x0505
  - Cluster 0x0506
  - Cluster 0x0507
  - Cluster 0x0508
  - Cluster 0x0509
  - Cluster 0x050A
  - Cluster 0x050B
  - Cluster 0x050C
  - Cluster 0x050D
  - Cluster 0x050E
  - Cluster 0x050F
  - Cluster 0x0B04
Endpoint 2:
  - Cluster 0x0004
  - Cluster 0x0006
  - Cluster 0x001D
  - Cluster 0x0406
Done
```

### endpoints

Displays all endpoints in device hierarchy.

```bash
> server endpoints
Endpoint 0
Endpoint 1
Endpoint 2
Done
```

### port

Display the current operational port for the server node.

```bash
> server port
5540
Done
```

### port \<udp_port\>

Sets the operational port to the given value. NOTE: server must be restarted to
take effect.

-   udp_port: new value to set operational port to

```bash
> server port 5541
Done
```

### sessions

Displays active session handles.

```bash
> server sessions
session id=0x0002 peerSessionId=0x0012 peerNodeId=0x000000000001b669 fabricIdx=1
Done
```

### start

Start the App Server on the Node. This also starts the commissioning window as
is done with the all-clusters-app.

```bash
> server start
...
[1639549415.105682][1468836:1468841] CHIP:SVR: Server Listening...
Done
```

### stop

Stops the App Server, closes all related sockets, and frees resources.

```bash
> server stop
Done
```

### udcport

Display the current commissioning port for the server node.

```bash
> server udcport
5550
Done
```

### udcport \<udp_port\>

Sets the commissioning port to the given value. NOTE: server must be restarted
to take effect.

-   udp_port: new value to set commissioning port to

```bash
> server udcport 5551
Done
```
