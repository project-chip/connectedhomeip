# Access Control

<a href="http://35.236.121.59/hub/user-redirect/git-pull?repo=https%3A%2F%2Fgithub.com%2Fproject-chip%2Fconnectedhomeip&urlpath=lab%2Ftree%2Fconnectedhomeip%2Fdocs%2Fguides%2Frepl%2FMatter%2520-%2520Access%2520Control.ipynb&branch=master">
<img src="https://i.ibb.co/hR3yWsC/launch-playground.png" alt="drawing" width="130"/>
</a>
<br></br>

This document explains how to use Access Control in Matter, and will be updated
as development proceeds.

## What Does and Doesn’t Work Right Now?

Briefly, you can read and write the entire ACL attribute in the
all-clusters-app, but Access Control isn’t yet turned on, so it won’t affect
interactions. There’s almost no error checking when writing the ACL attribute
(e.g. ensuring subjects match auth mode, only your fabric can be written, etc.)
so exercise caution for now.

## Clear Persisted Storage

Let's clear out our persisted storage (if one exists) to start from a clean
slate.

```python
import os, subprocess

if os.path.isfile('/tmp/repl-storage.json'):
    os.remove('/tmp/repl-storage.json')

# So that the all-clusters-app won't boot with stale prior state.
os.system('rm -rf /tmp/chip_*')
```

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace"><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">0</span>
</pre>

## Initialization

Let's first begin by setting up by importing some key modules that are needed to
make it easier for us to interact with the Matter stack.

> **NOTE**: _This is not needed if you launch the REPL from the command-line._

```python
import chip.native
import pkgutil
module = pkgutil.get_loader('chip.ChipReplStartup')
%run {module.path}
```

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace"><span style="color: #00ff00; text-decoration-color: #00ff00">──────────────────────────────────────── </span>Matter REPL<span style="color: #00ff00; text-decoration-color: #00ff00"> ────────────────────────────────────────</span>
</pre>

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace">

<span style="color: #000080; text-decoration-color: #000080; font-weight: bold">    </span>
<span style="color: #000080; text-decoration-color: #000080; font-weight: bold">            Welcome to the Matter Python REPL!</span>
<span style="color: #000080; text-decoration-color: #000080; font-weight: bold">    </span>
<span style="color: #000080; text-decoration-color: #000080; font-weight: bold">            For help, please type </span><span style="color: #008000; text-decoration-color: #008000; font-weight: bold">matterhelp()</span>
<span style="color: #000080; text-decoration-color: #000080; font-weight: bold">    </span>
<span style="color: #000080; text-decoration-color: #000080; font-weight: bold">            To get more information on a particular object/class, you can pass</span>
<span style="color: #000080; text-decoration-color: #000080; font-weight: bold">            that into </span><span style="color: #008000; text-decoration-color: #008000; font-weight: bold">matterhelp()</span><span style="color: #000080; text-decoration-color: #000080; font-weight: bold"> as well.</span>
<span style="color: #000080; text-decoration-color: #000080; font-weight: bold">    </span>
<span style="color: #000080; text-decoration-color: #000080; font-weight: bold">            </span>
</pre>

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace"><span style="color: #00ff00; text-decoration-color: #00ff00">─────────────────────────────────────────────────────────────────────────────────────────────</span>
</pre>

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace">

</pre>

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace"><span style="color: #af00ff; text-decoration-color: #af00ff">Restoring FabricAdmin from storage to manage FabricId </span><span style="color: #af00ff; text-decoration-color: #af00ff; font-weight: bold">1</span><span style="color: #af00ff; text-decoration-color: #af00ff">, FabricIndex </span><span style="color: #af00ff; text-decoration-color: #af00ff; font-weight: bold">1</span><span style="color: #af00ff; text-decoration-color: #af00ff">...</span>
</pre>

    New FabricAdmin: FabricId: 1(1)

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace">
<span style="color: #000080; text-decoration-color: #000080">Fabric Admins have been loaded and are available at </span><span style="color: #800000; text-decoration-color: #800000">fabricAdmins</span>
</pre>

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace">

