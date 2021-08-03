### Setup google cloud integration

Follow https://cloud.google.com/sdk/docs/install.

### Running builds

#### Local execution

In order to test locally, comment out the `machineType` entry in the build yaml.

```
# Once only setup:
gcloud components install cloud-build-local

# Build using this command from the checkout root
cloud-build-local --config integrations/cloudbuild/build-all.yaml --dryrun=false `pwd`
```

You can pass in optional argument of `--write-workspace=$HOME/tmp/testws` to
examine the output of the script.

#### Automated build location

Builds are set up to run automatically, with history available at:

https://console.cloud.google.com/cloud-build/builds?project=matter-build-automation

### Official build view permissions

Cloud build and artifact view permissions to the `matter-build-automation`
project are handled through a google group.

For active developers, please request to be included in the
`matter-open-source-developers-external@google.com` group.
