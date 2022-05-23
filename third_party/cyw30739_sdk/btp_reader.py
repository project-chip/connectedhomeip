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

    ds_len = items["ConfigDS2Location"] - items["ConfigDSLocation"]
    xs_location_end = 0x00600000

    items["ConfigXS1Location"] = items["ConfigDS2Location"] + ds_len

    if option.enable_ota:
        items["ConfigXS1Length"] = 0x00076000
        items["ConfigXS2Location"] = (
            items["ConfigXS1Location"] + items["ConfigXS1Length"]
        )
        items["ConfigXS2Length"] = xs_location_end - items["ConfigXS2Location"]
    else:
        items["ConfigXS1Length"] = xs_location_end - items["ConfigXS1Location"]
        items["ConfigXS2Length"] = 0
        items["ConfigXS2Location"] = xs_location_end

    items["ConfigXS1DS1Offset"] = items["ConfigXS1Location"] - \
        items["ConfigDSLocation"]

    for key in items:
        if type(items[key]) is int:
            items[key] = "0x{:08x}".format(items[key])

    print(json.dumps(items))
    return 0


if __name__ == "__main__":
    sys.exit(main())
