import pandas as pd

df = pd.read_json("runlist.json")
print(df.to_string())
