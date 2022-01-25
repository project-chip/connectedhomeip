#!/usr/bin/env python

import json
import sys


def main():
    btp_file = sys.argv[1]

    items = {}
    with open(btp_file) as btp:
        for line in btp:
            item = line.strip().split("=")
            if len(item) == 2:
                key = item[0].strip()
                value = item[1].strip()
                items[key] = value

    items["XIP_DS_OFFSET"] = "0x0001e000"
    items["XIP_LEN"] = "0x{:08x}".format(
        int(items["ConfigDS2Location"], 16)
        - int(items["ConfigDSLocation"], 16)
        - int(items["XIP_DS_OFFSET"], 16)
    )

    print(json.dumps(items))
    return 0


if __name__ == "__main__":
    sys.exit(main())
