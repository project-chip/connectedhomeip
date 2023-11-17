## Deploy Static Website on App Engine

### Setup google cloud integration

Follow https://cloud.google.com/sdk/docs/install.

### Build Coverage Report

To check out the Matter repository:

```
git clone --recurse-submodules git@github.com:project-chip/connectedhomeip.git
```

Run the following command to build coverage report:

```
cd connectedhomeip
./scripts/build_coverage.sh
```

After a successful build, the coverage report is located at
`out/coverage/coverage`

#### Upload your static website to Google App Engine

File `integrations/appengine/webapp_config.yaml` is used to configure the
settings of your App Engine application.

Directory `out/coverage/coverage` contains the coverage report files, such as
HTML, CSS, images, and JavaScript.

```
cd out/coverage/coverage
gcloud app deploy ../../../integrations/appengine/webapp_config.yaml
```
