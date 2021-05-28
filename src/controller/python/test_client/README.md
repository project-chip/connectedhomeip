# Python client using xmlrpc and docker
Sample client program using XML-RPC to do RPC across multiple containers running chip stack.


**Setup:**
1. Install the pre-requisites mentioned [here](https://github.com/chip-csg/connectedhomeip/blob/master/docs/BUILDING.md) for linux and RaPi like so:

    `sudo apt-get install git gcc g++ python pkg-config libssl-dev libdbus-1-dev \
     libglib2.0-dev libavahi-client-dev ninja-build python3-venv python3-dev \
     python3-pip unzip libgirepository1.0-dev libcairo2-dev`

     `sudo apt-get install pi-bluetooth`
2. **Reboot** your RaPi after installing the pre-requisites. 
3. Build the connectedhomeip repo or the certification fork repo by running the following scripts in the scripts directory:
    To just build the container:`./scripts/build_container.sh`
    To build both the sdk and the container: `./source scripts/build_sdk_container.sh`
4. Edit the rpc_client.py file and add your rpc calls, i/o validation.
5. Run the python script using `python3 rpc_client.py`

**Note:**
 - The script may emit warnings if the previous run did not cleanup the container properly.
