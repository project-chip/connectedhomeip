import pandas as pd
import subprocess

subprocess.run("gh run list -R project-chip/connectedhomeip -b master -s failure --json databaseId,displayTitle,workflowName > run_list.json", shell=True)
df = pd.read_json("run_list.json")
df.columns = ["ID", "Pull Request", "Workflow"]
print("Recent Failures:")
print(df.to_string(index=False))
df.to_csv("recent_fails.csv", index=False)
print()
print("Percentage Frequency:")
frequency = df["Workflow"].value_counts(normalize=True).mul(100).astype(str).reset_index(name="Percentage")
print(frequency.to_string())
frequency.to_csv("recent_fails_frequency.csv")
print()
for failId in df["ID"].tolist():
  subprocess.run(f"gh run view -R project-chip/connectedhomeip {failId} --log-failed", shell=True)
