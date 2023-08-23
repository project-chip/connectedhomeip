import logging
import os
import pandas as pd
import subprocess

def process_fail(id, pr, workflow):
    os.makedirs(f"logs/{pr}/{workflow}")
    subprocess.run(f"gh run view -R project-chip/connectedhomeip {id} --log-failed > logs/{pr}/{workflow}/fail_logs.txt", shell=True)

def main():
    logging.info("Gathering recent run failure information into run_list.json.")
    subprocess.run("gh run list -R project-chip/connectedhomeip -b master -s failure --json databaseId,displayTitle,workflowName > run_list.json", shell=True)
    df = pd.read_json("run_list.json")
    df.columns = ["ID", "Pull Request", "Workflow"]
    logging.info("Recent Failures:")
    logging.info(df.to_string(index=False))
    df.to_csv("recent_fails.csv", index=False)
    logging.info("Percentage Frequency of Workflow Failures:")
    frequency = df["Workflow"].value_counts(normalize=True).mul(100).astype(str).reset_index(name="Percentage")
    logging.info(frequency.to_string(index=False))
    frequency.to_csv("recent_workflow_fails_frequency.csv")
    df.apply(lambda row: process_fail(row["ID"], row["Pull Request"], row["Workflow"]), axis=1)

if __name__ == "__main__":
    main()
