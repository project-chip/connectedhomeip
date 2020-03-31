# Docker work environment

• Build a docker image (will have these pre-built soon)

```
cd tools/environment
docker build -f Dockerfile . -t chip-development
```

• Checkout your source to a personal location eg: `/home/MYUSER/source/connectedhomeip`
• Run the docker container

```
docker run --name chip-development -v /home/MYUSER/source/connectedhomeip:/home/developer/connectedhomeip  -it chip-development /bin/bash
```

