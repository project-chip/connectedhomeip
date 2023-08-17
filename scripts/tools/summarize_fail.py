import pandas as pd

df = pd.read_json("run_list.json")
df.columns = ["Pull Request", "Workflow"]
print("Recent Failures:")
print(df.to_string(index=False))
df.to_csv("recent_fails.csv", index=False)
print()
print("Percentage Frequency:")
frequency = df["Workflow"].value_counts(normalize=True).mul(100).astype(str) + "%"
print(frequency.to_string())
frequency.to_csv("recent_fails_frequency.csv")
