---
orphan: true
---

# Push AV Server

This tool provide a web server that can be used to implement Matter cameras. The
server does not go out of its way to provide validation of the media ingested
(run the test harness to do so), but it does offer as much visibility as
possible on what the ingest source is sending to the server.

## Before running

The PUSH AV server needs to access the client certificate used to upload media.
Unfortunately the Python's `ASGI` (web interface standard) extension for TLS
content is currently in a draft format and not implemented in any python web
server.

We have patched the web server we use to actually include TLS information as an
extension. Because of this, we need to patch the `hypercorn` dependency before
running the server. This is done with the following command (assuming it is run
from a shell with virtual env enabled):

```sh
$ patch -d $(pip show hypercorn | grep "Location: " | sed "s/Location: //") < src/tools/push_av_server/hypercorn.patch
```

## Example

Here is an example of an interaction with the push AV server tool.

```sh
# You can omit `--strict-mode` if you want the ref server to accept uploads on any URL paths
$ python server.py --working-directory ~/.pavstest --strict-mode


# First let's create a device key and certificate.
# The response will provide information as to where the key and certificate are located.
$ curl --cacert ~/.pavstest/certs/server/root.pem -XPOST https://localhost:1234/certs/dev/keypair

# Now that we have a device identity, we can create a stream
$ curl --cacert ~/.pavstest/certs/server/root.pem --cert ~/.pavstest/certs/device/dev.pem --key ~/.pavstest/certs/device/dev.key -XPOST https://localhost:1234/streams

# And now that we have access to our stream_id, we can build the publishing endpoint for
# any CMAF ingest flow we have. The example below assuming a stream id of "1".
$ export PUBLISHING_ENDPOINT=https://localhost:1234/streams/1

# The tool also contains a script to generate arbitrary CMAF content.
# This may be useful to implementers of a publish endpoint.
# This tool makes use of the previously created PUBLISHING_ENDPOINT environment variable.
# TODO Handle non-hardcoded client certificate
$ ./generate_cmaf_content.sh

# You can also list all streams and their associated files
$ curl -XGET --cacert ~/.pavstest/certs/server/root.pem https://localhost:1234/streams

# Get detailed information about the uploaded media file.
# This correspond to the ffprobe tool output
$ curl --cacert ~/.pavstest/certs/server/root.pem -XGET 'https://localhost:1234/probe/1/cmaf/example/video-720p.cmfv'

# You can also use the web server to sign certificates if given a CSR.
# First create a key and csr for your device:
$ openssl req -new -newkey rsa:2048 -nodes -keyout client.key -out client.csr -subj "/CN=test"

# When sending the CSR over JSON we need to have the newline characters be the literal \n.
$ sed '$!G' client.csr | paste -sd '\\n' - > client.curl.csr

# Then sign it with the server
$ curl --cacert ~/.pavstest/certs/server/root.pem -XPOST 'https://localhost:1234/certs/my-device/sign' -d "{\"csr\":\"$(cat client.curl.csr)\"}" --header "content-type: application/json"

```

This command-line flow demonstrates a complete interaction with a secure media
server designed for "push" streaming, where a client sends media data to the
server. The process involves setting up security, creating a stream, publishing
content, and managing the system.

The core technology here is `CMAF` (Common Media Application Format), a
standardized container format for segmented streaming video, and mTLS (Mutual
Transport Layer Security) for security.

### 1. Server & Initial Device Identity

This section covers starting the server and creating the first identity for a
client (a "device").

```sh
$ python server.py --working-directory ~/.pavstest
```

This command starts the Push AV (Audio/Video) server. The --working-directory
option tells the server where to store all its files, such as certificates,
keys, and uploaded media content.

```sh
$ curl --cacert ~/.pavstest/certs/server/root.pem -XPOST https://localhost:1234/certs/dev/keypair
```

This requests the server to create a new identity (a public certificate and a
private key) for a device.

`--cacert`: This trusts the server's self-signed root certificate, which is
necessary to establish a secure HTTPS connection.

`-XPOST`: Sends a request to create a new resource.

The server generates the key pair and saves it in its working directory (e.g.,
in `~/.pavstest/certs/device/`). This provides the credentials the device shall
use in the next step.

### 2. Authenticating and Creating a Stream

Now that the device has an identity, it uses it to authenticate itself to the
server and reserve a "stream" to which it can publish media.

```sh
curl --cacert ... --cert ... --key ... -XPOST https://localhost:1234/streams
```

This command creates a new media stream. The key difference here is that the
client presents its own certificate to prove its identity. This is called client
certificate authentication or mTLS (mutual TLS). It's like a two-way ID check:
the client verifies the server's identity (using `--cacert`), and the server
verifies the client's identity (using `--cert` and `--key`). The server responds
with a unique stream_id for the newly created stream.

```sh
export PUBLISHING_ENDPOINT=https://localhost:1234/streams/1
```

This command saves the stream's URL into an environment variable for easy
access. This URL is the ingest endpoint—the specific address where the `CMAF`
media segments will be uploaded. The 1 at the end is the stream_id returned by
the previous command.

### 3. Publishing Media Content

With the ingest endpoint defined, the client can now start pushing media data.

```sh
./generate_cmaf_content.sh
```

This script simulates a media source like a live camera or a file transcoder. It
generates sample video/audio content formatted as `CMAF` segments and then uses
a tool like curl to upload (HTTP PUT or POST) those segments to the
\$PUBLISHING_ENDPOINT. This is the "push" part of the "push AV server."

### 4. Monitoring and Inspection

These commands are for managing and checking the status of the streams and media
files on the server.

```sh
curl -XGET ... https://localhost:1234/streams
```

This lists all active streams on the server, allowing an administrator to see
which streams exist and what files are associated with them.

```sh
curl ... -XGET 'https://localhost:1234/probe/1/cmaf/example/video-720p.cmfv'
```

This command retrieves detailed technical metadata about a specific media file
(`video-720p.cmfv`) within a specific stream (stream_id 1). The output is
similar to what the popular media analysis tool `ffprobe` would provide, showing
information like codec, resolution, bitrate, etc.

### 5. Alternative Certificate Issuance (`CSR`)

This final section shows a more standard, secure method for a device to obtain a
certificate, using a Certificate Signing Request (`CSR`). This is common
practice in Public Key Infrastructure (PKI).

```sh
openssl req -new -newkey ... -out client.csr ...
```

The device first generates its own private key and a `CSR`. The `CSR` is a block
of encoded text containing the device's public key and identity information.
It's a formal request to a Certificate Authority (in this case, our server) to
sign the public key.

```sh
sed ... | paste ... > client.curl.csr
```

This is a text-formatting command. It takes the multi-line `CSR` file and
converts the newline characters into literal \n characters so that the entire
`CSR` can be embedded cleanly into a single-line JSON payload.

```sh
curl ... -XPOST '...' -d "{\"csr\":\"$(cat client.curl.csr)\"}"
```

The device sends its `CSR` to the server's /sign endpoint. The server acts as a
Certificate Authority, verifies the request, and uses its own root key to sign
the device's public key, creating a valid client certificate. The server saves
this certificate and returns its path in the response. The device can then fetch
the certificate content from that path and use it with its private key to
authenticate, just as in Step 2.
