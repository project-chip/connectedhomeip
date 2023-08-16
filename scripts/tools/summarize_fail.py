import pandas as pd

df = pd.read_json("runlist.json")
print("Recent Failures:")
print(df)
print("Percentage Frequency:")
print(df["workflowName"].value_counts(normalize=True) * 100)
