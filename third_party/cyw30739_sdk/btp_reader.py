#!/usr/bin/env python

import argparse
import json
import pathlib
import sys


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--btp", required=True, type=pathlib.Path)
    parser.add_argument("--enable_ota", action="store_true")

    option = parser.parse_args()

    items = {}
    with open(option.btp) as btp:
        for line in btp:
            item = line.strip().split("=")
            if len(item) == 2:
                key = item[0].strip()
                value = item[1].strip()

                if value.startswith('"'):
                    items[key] = value.strip('"')
                else:
                    items[key] = int(value, 0)

    items["XS_LOCATION_ACTIVE"] = 0x0052E000

    if option.enable_ota:
        items["XS_LOCATION_UPGRADE"] = 0x005A3000
    else:
        items["XS_LOCATION_UPGRADE"] = 0x00600000

    items["XIP_DS_OFFSET"] = items["XS_LOCATION_ACTIVE"] - \
        items["ConfigDSLocation"]
    items["XIP_LEN"] = items["XS_LOCATION_UPGRADE"] - \
        items["XS_LOCATION_ACTIVE"]

    for key in items:
        if type(items[key]) is int:
            items[key] = "0x{:08x}".format(items[key])

    print(json.dumps(items))
    return 0


if __name__ == "__main__":
    sys.exit(main())
