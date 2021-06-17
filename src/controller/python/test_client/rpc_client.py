import xmlrpc.client
import docker
import time
from docker.models.containers import Container
import asyncio
from asyncio import TimeoutError, wait_for
from typing import Optional

client = docker.from_env()

def destroy_container(container: Container) -> None:
    container.kill()
    container.remove()

async def container_running(container: Container) -> None:
    sleep_interval = 0.2

    while True:
        # Check if the container is running, then sleep for 0.1 sec
        if client.containers.get(container.id).status == "running":
            return

        # Sleep first to give container some time
        await asyncio.sleep(sleep_interval)

async def container_ready(container: Container) -> None:
    # Wait for the container for start running
    try:
        await wait_for(
            container_running(container), 5 #seconds
        )
    except TimeoutError as e:
        print("Container timed out")
        destroy_container(container)
        raise e

#Create docker containers
def run_new_container(docker_image_tag: str, port: int) -> Container:
    # Create containers
    try:
        # Note: These options are different from test harness, because the network topology is different
        # Test Harness has dedicated network on which we can find containers by hostname
        # This container is launched from the host directly and is bridged, hence we can easily use ports to connect
        mount_volumes = {"/var/run/dbus" : {'bind': '/var/run/dbus', 'mode': 'rw'}}
        return client.containers.run(
            docker_image_tag, detach=True, ports={5000:port}, volumes=mount_volumes, privileged=True
        )
    except Exception as error:
        print(
            "Error ocurred while creating a container from image " + str(error)
        )

def get_shortened_id(self, container: Container) -> str:
    # Docker containers are addressable by first 12 chars of the container id
    # https://github.com/docker/docker-py/issues/2660
    return container.id[:12]

async def create_container(docker_image_tag: str,  port: int) -> Container:
    container = run_new_container(docker_image_tag=docker_image_tag, port=port)
    if container is None:
        print("Unable to run container: " + docker_image_tag)
    await container_ready(container)
    print("Container running for " + docker_image_tag)

    return container

async def main():
    try: 
        container_1 = await create_container(docker_image_tag="chip-test", port=5050)
    except AttributeError as e:
        for container in client.containers.list():
            for tag in container.image.tags:
                if tag == "chip-test:latest":
                    print("WARNING:Container already running, killing the container, please try running the script again")
                    destroy_container(container)
                    exit()
    
    print("List of containers: " + str(client.containers.list()))

    # Wait for the controller to start the rpc server
    time.sleep(1)
    
    # Create an RPC server
    server_1 = xmlrpc.client.ServerProxy("http://localhost:5050/")
    discriminator = 3840
    pin_code = 20202021
    node_id = 1234

    # Invoke RPCs
    try: 
        print("Calling RPCs")
        print("echo_alive Response:" + server_1.echo_alive("Test"))
        scan = server_1.ble_scan()
        print(f"scan: {scan}")
        connect = server_1.ble_connect(discriminator, pin_code, node_id)
        print(f"connect: {connect}")
        pase_data = server_1.get_pase_data()
        print(f"pase_data: {pase_data}")
        fabric_id = server_1.get_fabric_id()
        print(f"fabric_id: {fabric_id}")

    except Exception as e:
        print(e)

    # Cleanup
    destroy_container(container_1)

if __name__ == "__main__":
    loop = asyncio.get_event_loop()
    loop.run_until_complete(main())
    loop.close()
