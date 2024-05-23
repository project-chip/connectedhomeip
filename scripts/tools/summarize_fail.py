import datetime
import logging
import os
import sqlite3
import subprocess

import pandas as pd
import yaml
from slugify import slugify

yesterday = (datetime.datetime.now() - datetime.timedelta(days=1)).strftime('%Y-%m-%d')

with open("scripts/tools/build_fail_definitions.yaml", "r") as fail_defs:
    try:
        error_catalog = yaml.safe_load(fail_defs)
    except Exception:
        logging.exception("Could not load fail definition file.")


def pass_fail_rate(workflow):
    logging.info(f"Checking recent pass/fail rate of workflow {workflow}.")
    workflow_fail_rate_output_path = f"workflow_pass_rate/{slugify(workflow)}"
    if not os.path.exists(workflow_fail_rate_output_path):
        os.makedirs(workflow_fail_rate_output_path)
        subprocess.run(
            f"gh run list -R project-chip/connectedhomeip -b master -w '{workflow}' -L 500 --created {yesterday} --json conclusion > {workflow_fail_rate_output_path}/run_list.json", shell=True)
    else:
        logging.info("This workflow has already been processed.")


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

    return [pr, workflow, root_cause]


def main():
    logging.info("Gathering recent fails information into fail_run_list.json.")
    subprocess.run(
        f"gh run list -R project-chip/connectedhomeip -b master -s failure -L 500 --created {yesterday} --json databaseId,displayTitle,startedAt,workflowName > fail_run_list.json", shell=True)

    logging.info("Reading fail_run_list.json into a DataFrame.")
    df = pd.read_json("fail_run_list.json")

    logging.info(f"Listing recent fails from {yesterday}.")
    df.columns = ["ID", "Pull Request", "Start Time", "Workflow"]
    print(f"Recent Fails From {yesterday}:")
    print(df.to_string(columns=["Pull Request", "Workflow"], index=False))
    print()
    df.to_csv("recent_fails.csv", index=False)

    logging.info("Listing frequency of recent fails by workflow.")
    frequency = df["Workflow"].value_counts(normalize=True).mul(100).round().astype(
        str).reset_index(name="Percentage")  # Reformat this from "50.0" to "50%"
    print("Share of Recent Fails by Workflow:")
    print(frequency.to_string(index=False))
    print()
    frequency.to_csv("recent_workflow_fails_frequency.csv")

    logging.info("Gathering recent runs information into all_run_list.json.")
    subprocess.run(
        f"gh run list -R project-chip/connectedhomeip -b master -L 5000 --created {yesterday} --json workflowName > all_run_list.json", shell=True)

    logging.info("Reading all_run_list.json into a DataFrame.")
    all_df = pd.read_json("all_run_list.json")

    logging.info("Gathering pass/fail rate info.")
    all_df.columns = ["Workflow"]
    all_df.apply(lambda row: pass_fail_rate(row["Workflow"]), axis=1)

    logging.info("Conducting fail information parsing.")
    root_causes = df.apply(lambda row: process_fail(row["ID"], row["Pull Request"],
                           row["Start Time"], row["Workflow"]), axis=1, result_type="expand")
    root_causes.columns = ["Pull Request", "Workflow", "Cause of Failure"]
    print("Likely Root Cause of Recent Fails:")
    print(root_causes.to_string(index=False))
    print()
    root_causes.to_csv("failure_cause_summary.csv")

    logging.info("Listing percent pass rate of recent fails by workflow.")
    pass_rate = {}
    for workflow in next(os.walk("workflow_pass_rate"))[1]:
        try:
            info = pd.read_json(f"workflow_pass_rate/{workflow}/run_list.json")
            info = info[info["conclusion"].str.len() > 0]
        except Exception:
            logging.exception(f"Recent runs info for {workflow} was not collected.")
        try:
            pass_rate[workflow] = [info.value_counts(normalize=True).mul(100).round()["success"]]
        except Exception:
            pass_rate[workflow] = [0.0]
    pass_rate = pd.DataFrame.from_dict(pass_rate, 'index', columns=["Pass Rate"]).sort_values("Pass Rate")
    print("Recent Pass Rate of Each Workflow:")
    pass_rate.to_markdown("docs/daily_pass_percentage.md")
    pass_rate_sql = sqlite3.connect("workflow_pass_rate.sqlite3")
    pass_rate.to_sql("workflow_pass_rate", pass_rate_sql, if_exists="replace")
    print(pass_rate.to_string())
    pass_rate.to_csv("workflow_pass_rate.csv")


if __name__ == "__main__":
    main()
