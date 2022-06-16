import os
import hashlib


def gen_hash_from_dir(path: str) -> str:
  file_list = []
  for dirpath, dirnames, files in os.walk(path):
    for filename in files:
      file_list.append(os.path.join(dirpath, filename))
  file_list = sorted(file_list)
  md5hash = hashlib.md5()
  for filename in file_list:
    with open(filename, "rb") as f:
      md5hash.update(f.read())
  md5hash.hexdigest()
  return md5hash.hexdigest()

r = gen_hash_from_dir("zzz_generated/rootnode_contactsensor_DreXRHtsq9")

# f6f2fa3c57645a966e050e85f677283b 1st
# 8f9e1e73886366fc099802bba35503d6 2nd
