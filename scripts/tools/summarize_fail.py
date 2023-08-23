import logging
import os
import pandas as pd
from slugify import slugify
import subprocess

def process_fail(id, pr, workflow, start_time):
    logging.info("Building output file structure.")
    output_path = f"recent_fails_logs/{slugify(pr)}/{slugify(workflow)}/{slugify(start_time)}"
    os.makedirs(output_path)

    logging.info("Gathering raw fail logs.")
    subprocess.run(f"gh run view -R project-chip/connectedhomeip {id} --log-failed > {output_path}/fail_logs.txt", shell=True)

def main():
    logging.info("Gathering recent fails information into run_list.json.")
    subprocess.run("gh run list -R project-chip/connectedhomeip -b master -s failure --json databaseId,displayTitle,workflowName,startedAt > run_list.json", shell=True)

    logging.info("Reading run_list.json into a DataFrame.")
    df = pd.read_json("run_list.json")

    logging.info("Listing recent fails.")
    df.columns = ["ID", "Pull Request", "Workflow", "Start Time"]
    print("Recent Fails:")
    print(df.to_string(index=False))
    df.to_csv("recent_fails.csv", index=False)

    logging.info("Listing frequency of recent fails by workflow.")
    frequency = df["Workflow"].value_counts(normalize=True).mul(100).astype(str).reset_index(name="Percentage")  # Reformat this from "50.0" to "50%"
    print("Percentage Frequency of Fails by Workflow:")
    print(frequency.to_string(index=False))
    frequency.to_csv("recent_workflow_fails_frequency.csv")

    logging.info("Conducting fail information parsing.")
    df.apply(lambda row: process_fail(row["ID"], row["Pull Request"], row["Workflow"], row["Start Time"]), axis=1)

if __name__ == "__main__":
    main()
