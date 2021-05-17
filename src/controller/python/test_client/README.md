# python-xmlrpc-docker
Sample client program using XML-RPC to do RPC across multiple containers running chip stack.


Setup:
1. Build the connectedhomeip repo or the certification fork repo by running the following scripts in the scripts directory:
    To just build the container:`source scripts/build_container.sh`
    To build both the sdk and the container: `source scripts/build_sdk_container.sh`
4. Edit the rpc_client.py file and add your rpc calls, i/o validation.
5. Run the python script using `python rpc_client.py`

Note:
 The script may emit warnings if the previous run did not cleanup the container properly 
