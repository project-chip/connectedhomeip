# Matter AV Analysis Node Example

An AV Analysis Node (device type 0x0149): it establishes an Analysis video
stream on a camera, pulls it over WebRTC and reports the contexts it perceives
through the AV Analysis cluster. Media arrives through `libdatachannel` and is
not decoded.

## Build

The AV analysis node, the Linux camera it streams from, and chip-tool. The
camera needs the packages listed in its [README](../camera-app/linux/README.md).

```bash
./scripts/build/build_examples.py --target linux-x64-av-analysis-clang --target linux-x64-camera-clang --target linux-x64-chip-tool-clang build
```

## Run with the Linux camera

Start the camera with its test sources and the node with the app pipe, each with
its own storage and port:

```bash
out/linux-x64-camera-clang/chip-camera-app --discriminator 3840 --passcode 20202021 --KVS /tmp/camera_kvs --secured-device-port 5540 --camera-test-videosrc --camera-test-audiosrc


out/linux-x64-av-analysis-clang/chip-av-analysis-app --discriminator 3841 --passcode 20202022 --KVS /tmp/node_kvs --secured-device-port 5541 --app-pipe /tmp/chip_av_analysis_fifo
```

Commission both onto one fabric, camera as node 1 and analysis node as node 2,
and allow each invoke command on the other's clusters: the av analysis node
needs Manage access on the camera for `VideoStreamAllocate`, the camera needs
Operate access on the node for the WebRTC Requestor commands.

```bash
chip-tool pairing onnetwork-long 1 20202021 3840
chip-tool pairing onnetwork-long 2 20202022 3841

chip-tool accesscontrol write acl '[{"fabricIndex":1,"privilege":5,"authMode":2,"subjects":[112233],"targets":null},{"fabricIndex":1,"privilege":4,"authMode":2,"subjects":[2],"targets":null}]' 1 0

chip-tool accesscontrol write acl '[{"fabricIndex":1,"privilege":5,"authMode":2,"subjects":[112233],"targets":null},{"fabricIndex":1,"privilege":3,"authMode":2,"subjects":[1],"targets":null}]' 2 0
```

Establish the stream on the camera, then activate it over the camera's WebRTC
endpoint. The node logs `media flowing` once RTP arrives.

```bash
chip-tool avanalysis establish-analysis-stream 1 2 1
chip-tool avanalysis activate-analysis-stream 0 2 1 --WebRTCEndpointID 1
```

## Simulating detection

Enable a context the node supports, then report through the pipe. `SourceNodeId`
must be a camera the node has a connected stream from.

```bash
chip-tool avanalysis enable-context-triggers '[{"context":{"mfgCode":null,"namespaceID":73,"tag":11}}]' 2 1

echo '{"Name":"AvAnalysisSessionStart","SourceNodeId":1}' > /tmp/chip_av_analysis_fifo

echo '{"Name":"AvAnalysisPerceivedContext","NewContexts":[{"NamespaceId":73,"Tag":11}]}' > /tmp/chip_av_analysis_fifo

echo '{"Name":"AvAnalysisSessionEnd"}' > /tmp/chip_av_analysis_fifo

chip-tool avanalysis read-event perceived-context 2 1
```

Deactivating the stream ends the sessions sourced from it:

```bash
chip-tool avanalysis deactivate-analysis-stream 0 2 1
chip-tool avanalysis read-event analysis-session-end 2 1
```
