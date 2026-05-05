## Google Cloud Compute Engine

We have setup a Virtual Machine on
[Google Cloud](https://cloud.google.com/products/compute) to generate and host
various reports for the Matter SDK.

### Automated Reports

The reporting process is driven by scripts in the repository, minimizing the
need to update the VM configuration directly.

#### Scripts

-   **`integrations/compute_engine/automated_reports.sh`**: The master script
    that runs the selected reports, generates a landing page, and optionally
    deploys to App Engine.
-   **`integrations/compute_engine/run_alchemy_diff.sh`**: A specialized script
    to run the Alchemy diff report. It handles downloading the Alchemy binary,
    cloning the spec repository, and generating the diff. This script is called
    by the master script but can be run independently.

#### Available Reports

-   **Coverage Report**: Code coverage results for the Matter SDK.
-   **Conformance Report**: Cluster conformance test results.
-   **SDK vs Spec Diff**: Differences between SDK ZAP XMLs and the specification
    (using Alchemy).

### Local Testing

You can run the reports locally to verify changes or view the results without
deploying.

**Important:** Scripts must be executed from the root of the repository.

Example: Run the conformance and alchemy reports and view them locally:

```bash
./integrations/compute_engine/automated_reports.sh --conformance --alchemy --serve
```

Flags:

-   `--coverage`: Run coverage report.
-   `--conformance`: Run conformance report.
-   `--alchemy`: Run Alchemy diff report.
-   `--all`: Run all reports.
-   `--serve` or `--test`: Start a local web server at `http://localhost:8000`
    after generation.
-   `--deploy`: Deploy to App Engine (requires `gcloud` configured).
-   `--pat-secret NAME`: Specify the secret name for GitHub PAT (needed for
    Alchemy if spec repo is private).

### The Virtual Machine and "startup-script.sh"

We created a VM named `matter-build-coverage`. The machine configuration is
located
[here](https://pantheon.corp.google.com/compute/instancesDetail/zones/us-central1-a/instances/matter-build-coverage?inv=1&invt=AbnAfg&project=matter-build-automation).

This VM is scheduled to run daily. During boot, it runs `startup-script.sh`,
which clones the repository and runs:

```bash
./integrations/compute_engine/automated_reports.sh --all --deploy --pat-secret github-read-only-pat
```

The resulting files are published via an App Engine service:

-   **Hub Page**: `https://matter-build-automation.ue.r.appspot.com/`
-   **Coverage**:
    `https://matter-build-automation.ue.r.appspot.com/coverage/index.html`
-   **Conformance**:
    `https://matter-build-automation.ue.r.appspot.com/conformance_report.html`
-   **ZAP Diff**:
    `https://matter-build-automation.ue.r.appspot.com/sdk_spec_zapdiff.html`

### Making Changes

-   **To reporting logic**: Modify `automated_reports.sh` or
    `run_alchemy_diff.sh` in the repository.
-   **To VM startup behavior**: Go to the
    [VM configuration](https://pantheon.corp.google.com/compute/instancesDetail/zones/us-central1-a/instances/matter-build-coverage?inv=1&invt=AbnAfg&project=matter-build-automation),
    click `edit`, and update the startup script in the `Automation` text box.
