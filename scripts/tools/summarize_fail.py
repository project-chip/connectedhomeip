import pandas as pd

df = pd.read_json("runlist.json")
df.columns = ["Pull Request", "Workflow"]
print("Recent Failures:")
print(df.to_string(columns = ["Pull Request", "Workflow"], index = False))
print()
print("Percentage Frequency:")
print(df["Workflow"].value_counts(normalize=True).mul(100).astype(str) + "%")
