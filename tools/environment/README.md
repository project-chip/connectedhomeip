# Docker work environment
• Install Docker on your host platform using instructions from: <https://docs.docker.com/install/>

• Build a Docker image (will have these pre-built soon)

```
cd tools/environment
docker build -f Dockerfile . -t chip-development
```

• Checkout your source to a personal location eg: `/home/MYUSER/source/connectedhomeip`
• Run the Docker container

```
docker run --name chip-development -v /home/MYUSER/source/connectedhomeip:/home/developer/connectedhomeip  -it chip-development /bin/bash
```

