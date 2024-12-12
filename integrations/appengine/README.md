## Deploy Static Website on App Engine for The CHIP Coverage Report

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

Deploying your generated report:

```
cd out/coverage/coverage
cp ../../../integrations/appengine/webapp_config.yaml ../../../integrations/appengine/.gcloudignore .
gcloud app deploy webapp_config.yaml --project matter-build-automation
```

The output should look like:

```
Services to deploy:

descriptor:                  [/usr/local/google/home/<user>/connectedhomeip/out/coverage/coverage/webapp_config.yaml]
source:                      [/usr/local/google/home/<user>/connectedhomeip/out/coverage/coverage]
target project:              [matter-build-automation]
target service:              [default]
target version:              [20241212t175429]
target url:                  [https://matter-build-automation.ue.r.appspot.com]
target service account:      [matter-build-automation@appspot.gserviceaccount.com]


Do you want to continue (Y/n)?  Y

Beginning deployment of service [default]...
╔════════════════════════════════════════════════════════════╗
╠═ Uploading 0 files to Google Cloud Storage                ═╣
╚════════════════════════════════════════════════════════════╝
File upload done.
Updating service [default]...done.
Setting traffic split for service [default]...done.
Deployed service [default] to [https://matter-build-automation.ue.r.appspot.com]

You can stream logs from the command line by running:
  $ gcloud app logs tail -s default

To view your application in the web browser run:
  $ gcloud app browse --project=matter-build-automation
```

If you run into permission issues, reach out to a team member from Google.
