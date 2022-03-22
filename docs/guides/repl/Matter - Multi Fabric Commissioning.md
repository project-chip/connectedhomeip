# Multi Fabric - Commissioning and Interactions

<a href="http://35.236.121.59/hub/user-redirect/git-pull?repo=https%3A%2F%2Fgithub.com%2Fproject-chip%2Fconnectedhomeip&urlpath=lab%2Ftree%2Fconnectedhomeip%2Fdocs%2Fguides%2Frepl%2FMatter%2520-%2520Multi%2520Fabric%2520Commissioning.ipynb&branch=master">
<img src="https://i.ibb.co/hR3yWsC/launch-playground.png" alt="drawing" width="130"/>
</a>
<br></br>

This walks through creating multiple controllers on multiple fabrics, using
those controllers to commission a target onto those fabrics and finally,
interacting with them using the interaction model.

## FabricAdmins and Controllers

The `FabricAdmin` class (present in the `chip.FabricAdmin` package) is
responsible for adminstering a fabric. It houses the Fabric ID and Index, as
well as an RCAC and ICAC that provides the certificate material grounding that
fabric.

The `FabricAdmin` can be used to vend `ChipDeviceController` objects that
represent a controller instance with a specific identity grounded in the admin's
fabric. This controller can then be used to commission and interact with
devices.

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

## Initialization

Let's first begin by setting up by importing some key modules that are needed to
make it easier for us to interact with the Matter stack.

`ChipReplStartup.py` is run within the global namespace. This results in all of
its imports being made available here.

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

    2022-01-25 16:58:57 johnsj-macbookpro1.roam.corp.google.com root[27921] ERROR [Errno 2] No such file or directory: '/tmp/repl-storage.json'
    2022-01-25 16:58:57 johnsj-macbookpro1.roam.corp.google.com root[27921] WARNING Could not load configuration from /tmp/repl-storage.json - resetting configuration...

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace">
<span style="color: #af00ff; text-decoration-color: #af00ff">No previous fabric admins discovered in persistent storage - creating a new one...</span>
</pre>

    New FabricAdmin: FabricId: 1(1)

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace">

</pre>

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace"><span style="color: #af00ff; text-decoration-color: #af00ff">Creating default device controller on fabric </span><span style="color: #af00ff; text-decoration-color: #af00ff; font-weight: bold">1</span><span style="color: #af00ff; text-decoration-color: #af00ff">...</span>
</pre>

    Allocating new controller with FabricId: 1(1), NodeId: 1

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace">

<span style="color: #000080; text-decoration-color: #000080">Default CHIP Device Controller has been initialized to manage </span><span style="color: #800000; text-decoration-color: #800000; font-weight: bold">fabricAdmins[</span><span style="color: #800000; text-decoration-color: #800000; font-weight: bold">0</span><span style="color: #800000; text-decoration-color: #800000; font-weight: bold">]</span><span style="color: #000080; text-decoration-color: #000080; font-weight: bold">, and is </span>
<span style="color: #000080; text-decoration-color: #000080; font-weight: bold">available as </span><span style="color: #800000; text-decoration-color: #800000; font-weight: bold">devCtrl</span>
</pre>

At startup, the REPL will attempt to find any previously configured fabrics
stored in persisted storage. If it can't find any (as is the case here), it will
construct a default `FabricAdmin` object on Fabric 1 (Index 1) as well as
construct a device controller (`devCtrl`) on that fabric.

```python
fabricAdmins
```

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace">
<span style="font-weight: bold">[</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">chip.FabricAdmin.FabricAdmin</span><span style="color: #000000; text-decoration-color: #000000"> object at </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">0x7feacf3d1dc0</span><span style="font-weight: bold">&gt;</span>
<span style="font-weight: bold">]</span>
</pre>

You can check the underlying fabric info by typing the name directly in the
repl.

```python
devCtrl
```

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace"><span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">Controller</span><span style="color: #000000; text-decoration-color: #000000"> for Fabric </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">0000000000000001</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #000000; text-decoration-color: #000000; font-weight: bold">(</span><span style="color: #000000; text-decoration-color: #000000">Compressed Fabric Id: b75777b5840b9309</span><span style="color: #000000; text-decoration-color: #000000; font-weight: bold">)</span><span style="font-weight: bold">&gt;</span>
</pre>

