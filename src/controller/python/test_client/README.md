# python-xmlrpc-docker
Sample client program using XML-RPC to do RPC across multiple containers running chip stack.


Setup:
1. Build the connectedhomeip repo or the certification fork repo by running the following scripts in the scripts directory:
    To just build the container:`source scripts/build_container.sh`
    To build both the sdk and the container: `source scripts/build_sdk_container.sh`
2. Copy the chip wheel (.whl file) package from `out/debug/controller/python/` into this directory
3. Build & tag the server docker container using `docker build -t <image-name>`
4. Edit the rpc_client.py file and change the name of the image from chip-tool to the one entered in the previous step.
5. Edit the rpc calls and replace with the rpc calls that you wan to test. 
