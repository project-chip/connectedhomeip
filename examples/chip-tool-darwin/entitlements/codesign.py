import argparse
import subprocess
import re


def find_identity():
    fail_str = "0 valid identities found"
    cmd = "/usr/bin/security find-identity -v -p codesigning"
    find_result = str(subprocess.check_output(cmd.split()))
    if fail_str in find_result:
        exit(-1)
    result = re.search(r'\b[0-9a-fA-F]{40}\b', find_result)
    return result.group()


def codesign(args):
    identity = find_identity()
    cmd = "codesign --force -d --sign {identity} --entitlements {entitlement} {target}".format(
        identity=identity,
        entitlement=args.entitlements_path,
        target=args.target_path)
    print("COMMAND " + cmd)
    codesign_result = str(subprocess.check_output(cmd.split()))
    print("Codesign Result: {}".format(codesign_result))
    with open(args.log_path, "w") as f:
        f.write(codesign_result)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Codesign the chip-tool-darwin binary')
    parser.add_argument('--entitlements_path', default='entitlements/chip-tool-darwin.entitlements',
                        help='Set the entitlements for codesign', required=True)
    parser.add_argument(
        '--log_path', help='Output log file destination', required=True)
    parser.add_argument('--target_path', help='Binary to sign', required=True)

    args = parser.parse_args()
    codesign(args)
