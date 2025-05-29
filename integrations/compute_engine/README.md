## Google Cloud Compute Engine

We have setup a Virtual Machine on
[Google Cloud](https://cloud.google.com/products/compute) to generate both the
[Matter SDK coverage report](https://matter-build-automation.ue.r.appspot.com)
and the
[Matter SDK Conformance report](https://matter-build-automation.ue.r.appspot.com/conformance_report.html).

### The Matter SDK Virtual Machine and the "startup-script.sh"

We created a VM named `matter-build-coverage`. The machine configuration is
located
[here](https://pantheon.corp.google.com/compute/instancesDetail/zones/us-central1-a/instances/matter-build-coverage?inv=1&invt=AbnAfg&project=matter-build-automation).
Reach out to Google team members if you need to make changes to this VM.

This virtual machine is scheduled to run daily, starting at 11:45PM and stopping
at 2am. During boot, the machine runs the `startup-script.sh`.

The `startup-script.sh` script contains commands to checkout the SDK repository
and create both the SDK coverage report and conformance report. The startup
script uses `scripts/build_coverage.sh` to generate the coverage report and
`scripts/examples/conformance_report.py` to generate the conformance report. The
resulting HTML files are published via an App Engine service and available here
([coverage report](https://matter-build-automation.ue.r.appspot.com/),
[conformance report](https://matter-build-automation.ue.r.appspot.com/conformance_report.html)).

### Making Changes to "startup-script.sh"

If you make changes to `startup-script.sh`, make sure you go to the
[VM configuration](https://pantheon.corp.google.com/compute/instancesDetail/zones/us-central1-a/instances/matter-build-coverage?inv=1&invt=AbnAfg&project=matter-build-automation),
click `edit` and update the startup script in the `Automation` text box, to
reflect your changes. The script in the Matter SDK repo is just a copy of the
configuration in the VM.
