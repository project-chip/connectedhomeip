import pandas as pd

df = pd.read_json("runlist.json")
df.columns = ["Pull Request", "Workflow"]
print("Recent Failures:")
print(df.to_string(columns = ["Pull Request", "Workflow"], index = False))
print()
print("Percentage Frequency:")
print(df["workflowName"].value_counts(normalize=True) * 100)