</pre>

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace"><span style="color: #af00ff; text-decoration-color: #af00ff">Creating default device controller on fabric </span><span style="color: #af00ff; text-decoration-color: #af00ff; font-weight: bold">1</span><span style="color: #af00ff; text-decoration-color: #af00ff">...</span>
</pre>

    Allocating new controller with FabricId: 1(1), NodeId: 1

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace">

<span style="color: #000080; text-decoration-color: #000080">Default CHIP Device Controller has been initialized to manage </span><span style="color: #800000; text-decoration-color: #800000; font-weight: bold">fabricAdmins[</span><span style="color: #800000; text-decoration-color: #800000; font-weight: bold">0</span><span style="color: #800000; text-decoration-color: #800000; font-weight: bold">]</span><span style="color: #000080; text-decoration-color: #000080; font-weight: bold">, and is </span>
<span style="color: #000080; text-decoration-color: #000080; font-weight: bold">available as </span><span style="color: #800000; text-decoration-color: #800000; font-weight: bold">devCtrl</span>
</pre>

## Commission and Setup Server

### Launch Server

Let's launch an instance of the `chip-all-clusters-app`.

```python
import time, os
import subprocess
os.system('pkill -f chip-all-clusters-app')
time.sleep(1)

# The location of the all-clusters-app in the cloud playground is one level higher - adjust for this by testing for file presence.
if (os.path.isfile('../../../out/debug/chip-all-clusters-app')):
    appPath = '../../../out/debug/chip-all-clusters-app'
else:
    appPath = '../../../../out/debug/chip-all-clusters-app'

process = subprocess.Popen(appPath, stdout=subprocess.DEVNULL)
time.sleep(1)
```

### Commission Target

Commission the target with a NodeId of 1.

```python
devCtrl.CommissionIP(b'127.0.0.1', 20202021, 2)
```

    2022-01-29 16:01:43 johnsj-macbookpro1.roam.corp.google.com chip.SC[9915] ERROR The device does not support GetClock_RealTimeMS() API. This will eventually result in CASE session setup failures.


    Node address has been updated
    Commissioning complete

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace"><span style="color: #00ff00; text-decoration-color: #00ff00; font-style: italic">True</span>
</pre>

## Bootstrap ACLs

(For now) normally after commissioning there would be at least a single admin
entry, but currently the ACL will be empty, so add that entry manually. This
step will be removed later when it’s no longer necessary.

