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
Unfortunately the Python's ASGI (web interface standard) extension for TLS
content is currently in a draft format and not implemented in any python web
server.

We have patched the webserver we use to actually include TLS information as an
extension. Because of this, we need to patch the hypercorn dependency before
running the server. This is done with the following command (assuming it is run
from a shell with virtual env enabled):

```sh
$ patch -d $VIRTUAL_ENV < src/tools/push_av_server/hypercorn.patch
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
