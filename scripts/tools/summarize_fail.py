import logging
import os
import subprocess

import pandas as pd
from slugify import slugify

error_catalog = {
    "CodeQL": {
        "No space left on device": {
            "short": "Ran out of space",
            "detail": "Exception with signature \"No space left on device\""
        },
        "Check that the disk containing the database directory has ample free space.": {
            "short": "Ran out of space",
            "detail": "Fatal internal error with message indicating that disk space most likely ran out"
        }
    },
    "Build example": {
        "Could not find a version that satisfies the requirement": {
            "short": "Requirements issue",
            "detail": "Unable to install a requirements in Python requirements.txt"
        },
        "No module named": {
            "short": "Missing module",
            "detail": "Expected module was missing"
        }
    },
    "Full builds": {
        "No space left on device": {
            "short": "Ran out of space",
            "detail": "Exception with signature \"No space left on device\""
        }
    }
}


def process_fail(id, pr, start_time, workflow):
    logging.info(f"Processing failure in {pr}, workflow {workflow} that started at {start_time}.")

    logging.info("Building output file structure.")
    output_path = f"recent_fails_logs/{slugify(pr)}/{slugify(workflow)}/{slugify(start_time)}"
    os.makedirs(output_path)

    logging.info("Gathering raw fail logs.")
    subprocess.run(f"gh run view -R project-chip/connectedhomeip {id} --log-failed > {output_path}/fail_log.txt", shell=True)

    # Eventually turn this into a catalog of error messages per workflow
    logging.info("Collecting info on likely cause of failure.")
    root_cause = "Unknown cause"
    with open(f"{output_path}/fail_log.txt") as fail_log_file:
        fail_log = fail_log_file.read()
        workflow_category = workflow.split(" - ")[0]
        if workflow_category in error_catalog:
            for error_message in error_catalog[workflow_category]:
                if error_message in fail_log:
                    root_cause = error_catalog[workflow_category][error_message]["short"]
                    break

    logging.info(f"Checking recent pass/fail rate of workflow {workflow}.")
    workflow_fail_rate_output_path = f"workflow_pass_rate/{slugify(workflow)}"
    if not os.path.exists(workflow_fail_rate_output_path):
        os.makedirs(workflow_fail_rate_output_path)
        subprocess.run(f"gh run list -R project-chip/connectedhomeip -b master -w '{workflow}' --json conclusion > {workflow_fail_rate_output_path}/run_list.json", shell=True)
    else:
        logging.info("This workflow has already been processed.")
    
    return [pr, workflow, root_cause]


def main():
    logging.info("Gathering recent fails information into run_list.json.")
    subprocess.run("gh run list -R project-chip/connectedhomeip -b master -s failure --json databaseId,displayTitle,startedAt,workflowName > run_list.json", shell=True)

    logging.info("Reading run_list.json into a DataFrame.")
    df = pd.read_json("run_list.json")

    logging.info("Listing recent fails.")
    df.columns = ["ID", "Pull Request", "Start Time", "Workflow"]
    print("Recent Fails:")
    print(df.to_string(columns=["Pull Request", "Workflow"], index=False))
    print()
    df.to_csv("recent_fails.csv", index=False)

    logging.info("Listing frequency of recent fails by workflow.")
    frequency = df["Workflow"].value_counts(normalize=True).mul(100).astype(
        str).reset_index(name="Percentage")  # Reformat this from "50.0" to "50%"
    print("Share of Recent Fails by Workflow:")
    print(frequency.to_string(index=False))
    print()
    frequency.to_csv("recent_workflow_fails_frequency.csv")

    logging.info("Conducting fail information parsing.")
    root_causes = df.apply(lambda row: process_fail(row["ID"], row["Pull Request"],
                           row["Start Time"], row["Workflow"]), axis=1, result_type="expand")
    root_causes.columns = ["Pull Request", "Workflow", "Cause of Failure"]
    print("Likely Root Cause of Recent Fails:")
    print(root_causes.to_string(index=False))
    print()
    root_causes.to_csv("failure_cause_summary.csv")

    logging.info("Listing percent fail rate of recent fails by workflow.")
    fail_rate = {}
    for workflow in next(os.walk("workflow_pass_rate"))[1]:
        try:
            info = pd.read_json(f"workflow_pass_rate/{workflow}/run_list.json")
            info = info[info["conclusion"].str.len() > 0]
            fail_rate[workflow] = [info.value_counts(normalize=True)["failure"] * 100]
        except:
            logging.error(f"Recent runs info for {workflow} was not collected.")
    fail_rate = pd.DataFrame.from_dict(fail_rate, 'index', columns=["Fail Rate"])
    print("Recent Fail Rate of Each Workflow:")
    print(fail_rate.to_string())
    fail_rate.to_csv("workflow_fail_rate.csv")

if __name__ == "__main__":
    main()
