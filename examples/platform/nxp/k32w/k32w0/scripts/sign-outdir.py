import argparse
import os
import subprocess


def main(args):
    if "NXP_K32W0_SDK_ROOT" in os.environ and os.environ["NXP_K32W0_SDK_ROOT"] != "":
        sign_images_path = os.environ["NXP_K32W0_SDK_ROOT"] + "/tools/imagetool/sign_images.sh"
    else:
        sign_images_path = os.path.abspath(
            __file__ + "/../../../../../../../third_party/nxp/k32w0_sdk/repo/core/tools/imagetool/sign_images.sh")

    # Give execute permission if needed
    if os.access(sign_images_path, os.X_OK) is False:
        os.chmod(sign_images_path, 0o766)

    # Convert script to unix format if needed
    subprocess.call("(file " + sign_images_path + " | grep CRLF > /dev/null) && (dos2unix " + sign_images_path + ")", shell=True)

    # Call sign_images.sh script with the output directory
    cmd = sign_images_path + " " + os.getcwd()
    if args.simple_hash:
        cmd = cmd + " -SimpleHashVerification"

    subprocess.call(cmd, shell=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--simple-hash",
        help="When enabled, adds a hash of the whole image at the end of the binary.",
        action="store_true"
    )
    args = parser.parse_args()

    main(args)
