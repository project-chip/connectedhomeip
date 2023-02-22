#!/usr/bin/env python3

# Copyright (c) 2022 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import argparse
import logging
import os
import subprocess
import sys

# Absolute path to Tizen Studio CLI tool.
tizen_sdk_root = os.environ["TIZEN_SDK_ROOT"]
tizen_cli = os.path.join(tizen_sdk_root, "tools", "ide", "bin", "tizen")

# Setup basic logging capabilities.
logging.basicConfig(level=logging.DEBUG)


def create_author_certificate(alias: str, password: str,
                              name: str = "", email: str = ""):
    cmd = [tizen_cli, "certificate", "--alias", alias, "--password", password]
    if name:
        cmd.extend(["--name", name])
    if email:
        cmd.extend(["--email", email])
    logging.debug("Execute: %s", " ".join(cmd))
    with subprocess.Popen(cmd, stdout=subprocess.PIPE) as proc:
        for line in proc.stdout.readlines():
            line = line.decode().rstrip()
            if line.startswith("Working path:"):
                wd = line[len("Working path:"):].strip()
            print(line)
    return os.path.join(wd, "author.p12")


def check_security_profile(profile):

    # XXX: If Tizen Studio SDK data directory was removed but the config file
    #      was not, Tizen Studio CLI does not create profiles XML file. This
    #      is a workaround to create a dummy XML profiles file so later Tizen
    #      will regenerate it with the correct content.

    tizen_sdk_data_dir = None
    with open(os.environ["TIZEN_SDK_ROOT"] + "/sdk.info") as f:
        for line in f.readlines():
            if line.startswith("TIZEN_SDK_DATA_PATH"):
                tizen_sdk_data_dir = line.split("=")[1].strip()
                break
    if not tizen_sdk_data_dir:
        logging.error("Cannot find Tizen SDK data directory")
        return False

    profiles_xml = os.path.join(tizen_sdk_data_dir, "profile", "profiles.xml")
    if not os.path.exists(profiles_xml):
        os.makedirs(os.path.dirname(profiles_xml), exist_ok=True)
        with open(profiles_xml, "w") as f:
            f.write('<profiles/>')

    cmd = [tizen_cli, "security-profiles", "list", "--name", profile]
    logging.debug("Execute: %s", " ".join(cmd))
    with subprocess.Popen(cmd, stdout=subprocess.PIPE) as proc:
        for line in proc.stdout.readlines():
            line = line.decode().rstrip()
            print(line)
        return proc.wait() == 0


def add_security_profile(profile: str, certificate: str, password: str):
    cmd = [tizen_cli, "security-profiles", "add", "--active",
           "--name", profile, "--author", certificate, "--password", password]
    logging.debug("Execute: %s", " ".join(cmd))
    with subprocess.Popen(cmd, stdout=subprocess.PIPE) as proc:
        for line in proc.stdout.readlines():
            line = line.decode().rstrip()
            print(line)
        return proc.wait() == 0


def update_stamp_file(path: str, message: str):
    if path:
        with open(path, "w") as f:
            f.write(message + "\n")


parser = argparse.ArgumentParser(
    description="Setup Tizen Studio development security profile.")
parser.add_argument(
    '--author-certificate-name', metavar='NAME',
    help="Set author certificate 'name' field.")
parser.add_argument(
    '--author-certificate-email', metavar='EMAIL',
    help="Set author certificate 'email' field.")
parser.add_argument(
    '--author-certificate-password', metavar='PASSWORD', required=True,
    help="Password for author certificate.")
parser.add_argument(
    '--sign-security-profile', metavar='NAME', required=True,
    help="Name of the security profile to add.")
parser.add_argument(
    '--stamp-file', metavar='FILE',
    help="Update the stamp file upon success.")

args = parser.parse_args()

rv = check_security_profile(args.sign_security_profile)
if rv:
    update_stamp_file(args.stamp_file, "Using existing security profile.")
    logging.info("Security profile already exists")
    sys.exit()

# Create author certificate if it does not exist. If the certificate already
# exists, it will be used. However, if the password is different, it will not
# be possible to use the certificate when updating the security profile.
cert = create_author_certificate("CHIP",
                                 args.author_certificate_password,
                                 args.author_certificate_name,
                                 args.author_certificate_email)
if not cert:
    logging.error("Failed to create author certificate")
    sys.exit(1)

rv = add_security_profile(args.sign_security_profile, cert,
                          args.author_certificate_password)
if not rv:
    logging.error("Failed to add security profile")
    sys.exit(1)

update_stamp_file(args.stamp_file, "New security profile created.")
sys.exit()