```python
await devCtrl.ReadAttribute(2, [ (0, Clusters.OperationalCredentials)], True)
```

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace">
<span style="font-weight: bold">{</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">0</span>: <span style="font-weight: bold">{</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">class</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008000; text-decoration-color: #008000">'chip.clusters.Objects.OperationalCredentials'</span><span style="font-weight: bold">&gt;</span>: <span style="color: #800080; text-decoration-color: #800080; font-weight: bold">OperationalCredentials</span><span style="font-weight: bold">(</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="color: #808000; text-decoration-color: #808000">NOCs</span>=<span style="font-weight: bold">[</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   </span><span style="color: #800080; text-decoration-color: #800080; font-weight: bold">NOCStruct</span><span style="font-weight: bold">(</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">fabricIndex</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">1</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">noc</span>=<span style="color: #008000; text-decoration-color: #008000">b'\x150\x01\x01\x01$\x02\x017\x03$\x13\x01$\x15\x01\x18&amp;\x04\x80"\x81\'&amp;\x05\x80%M:7\x06$\x11\x02$\x15\x01\x18$\x07\x01$\x08\x010\tA\x04\x19\x1ef\xaa\x8d_A\x19\x9f\xee\xa1\x1fn\x82\xb6}(\xd6\x00\'\xeb\x80Y\xf9\xd7\xa8\xbe\x98\xce\xba4v\xa0\xa0\xa9\xbaUO)?\xbb\xe0\xa4\x12r\x0cb\xe0\xc8\xa7r\xe3\xd5\x8e\x159\xc6\xaf\xc8\xbc\xb4\x8b\xf9E7\n5\x01(\x01\x18$\x02\x016\x03\x04\x02\x04\x01\x180\x04\x14\x1f\x87b\xe9`2\x03C{o`\x9e\x14\xe3\x8c\x0b\x83\xcd\x10Z0\x05\x14p\xfc\xf9\nFI\xba9\xb6SH\xcb\xd0meg\n\t\\=\x180\x0b@\xd3\x03x&lt;\x0f\xdc\xceBl\x01\xfb[Kly+\xd4\xab\xa08\xd3f \xa5\xb9\xb4aP\xce|\xee"to\x17\xc1m\xcfQ\x10pq=7\r\xf6\xe4\x89+\xcb\x98\xa9\x176T\x82\x83\x8d\x04#\xa2\xd3\xf2\xb6\x18'</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">icac</span>=<span style="color: #008000; text-decoration-color: #008000">b'\x150\x01\x01\x00$\x02\x017\x03$\x14\x00$\x15\x01\x18&amp;\x04\x80"\x81\'&amp;\x05\x80%M:7\x06$\x13\x01$\x15\x01\x18$\x07\x01$\x08\x010\tA\x04,\x0c\x82d\xa8\xd8-o\x860Q\xeb\x8d\x87\xe6x\x9e\x0b\xf8\xc91\xa9\xc5\x01nxB\x17h\xbc\x98]\xc9&amp;\x19\x9f\xde\x97&amp;\x80M\xca\x8b\xa1\xa0g\xfd\xae}\x12\x8a\x98\x08\x86k\xc7=\xc3\xea\x0e\xb0\xf0p\x057\n5\x01)\x01\x18$\x02`0\x04\x14p\xfc\xf9\nFI\xba9\xb6SH\xcb\xd0meg\n\t\\=0\x05\x14~\xf0\n\'\'\x89\xc26\xa8\xdaz\xbeCv\xb2\x7fn\xca^\xb5\x180\x0b@\x11\xa5\xf9\x80\x1a\x9c\xe5\xbf\x1e\xb3\n\x83\x0eW6!\x0f\xb0,\xa7\xdb\xb0\xe3B\xdc\\\xec\xbb\x02\xc2\x04\xd9\x0c\xa7\xdfj\x1a\x15]\x18\x08\xdfFz%)b\xfa\x9b\xca\x99\xccY\x01\xa7}\xf1|\xe6\xfe\xf0*\x10W\x18'</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   </span><span style="font-weight: bold">)</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="font-weight: bold">]</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="color: #808000; text-decoration-color: #808000">fabricsList</span>=<span style="font-weight: bold">[</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   </span><span style="color: #800080; text-decoration-color: #800080; font-weight: bold">FabricDescriptor</span><span style="font-weight: bold">(</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">fabricIndex</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">1</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">rootPublicKey</span>=<span style="color: #008000; text-decoration-color: #008000">b'\x04\xa2\x88}\xbd\xdb\xd4\xbc^U\xa3\xcef\xf7\x92\xbe\xa7X;E\xb8-Y\x12^$\xe0\x1bS\xf9\x9a\x8f\xacvj\xfa!&amp;m6\x00\xe3\xd1\x85o\x1b\x9eH\xabC?\xd8\xec\x08lH\xb5X\x1f:u\xb0\xfbj\x98'</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">vendorId</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">31968</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">fabricId</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">1</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">nodeId</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">2</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">label</span>=<span style="color: #008000; text-decoration-color: #008000">''</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   </span><span style="font-weight: bold">)</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="font-weight: bold">]</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="color: #808000; text-decoration-color: #808000">supportedFabrics</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">16</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="color: #808000; text-decoration-color: #808000">commissionedFabrics</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">1</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="color: #808000; text-decoration-color: #808000">trustedRootCertificates</span>=<span style="font-weight: bold">[</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   </span><span style="color: #008000; text-decoration-color: #008000">b'\x150\x01\x01\x00$\x02\x017\x03$\x14\x00$\x15\x01\x18&amp;\x04\x80"\x81\'&amp;\x05\x80%M:7\x06$\x14\x00$\x15\x01\x18$\x07\x01$\x08\x010\tA\x04\xa2\x88}\xbd\xdb\xd4\xbc^U\xa3\xcef\xf7\x92\xbe\xa7X;E\xb8-Y\x12^$\xe0\x1bS\xf9\x9a\x8f\xacvj\xfa!&amp;m6\x00\xe3\xd1\x85o\x1b\x9eH\xabC?\xd8\xec\x08lH\xb5X\x1f:u\xb0\xfbj\x987\n5\x01)\x01\x18$\x02`0\x04\x14~\xf0\n\'\'\x89\xc26\xa8\xdaz\xbeCv\xb2\x7fn\xca^\xb50\x05\x14~\xf0\n\'\'\x89\xc26\xa8\xdaz\xbeCv\xb2\x7fn\xca^\xb5\x180\x0b@3;\xd5n\xa0\xb1O\xecI:\x9cM\xf4u\x12a\x96\x1d\x13\x19\xa4D\xb3v&amp;_o.(\xcbs\xeb\xc0\xc4\xb80\xc2\xecF4\xfbV\'\xf7X\xe2A\xaa\xa5l\r5\xba\xbd\xa4I&amp;C\xff\xed\xd8\xa8_\x06\x18'</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="font-weight: bold">]</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="color: #808000; text-decoration-color: #808000">currentFabricIndex</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">1</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="color: #808000; text-decoration-color: #808000">attributeList</span>=<span style="color: #800080; text-decoration-color: #800080; font-style: italic">None</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="color: #808000; text-decoration-color: #808000">featureMap</span>=<span style="color: #800080; text-decoration-color: #800080; font-style: italic">None</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="color: #808000; text-decoration-color: #808000">clusterRevision</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">1</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="font-weight: bold">)</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="font-weight: bold">}</span>
<span style="font-weight: bold">}</span>
</pre>

