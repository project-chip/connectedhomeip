## Startup Script of Compute Engine

A startup script is a file that contains commands that run when a virtual
machine instance boots. Compute Engine provides support for running startup
scripts on Linux and Windows virtual machines.

### Create a virtual machine instance using startup script

The `startup-script.sh` could be used as the startup script of a virtual machine
instance which run Matter coverage report and publish the result via an App
Engine service.

You can create a virtual machine instance by using the gcloud compute instances
create command with the `--metadata-from-file` flag.

```
gcloud compute instances create VM_NAME \
  --image-project=PROJECT_NAME \
  --image-family=ubuntu-22.04 \
  --metadata-from-file=startup-script=FILE_PATH
```

Replace the following:

`PROJECT_NAME`: the name of the project host the virtual machine instance

`VM_NAME`: the name of the virtual machine instance

`FILE_PATH`: the relative path to the startup script file