### Commission onto Fabric 1

#### Launch Server

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

#### Commission Target

Commission the target onto Fabric 1 using the default device controller instance
with a NodeId of 1.

```python
devCtrl.CommissionIP(b'127.0.0.1', 20202021, 2)
```

    2022-03-21 17:50:16 songguo.sha.corp.google.com chip.CTL[2635279] ERROR Unable to find country code, defaulting to XX
    2022-03-21 17:50:16 songguo.sha.corp.google.com chip.DL[2635279] ERROR Avahi resolve found
    2022-03-21 17:50:16 songguo.sha.corp.google.com chip.SC[2635279] ERROR The device does not support GetClock_RealTimeMS() API. This will eventually result in CASE session setup failures.


    Commissioning complete

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace"><span style="color: #00ff00; text-decoration-color: #00ff00; font-style: italic">True</span>
</pre>

### Read OpCreds Cluster

Read out the OpCreds cluster to confirm membership into Fabric 1.

```python
await devCtrl.ReadAttribute(2, [(Clusters.OperationalCredentials.Attributes.Fabrics)], fabricFiltered=False)
```

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace">
<span style="font-weight: bold">{</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">0</span>: <span style="font-weight: bold">{</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">class</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008000; text-decoration-color: #008000">'chip.clusters.Objects.OperationalCredentials'</span><span style="font-weight: bold">&gt;</span>: <span style="font-weight: bold">{</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">class</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008000; text-decoration-color: #008000">'chip.clusters.Attribute.DataVersion'</span><span style="font-weight: bold">&gt;</span>: <span style="color: #008080; text-decoration-color: #008080; font-weight: bold">2299082310</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">class</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008000; text-decoration-color: #008000">'chip.clusters.Objects.OperationalCredentials.Attributes.Fabrics'</span><span style="font-weight: bold">&gt;</span>: <span style="font-weight: bold">[</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   </span><span style="color: #800080; text-decoration-color: #800080; font-weight: bold">FabricDescriptor</span><span style="font-weight: bold">(</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">rootPublicKey</span>=<span style="color: #008000; text-decoration-color: #008000">b'\x04\xba\x07f\xa8`\xfeh\xefe\x9eh\n\xda\x93\xb4nc\xdb\xcb\x89\xc2x]I\xaf#\x8c\xbc\xa7~5\x14$\xd7h\x84\x03\xf0-,A\x06\xcd\xa0\xc8\t\xca\xa7!\x06\xefc!B`3\xe9H\xfa\xf9\x97):\x11'</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">vendorId</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">28446</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">fabricId</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">1</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">nodeId</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">2</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">label</span>=<span style="color: #008000; text-decoration-color: #008000">''</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">fabricIndex</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">1</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   </span><span style="font-weight: bold">)</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="font-weight: bold">]</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="font-weight: bold">}</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="font-weight: bold">}</span>
<span style="font-weight: bold">}</span>
</pre>

### Commission onto Fabric 2

#### Create new FabricAdmin

```python
import chip.FabricAdmin as FabricAdmin
fabric2 = FabricAdmin.FabricAdmin(fabricId = 2, fabricIndex = 2)
```

    New FabricAdmin: FabricId: 2(2)

Here's a brief peek at the JSON data that is in the persisted storage file.