```python
acl = [ Clusters.AccessControl.Structs.AccessControlEntry(
    fabricIndex = 1,
    privilege = Clusters.AccessControl.Enums.Privilege.kAdminister,
    authMode = Clusters.AccessControl.Enums.AuthMode.kCase,
    subjects = [ 1 ] )
]

acl
```

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace">
<span style="font-weight: bold">[</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="color: #800080; text-decoration-color: #800080; font-weight: bold">AccessControlEntry</span><span style="font-weight: bold">(</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #808000; text-decoration-color: #808000">fabricIndex</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">1</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #808000; text-decoration-color: #808000">privilege</span>=<span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">Privilege.kAdminister:</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">5</span><span style="font-weight: bold">&gt;</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #808000; text-decoration-color: #808000">authMode</span>=<span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">AuthMode.kCase:</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">2</span><span style="font-weight: bold">&gt;</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #808000; text-decoration-color: #808000">subjects</span>=<span style="font-weight: bold">[</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">1</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="font-weight: bold">]</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #808000; text-decoration-color: #808000">targets</span>=<span style="color: #800080; text-decoration-color: #800080">Null</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="font-weight: bold">)</span>
<span style="font-weight: bold">]</span>
</pre>

```python
await devCtrl.WriteAttribute(2, [ (0, Clusters.AccessControl.Attributes.Acl( acl ) ) ] )
```

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace">
<span style="font-weight: bold">[</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="color: #800080; text-decoration-color: #800080; font-weight: bold">AttributeStatus</span><span style="font-weight: bold">(</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #808000; text-decoration-color: #808000">Path</span>=<span style="color: #800080; text-decoration-color: #800080; font-weight: bold">AttributePath</span><span style="font-weight: bold">(</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="color: #808000; text-decoration-color: #808000">EndpointId</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">0</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="color: #808000; text-decoration-color: #808000">ClusterId</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">31</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="color: #808000; text-decoration-color: #808000">AttributeId</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">0</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="font-weight: bold">)</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #808000; text-decoration-color: #808000">Status</span>=<span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">Status.Success:</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">0</span><span style="font-weight: bold">&gt;</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="font-weight: bold">)</span>
<span style="font-weight: bold">]</span>
</pre>

```python
data = await devCtrl.ReadAttribute(2, [ (0, Clusters.AccessControl.Attributes.Acl) ] )
data
```

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace">
<span style="font-weight: bold">{</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">0</span>: <span style="font-weight: bold">{</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">class</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008000; text-decoration-color: #008000">'chip.clusters.Objects.AccessControl'</span><span style="font-weight: bold">&gt;</span>: <span style="font-weight: bold">{</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">class</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008000; text-decoration-color: #008000">'chip.clusters.Objects.AccessControl.Attributes.Acl'</span><span style="font-weight: bold">&gt;</span>: <span style="font-weight: bold">[</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   </span><span style="color: #800080; text-decoration-color: #800080; font-weight: bold">AccessControlEntry</span><span style="font-weight: bold">(</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">fabricIndex</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">1</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">privilege</span>=<span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">Privilege.kAdminister:</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">5</span><span style="font-weight: bold">&gt;</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">authMode</span>=<span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">AuthMode.kCase:</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">2</span><span style="font-weight: bold">&gt;</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">subjects</span>=<span style="font-weight: bold">[</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   │   </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">1</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="font-weight: bold">]</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">targets</span>=<span style="color: #800080; text-decoration-color: #800080">Null</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   </span><span style="font-weight: bold">)</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="font-weight: bold">]</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="font-weight: bold">}</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="font-weight: bold">}</span>
<span style="font-weight: bold">}</span>
</pre>

```python
acl = data[0][chip.clusters.Objects.AccessControl][chip.clusters.Objects.AccessControl.Attributes.Acl]
acl
```

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace">
<span style="font-weight: bold">[</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="color: #800080; text-decoration-color: #800080; font-weight: bold">AccessControlEntry</span><span style="font-weight: bold">(</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #808000; text-decoration-color: #808000">fabricIndex</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">1</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #808000; text-decoration-color: #808000">privilege</span>=<span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">Privilege.kAdminister:</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">5</span><span style="font-weight: bold">&gt;</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #808000; text-decoration-color: #808000">authMode</span>=<span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">AuthMode.kCase:</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">2</span><span style="font-weight: bold">&gt;</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #808000; text-decoration-color: #808000">subjects</span>=<span style="font-weight: bold">[</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">1</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="font-weight: bold">]</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #808000; text-decoration-color: #808000">targets</span>=<span style="color: #800080; text-decoration-color: #800080">Null</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="font-weight: bold">)</span>
<span style="font-weight: bold">]</span>
</pre>

```python
acl.append(Clusters.AccessControl.Structs.AccessControlEntry(
    fabricIndex = 1,
    privilege = Clusters.AccessControl.Enums.Privilege.kOperate,
    authMode = Clusters.AccessControl.Enums.AuthMode.kCase,
    targets = [ Clusters.AccessControl.Structs.Target(
        endpoint = 1,
    ) ] ) )
acl
```

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace">
<span style="font-weight: bold">[</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="color: #800080; text-decoration-color: #800080; font-weight: bold">AccessControlEntry</span><span style="font-weight: bold">(</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #808000; text-decoration-color: #808000">fabricIndex</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">1</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #808000; text-decoration-color: #808000">privilege</span>=<span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">Privilege.kAdminister:</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">5</span><span style="font-weight: bold">&gt;</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #808000; text-decoration-color: #808000">authMode</span>=<span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">AuthMode.kCase:</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">2</span><span style="font-weight: bold">&gt;</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #808000; text-decoration-color: #808000">subjects</span>=<span style="font-weight: bold">[</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">1</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="font-weight: bold">]</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #808000; text-decoration-color: #808000">targets</span>=<span style="color: #800080; text-decoration-color: #800080">Null</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="font-weight: bold">)</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="color: #800080; text-decoration-color: #800080; font-weight: bold">AccessControlEntry</span><span style="font-weight: bold">(</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #808000; text-decoration-color: #808000">fabricIndex</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">1</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #808000; text-decoration-color: #808000">privilege</span>=<span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">Privilege.kOperate:</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">3</span><span style="font-weight: bold">&gt;</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #808000; text-decoration-color: #808000">authMode</span>=<span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">AuthMode.kCase:</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">2</span><span style="font-weight: bold">&gt;</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #808000; text-decoration-color: #808000">subjects</span>=<span style="color: #800080; text-decoration-color: #800080">Null</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #808000; text-decoration-color: #808000">targets</span>=<span style="font-weight: bold">[</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="color: #800080; text-decoration-color: #800080; font-weight: bold">Target</span><span style="font-weight: bold">(</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">cluster</span>=<span style="color: #800080; text-decoration-color: #800080">Null</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">endpoint</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">1</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">deviceType</span>=<span style="color: #800080; text-decoration-color: #800080">Null</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="font-weight: bold">)</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="font-weight: bold">]</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="font-weight: bold">)</span>
<span style="font-weight: bold">]</span>
</pre>

```python
await devCtrl.WriteAttribute(2, [ (0, Clusters.AccessControl.Attributes.Acl( acl ) ) ] )
```

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace">
<span style="font-weight: bold">[</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="color: #800080; text-decoration-color: #800080; font-weight: bold">AttributeStatus</span><span style="font-weight: bold">(</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #808000; text-decoration-color: #808000">Path</span>=<span style="color: #800080; text-decoration-color: #800080; font-weight: bold">AttributePath</span><span style="font-weight: bold">(</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="color: #808000; text-decoration-color: #808000">EndpointId</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">0</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="color: #808000; text-decoration-color: #808000">ClusterId</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">31</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="color: #808000; text-decoration-color: #808000">AttributeId</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">0</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="font-weight: bold">)</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #808000; text-decoration-color: #808000">Status</span>=<span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">Status.Success:</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">0</span><span style="font-weight: bold">&gt;</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="font-weight: bold">)</span>
<span style="font-weight: bold">]</span>
</pre>

```python
await devCtrl.ReadAttribute(2, [ (0, Clusters.AccessControl.Attributes.Acl ) ] )
```

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace">
<span style="font-weight: bold">{</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">0</span>: <span style="font-weight: bold">{</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">class</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008000; text-decoration-color: #008000">'chip.clusters.Objects.AccessControl'</span><span style="font-weight: bold">&gt;</span>: <span style="font-weight: bold">{</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">class</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008000; text-decoration-color: #008000">'chip.clusters.Objects.AccessControl.Attributes.Acl'</span><span style="font-weight: bold">&gt;</span>: <span style="font-weight: bold">[</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   </span><span style="color: #800080; text-decoration-color: #800080; font-weight: bold">AccessControlEntry</span><span style="font-weight: bold">(</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">fabricIndex</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">1</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">privilege</span>=<span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">Privilege.kAdminister:</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">5</span><span style="font-weight: bold">&gt;</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">authMode</span>=<span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">AuthMode.kCase:</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">2</span><span style="font-weight: bold">&gt;</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">subjects</span>=<span style="font-weight: bold">[</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   │   </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">1</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="font-weight: bold">]</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">targets</span>=<span style="color: #800080; text-decoration-color: #800080">Null</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   </span><span style="font-weight: bold">)</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   </span><span style="color: #800080; text-decoration-color: #800080; font-weight: bold">AccessControlEntry</span><span style="font-weight: bold">(</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">fabricIndex</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">1</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">privilege</span>=<span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">Privilege.kOperate:</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">3</span><span style="font-weight: bold">&gt;</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">authMode</span>=<span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">AuthMode.kCase:</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">2</span><span style="font-weight: bold">&gt;</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">subjects</span>=<span style="color: #800080; text-decoration-color: #800080">Null</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">targets</span>=<span style="font-weight: bold">[</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   │   </span><span style="color: #800080; text-decoration-color: #800080; font-weight: bold">Target</span><span style="font-weight: bold">(</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">cluster</span>=<span style="color: #800080; text-decoration-color: #800080">Null</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">endpoint</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">1</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">deviceType</span>=<span style="color: #800080; text-decoration-color: #800080">Null</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   │   </span><span style="font-weight: bold">)</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="font-weight: bold">]</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   </span><span style="font-weight: bold">)</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="font-weight: bold">]</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="font-weight: bold">}</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="font-weight: bold">}</span>
<span style="font-weight: bold">}</span>
</pre>