```python
builtins.chipStack.GetStorageManager().jsonData
```

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace">
<span style="font-weight: bold">{</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="color: #008000; text-decoration-color: #008000">'repl-config'</span>: <span style="font-weight: bold">{</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #008000; text-decoration-color: #008000">'fabricAdmins'</span>: <span style="font-weight: bold">{</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="color: #008000; text-decoration-color: #008000">'1'</span>: <span style="font-weight: bold">{</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   </span><span style="color: #008000; text-decoration-color: #008000">'fabricId'</span>: <span style="color: #008080; text-decoration-color: #008080; font-weight: bold">1</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="font-weight: bold">}</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="color: #008000; text-decoration-color: #008000">'2'</span>: <span style="font-weight: bold">{</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   </span><span style="color: #008000; text-decoration-color: #008000">'fabricId'</span>: <span style="color: #008080; text-decoration-color: #008080; font-weight: bold">2</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="font-weight: bold">}</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="font-weight: bold">}</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="font-weight: bold">}</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="color: #008000; text-decoration-color: #008000">'sdk-config'</span>: <span style="font-weight: bold">{</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #008000; text-decoration-color: #008000">'gcc'</span>: <span style="color: #008000; text-decoration-color: #008000">'6AMAAA=='</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #008000; text-decoration-color: #008000">'gdc'</span>: <span style="color: #008000; text-decoration-color: #008000">'6AMAAA=='</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #008000; text-decoration-color: #008000">'ExampleOpCredsCAKey1'</span>: <span style="color: #008000; text-decoration-color: #008000">'BLoHZqhg/mjvZZ5oCtqTtG5j28uJwnhdSa8jjLynfjUUJNdohAPwLSxBBs2gyAnKpyEG72MhQmAz6Uj6+ZcpOhEiwYZIs1Zs8jTWTwIfrnpBbKivp6iocOqEF14G0KLoOw=='</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #008000; text-decoration-color: #008000">'ExampleOpCredsICAKey1'</span>: <span style="color: #008000; text-decoration-color: #008000">'BJDYC+BfoNm9VtxmUwt2UdGbsJvjF6NUMSTW/gdXMr9YwlGNd2WaJ5vTGb9pS4tWPOsCjqsRmhtLVuAWe68jniOjjr64anP1DcFQiSxOsrWmwlghxN4ps6oRdh1+wLIZ/A=='</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #008000; text-decoration-color: #008000">'ExampleCAIntermediateCert1'</span>: <span style="color: #008000; text-decoration-color: #008000">'MIIBlTCCATygAwIBAgIBADAKBggqhkjOPQQDAjAiMSAwHgYKKwYBBAGConwBBAwQMDAwMDAwMDAwMDAwMDAwMDAeFw0yMTAxMDEwMDAwMDBaFw0zMDEyMzAwMDAwMDBaMCIxIDAeBgorBgEEAYKifAEDDBAwMDAwMDAwMDAwMDAwMDAxMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEkNgL4F+g2b1W3GZTC3ZR0Zuwm+MXo1QxJNb+B1cyv1jCUY13ZZonm9MZv2lLi1Y86wKOqxGaG0tW4BZ7ryOeI6NjMGEwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYwHQYDVR0OBBYEFPx8CeluC/tiLeUZyboeJHaibmnZMB8GA1UdIwQYMBaAFHV8XorSBpWQpPrTWaniIKzUMQQUMAoGCCqGSM49BAMCA0cAMEQCIDZarzIwBowTlNEEYOG+W2qAiv0+7a8WZrV+4Oj8imfwAiB7uMT2Cu2grqAychG6QGyn9pirJb1ITc6rohp3kcJzZQ=='</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #008000; text-decoration-color: #008000">'ExampleCARootCert1'</span>: <span style="color: #008000; text-decoration-color: #008000">'MIIBljCCATygAwIBAgIBADAKBggqhkjOPQQDAjAiMSAwHgYKKwYBBAGConwBBAwQMDAwMDAwMDAwMDAwMDAwMDAeFw0yMTAxMDEwMDAwMDBaFw0zMDEyMzAwMDAwMDBaMCIxIDAeBgorBgEEAYKifAEEDBAwMDAwMDAwMDAwMDAwMDAwMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEugdmqGD+aO9lnmgK2pO0bmPby4nCeF1JryOMvKd+NRQk12iEA/AtLEEGzaDICcqnIQbvYyFCYDPpSPr5lyk6EaNjMGEwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYwHQYDVR0OBBYEFHV8XorSBpWQpPrTWaniIKzUMQQUMB8GA1UdIwQYMBaAFHV8XorSBpWQpPrTWaniIKzUMQQUMAoGCCqGSM49BAMCA0gAMEUCIQDOKet8M81frd8SE9jEKGBoG8fP9EyELJU8rcr7imjFbQIgPPYKXr+qYmLu0UgdwG1gYA4DgwtroeBq08lXLI4vmiA='</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #008000; text-decoration-color: #008000">'f/1/r'</span>: <span style="color: #008000; text-decoration-color: #008000">'FTABAQAkAgE3AyQUABgmBIAigScmBYAlTTo3BiQUABgkBwEkCAEwCUEEugdmqGD+aO9lnmgK2pO0bmPby4nCeF1JryOMvKd+NRQk12iEA/AtLEEGzaDICcqnIQbvYyFCYDPpSPr5lyk6ETcKNQEpARgkAmAwBBR1fF6K0gaVkKT601mp4iCs1DEEFDAFFHV8XorSBpWQpPrTWaniIKzUMQQUGDALQM4p63wzzV+t3xIT2MQoYGgbx8/0TIQslTytyvuKaMVtPPYKXr+qYmLu0UgdwG1gYA4DgwtroeBq08lXLI4vmiAY'</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #008000; text-decoration-color: #008000">'f/1/i'</span>: <span style="color: #008000; text-decoration-color: #008000">'FTABAQAkAgE3AyQUABgmBIAigScmBYAlTTo3BiQTARgkBwEkCAEwCUEEkNgL4F+g2b1W3GZTC3ZR0Zuwm+MXo1QxJNb+B1cyv1jCUY13ZZonm9MZv2lLi1Y86wKOqxGaG0tW4BZ7ryOeIzcKNQEpARgkAmAwBBT8fAnpbgv7Yi3lGcm6HiR2om5p2TAFFHV8XorSBpWQpPrTWaniIKzUMQQUGDALQDZarzIwBowTlNEEYOG+W2qAiv0+7a8WZrV+4Oj8imfwe7jE9grtoK6gMnIRukBsp/aYqyW9SE3Oq6Iad5HCc2UY'</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #008000; text-decoration-color: #008000">'f/1/n'</span>: <span style="color: #008000; text-decoration-color: #008000">'FTABAQEkAgE3AyQTARgmBIAigScmBYAlTTo3BiQVASQRARgkBwEkCAEwCUEE7H1GfRAQAFIDbiSZ1dtU0Il6MbDWj7BAsn7x7ipBGMTremwb6wHJTgdYOn3TiLQyemtGpVPHSSrT/U0Eu/zXTDcKNQEoARgkAgE2AwQCBAEYMAQUlsEJfINpYHDYm44rs12a69sOI5EwBRT8fAnpbgv7Yi3lGcm6HiR2om5p2RgwC0Cxs6rv3m/CSr/Vvl6nqpjP+/UZlldwCzyTStX0bzxnLduY7WZMo8We5zSzW3y6/DwQ1jgzNKu+hBti1BeHZYCyGA=='</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #008000; text-decoration-color: #008000">'f/1/o'</span>: <span style="color: #008000; text-decoration-color: #008000">'FSQAATABYQTsfUZ9EBAAUgNuJJnV21TQiXoxsNaPsECyfvHuKkEYxOt6bBvrAclOB1g6fdOItDJ6a0alU8dJKtP9TQS7/NdMMlYH6llXytrjsG6QE8BgxWH1eA1jFzzVpFQVwjbhH7kY'</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #008000; text-decoration-color: #008000">'f/1/m'</span>: <span style="color: #008000; text-decoration-color: #008000">'FSUAHm8sAQAY'</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #008000; text-decoration-color: #008000">'ExampleOpCredsCAKey2'</span>: <span style="color: #008000; text-decoration-color: #008000">'BFXPy14Sye6FHxUEBSC3ZyT5A6V9JA376ysIqynHsvlNhcoVwau6SW+orHgQid3/STqlOx4VbHjtINkkYNyrKlt7VJaWA6n3jSUbGQPIepAyZMy01tRmLtLcuO+r+F9UPw=='</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="color: #008000; text-decoration-color: #008000">'ExampleOpCredsICAKey2'</span>: <span style="color: #008000; text-decoration-color: #008000">'BMyJDqEcQpKfxOGH3RHaaIPJNTLjAhpvpq2HhCR4Y9tsdx0P6HhsCa5IHKE1qwZ8jgMrVLX+/Jv7vPGQOKDd+OvEs4PtAus9Z7wPohnLLKIjfTrLssJIr+cKR3/dQ24fEg=='</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="font-weight: bold">}</span>
<span style="font-weight: bold">}</span>
</pre>

```python
devCtrl2 = fabric2.NewController()
devCtrl2
```

    2022-03-21 17:51:28 songguo.sha.corp.google.com chip.DL[2635279] ERROR Cannot set hostname on this system, continue anyway...


    Allocating new controller with FabricId: 2(2), NodeId: 2

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace"><span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">Controller</span><span style="color: #000000; text-decoration-color: #000000"> for Fabric </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">0000000000000002</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #000000; text-decoration-color: #000000; font-weight: bold">(</span><span style="color: #000000; text-decoration-color: #000000">Compressed Fabric Id: b68c0efe11caa83c</span><span style="color: #000000; text-decoration-color: #000000; font-weight: bold">)</span><span style="font-weight: bold">&gt;</span>
</pre>

#### Open Commissioning Window

```python
await devCtrl.SendCommand(2, 0, Clusters.AdministratorCommissioning.Commands.OpenBasicCommissioningWindow(100), timedRequestTimeoutMs=1000)
```

```python
devCtrl2.CommissionIP(b'127.0.0.1', 20202021, 2)
```

    2022-01-25 16:59:00 johnsj-macbookpro1.roam.corp.google.com chip.CTL[27921] ERROR Unable to find country code, defaulting to WW
    2022-01-25 16:59:00 johnsj-macbookpro1.roam.corp.google.com chip.SC[27921] ERROR The device does not support GetClock_RealTimeMS() API. This will eventually result in CASE session setup failures.


    Commissioning complete

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace"><span style="color: #00ff00; text-decoration-color: #00ff00; font-style: italic">True</span>
</pre>

### Read OpCreds Cluster

Read out the OpCreds cluster to confirm membership into Fabric 2.

```python
await devCtrl2.ReadAttribute(2, [(Clusters.OperationalCredentials.Attributes.Fabrics)], fabricFiltered=False)
```

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace">
<span style="font-weight: bold">{</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">0</span>: <span style="font-weight: bold">{</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">class</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008000; text-decoration-color: #008000">'chip.clusters.Objects.OperationalCredentials'</span><span style="font-weight: bold">&gt;</span>: <span style="font-weight: bold">{</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">class</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008000; text-decoration-color: #008000">'chip.clusters.Attribute.DataVersion'</span><span style="font-weight: bold">&gt;</span>: <span style="color: #008080; text-decoration-color: #008080; font-weight: bold">2299082327</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">class</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008000; text-decoration-color: #008000">'chip.clusters.Objects.OperationalCredentials.Attributes.Fabrics'</span><span style="font-weight: bold">&gt;</span>: <span style="font-weight: bold">[</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   </span><span style="color: #800080; text-decoration-color: #800080; font-weight: bold">FabricDescriptor</span><span style="font-weight: bold">(</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">rootPublicKey</span>=<span style="color: #008000; text-decoration-color: #008000">b'\x04\xba\x07f\xa8`\xfeh\xefe\x9eh\n\xda\x93\xb4nc\xdb\xcb\x89\xc2x]I\xaf#\x8c\xbc\xa7~5\x14$\xd7h\x84\x03\xf0-,A\x06\xcd\xa0\xc8\t\xca\xa7!\x06\xefc!B`3\xe9H\xfa\xf9\x97):\x11'</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">vendorId</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">28446</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">fabricId</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">1</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">nodeId</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">2</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">label</span>=<span style="color: #008000; text-decoration-color: #008000">''</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">fabricIndex</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">1</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   </span><span style="font-weight: bold">)</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   </span><span style="color: #800080; text-decoration-color: #800080; font-weight: bold">FabricDescriptor</span><span style="font-weight: bold">(</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">rootPublicKey</span>=<span style="color: #008000; text-decoration-color: #008000">b'\x04U\xcf\xcb^\x12\xc9\xee\x85\x1f\x15\x04\x05 \xb7g$\xf9\x03\xa5}$\r\xfb\xeb+\x08\xab)\xc7\xb2\xf9M\x85\xca\x15\xc1\xab\xbaIo\xa8\xacx\x10\x89\xdd\xffI:\xa5;\x1e\x15lx\xed \xd9$`\xdc\xab*['</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">vendorId</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">56886</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">fabricId</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">2</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">nodeId</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">2</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">label</span>=<span style="color: #008000; text-decoration-color: #008000">''</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">fabricIndex</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">2</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   </span><span style="font-weight: bold">)</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="font-weight: bold">]</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="font-weight: bold">}</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="font-weight: bold">}</span>
<span style="font-weight: bold">}</span>
</pre>

## Relaunch REPL

Let's simulate re-launching the REPL to show-case the capabilities of the
persistence storage and its mechanics.

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

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace"><span style="color: #af00ff; text-decoration-color: #af00ff">Restoring FabricAdmin from storage to manage FabricId </span><span style="color: #af00ff; text-decoration-color: #af00ff; font-weight: bold">2</span><span style="color: #af00ff; text-decoration-color: #af00ff">, FabricIndex </span><span style="color: #af00ff; text-decoration-color: #af00ff; font-weight: bold">2</span><span style="color: #af00ff; text-decoration-color: #af00ff">...</span>
</pre>

    New FabricAdmin: FabricId: 2(2)

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

The REPL has now loaded the two fabrics that were created in the previous
session into the `fabricAdmins` variable. It has also created a default
controller on the first fabric in that list (Fabric 1) as `devCtrl`.

### Establish CASE and Read OpCreds

To prove that we do indeed have two distinct fabrics and controllers on each
fabric, let's go ahead and update the label of each fabric. To do so, you'd need
to succcessfully establish a CASE session through a controller on the respective
fabric, and call the 'UpdateLabel' command.

Underneath the covers, each device controller will do operational discovery of
the NodeId being read and establish a CASE session before issuing the IM
interaction.

```python
await devCtrl.SendCommand(2, 0, Clusters.OperationalCredentials.Commands.UpdateFabricLabel("Fabric1Label"))
await devCtrl.ReadAttribute(2, [(Clusters.OperationalCredentials.Attributes.Fabrics)], fabricFiltered=False)
```

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace">
<span style="font-weight: bold">{</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">0</span>: <span style="font-weight: bold">{</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">class</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008000; text-decoration-color: #008000">'chip.clusters.Objects.OperationalCredentials'</span><span style="font-weight: bold">&gt;</span>: <span style="font-weight: bold">{</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">class</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008000; text-decoration-color: #008000">'chip.clusters.Attribute.DataVersion'</span><span style="font-weight: bold">&gt;</span>: <span style="color: #008080; text-decoration-color: #008080; font-weight: bold">2299082347</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">class</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008000; text-decoration-color: #008000">'chip.clusters.Objects.OperationalCredentials.Attributes.Fabrics'</span><span style="font-weight: bold">&gt;</span>: <span style="font-weight: bold">[</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   </span><span style="color: #800080; text-decoration-color: #800080; font-weight: bold">FabricDescriptor</span><span style="font-weight: bold">(</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">rootPublicKey</span>=<span style="color: #008000; text-decoration-color: #008000">b'\x04\xba\x07f\xa8`\xfeh\xefe\x9eh\n\xda\x93\xb4nc\xdb\xcb\x89\xc2x]I\xaf#\x8c\xbc\xa7~5\x14$\xd7h\x84\x03\xf0-,A\x06\xcd\xa0\xc8\t\xca\xa7!\x06\xefc!B`3\xe9H\xfa\xf9\x97):\x11'</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">vendorId</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">28446</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">fabricId</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">1</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">nodeId</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">2</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">label</span>=<span style="color: #008000; text-decoration-color: #008000">'Fabric1Label'</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">fabricIndex</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">1</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   </span><span style="font-weight: bold">)</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   </span><span style="color: #800080; text-decoration-color: #800080; font-weight: bold">FabricDescriptor</span><span style="font-weight: bold">(</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">rootPublicKey</span>=<span style="color: #008000; text-decoration-color: #008000">b'\x04U\xcf\xcb^\x12\xc9\xee\x85\x1f\x15\x04\x05 \xb7g$\xf9\x03\xa5}$\r\xfb\xeb+\x08\xab)\xc7\xb2\xf9M\x85\xca\x15\xc1\xab\xbaIo\xa8\xacx\x10\x89\xdd\xffI:\xa5;\x1e\x15lx\xed \xd9$`\xdc\xab*['</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">vendorId</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">56886</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">fabricId</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">2</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">nodeId</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">2</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">label</span>=<span style="color: #008000; text-decoration-color: #008000">''</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">fabricIndex</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">2</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   </span><span style="font-weight: bold">)</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="font-weight: bold">]</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="font-weight: bold">}</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="font-weight: bold">}</span>
<span style="font-weight: bold">}</span>
</pre>

Instantiate a controller on fabric 2 and use it to read out the op creds from
that fabric.

```python
devCtrl2 = fabricAdmins[1].NewController()
await devCtrl2.SendCommand(2, 0, Clusters.OperationalCredentials.Commands.UpdateFabricLabel("Fabric2Label"))
await devCtrl2.ReadAttribute(2, [(Clusters.OperationalCredentials.Attributes.Fabrics)], fabricFiltered=False)
```

    2022-03-21 17:53:10 songguo.sha.corp.google.com chip.DL[2635279] ERROR Cannot set hostname on this system, continue anyway...
    2022-03-21 17:53:10 songguo.sha.corp.google.com chip.DL[2635279] ERROR Avahi resolve found


    Allocating new controller with FabricId: 2(2), NodeId: 2


    2022-03-21 17:53:10 songguo.sha.corp.google.com chip.SC[2635279] ERROR The device does not support GetClock_RealTimeMS() API. This will eventually result in CASE session setup failures.

<pre style="white-space:pre;overflow-x:auto;line-height:normal;font-family:Menlo,'DejaVu Sans Mono',consolas,'Courier New',monospace">
<span style="font-weight: bold">{</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="color: #008080; text-decoration-color: #008080; font-weight: bold">0</span>: <span style="font-weight: bold">{</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">class</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008000; text-decoration-color: #008000">'chip.clusters.Objects.OperationalCredentials'</span><span style="font-weight: bold">&gt;</span>: <span style="font-weight: bold">{</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">class</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008000; text-decoration-color: #008000">'chip.clusters.Attribute.DataVersion'</span><span style="font-weight: bold">&gt;</span>: <span style="color: #008080; text-decoration-color: #008080; font-weight: bold">2299082367</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="font-weight: bold">&lt;</span><span style="color: #ff00ff; text-decoration-color: #ff00ff; font-weight: bold">class</span><span style="color: #000000; text-decoration-color: #000000"> </span><span style="color: #008000; text-decoration-color: #008000">'chip.clusters.Objects.OperationalCredentials.Attributes.Fabrics'</span><span style="font-weight: bold">&gt;</span>: <span style="font-weight: bold">[</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   </span><span style="color: #800080; text-decoration-color: #800080; font-weight: bold">FabricDescriptor</span><span style="font-weight: bold">(</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">rootPublicKey</span>=<span style="color: #008000; text-decoration-color: #008000">b'\x04\xba\x07f\xa8`\xfeh\xefe\x9eh\n\xda\x93\xb4nc\xdb\xcb\x89\xc2x]I\xaf#\x8c\xbc\xa7~5\x14$\xd7h\x84\x03\xf0-,A\x06\xcd\xa0\xc8\t\xca\xa7!\x06\xefc!B`3\xe9H\xfa\xf9\x97):\x11'</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">vendorId</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">28446</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">fabricId</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">1</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">nodeId</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">2</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">label</span>=<span style="color: #008000; text-decoration-color: #008000">'Fabric1Label'</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">fabricIndex</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">1</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   </span><span style="font-weight: bold">)</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   </span><span style="color: #800080; text-decoration-color: #800080; font-weight: bold">FabricDescriptor</span><span style="font-weight: bold">(</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">rootPublicKey</span>=<span style="color: #008000; text-decoration-color: #008000">b'\x04U\xcf\xcb^\x12\xc9\xee\x85\x1f\x15\x04\x05 \xb7g$\xf9\x03\xa5}$\r\xfb\xeb+\x08\xab)\xc7\xb2\xf9M\x85\xca\x15\xc1\xab\xbaIo\xa8\xacx\x10\x89\xdd\xffI:\xa5;\x1e\x15lx\xed \xd9$`\xdc\xab*['</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">vendorId</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">56886</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">fabricId</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">2</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">nodeId</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">2</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">label</span>=<span style="color: #008000; text-decoration-color: #008000">'Fabric2Label'</span>,
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   │   </span><span style="color: #808000; text-decoration-color: #808000">fabricIndex</span>=<span style="color: #008080; text-decoration-color: #008080; font-weight: bold">2</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   │   </span><span style="font-weight: bold">)</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   │   </span><span style="font-weight: bold">]</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   │   </span><span style="font-weight: bold">}</span>
<span style="color: #7fbf7f; text-decoration-color: #7fbf7f">│   </span><span style="font-weight: bold">}</span>
<span style="font-weight: bold">}</span>
</pre>

```python
devCtrl2.Shutdown()
```
