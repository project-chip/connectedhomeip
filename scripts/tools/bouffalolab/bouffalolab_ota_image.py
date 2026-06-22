#!/usr/bin/env python3
#
#    Copyright (c) 2026 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

import argparse
import glob
import logging
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

MATTER_ROOT = Path(__file__).resolve().parents[3]

sys.path.insert(0, str(MATTER_ROOT / "src" / "app"))
import ota_image_tool  # noqa: E402

AUTO_OTA_ENV = "BOUFFALOLAB_AUTO_OTA"
ENV_NAMES = {
    "vendor_id": ("BOUFFALOLAB_OTA_VENDOR_ID", "CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID"),
    "product_id": ("BOUFFALOLAB_OTA_PRODUCT_ID", "CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID"),
    "version": ("BOUFFALOLAB_OTA_VERSION", "CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION"),
    "version_str": ("BOUFFALOLAB_OTA_VERSION_STR", "CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING"),
    "digest_algorithm": ("BOUFFALOLAB_OTA_DIGEST_ALGORITHM",),
}
HEADER_MACROS = {
    "vendor_id": "CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID",
    "product_id": "CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID",
    "version": "CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION",
    "version_str": "CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING",
}

log = logging.getLogger(__name__)


def _any_base_int(value):
    return int(str(value).strip("() "), 0)


def _strip_c_string(value):
    value = value.strip()
    if len(value) >= 2 and value[0] == '"' and value[-1] == '"':
        return value[1:-1]
    return value


def _read_project_config(path):
    values = {}
    if not path or not os.path.exists(path):
        return values

    define_re = re.compile(r"^\s*#\s*define\s+(\w+)\s+(.+?)\s*(?://.*)?$")
    with open(path, encoding="utf-8") as config:
        for line in config:
            match = define_re.match(line)
            if not match:
                continue
            name, value = match.groups()
            for key, macro in HEADER_MACROS.items():
                if name == macro and key not in values:
                    values[key] = _strip_c_string(value)
    return values


def _metadata(project_config):
    values = {"digest_algorithm": "sha256"}
    values.update(_read_project_config(project_config))

    for key, names in ENV_NAMES.items():
        for name in names:
            if name in os.environ and os.environ[name] != "":
                values[key] = os.environ[name]
                break

    missing = [key for key in ("vendor_id", "product_id", "version", "version_str") if key not in values]
    if missing:
        raise ValueError(
            "Missing OTA metadata: %s. Set BOUFFALOLAB_OTA_* environment variables or define them in CHIPProjectConfig.h."
            % ", ".join(missing)
        )

    values["vendor_id"] = _any_base_int(values["vendor_id"])
    values["product_id"] = _any_base_int(values["product_id"])
    values["version"] = _any_base_int(values["version"])
    values["version_str"] = str(values["version_str"])
    values["digest_algorithm"] = str(values["digest_algorithm"])
    return values


def _auto_ota_enabled():
    value = os.environ.get(AUTO_OTA_ENV, "1").strip().lower()
    return value not in ("0", "false", "no", "off")


def _metadata_suffix(metadata):
    version_str = re.sub(r"[^A-Za-z0-9._-]+", "_", metadata["version_str"]).strip("._-")
    if not version_str:
        version_str = "unknown"

    return "_vid-0x%04x_pid-0x%04x_vn-%s_vs-%s" % (
        metadata["vendor_id"],
        metadata["product_id"],
        metadata["version"],
        version_str,
    )


def _payload_output_name(payload_path, metadata):
    payload = Path(payload_path)
    name = payload.name
    metadata_suffix = _metadata_suffix(metadata)
    for suffix in (".xz.ota", ".xz.hash", ".ota", ".hash"):
        if name.endswith(suffix):
            stem = name[: -len(suffix)]
            if stem.endswith(metadata_suffix):
                return name
            return "%s%s%s" % (stem, metadata_suffix, suffix)
    if name.endswith(metadata_suffix):
        return name
    return "%s%s" % (name, metadata_suffix)


def _matter_output_name(payload_path, metadata):
    payload = Path(payload_path)
    name = payload.name
    if name.endswith(".matter"):
        name = name[: -len(".matter")]
    return "%s.matter" % _payload_output_name(name, metadata)


def _ensure_clean_ota_dir(work_dir):
    ota_dir = Path(work_dir) / "ota_images"
    if ota_dir.exists():
        shutil.rmtree(ota_dir)
    ota_dir.mkdir(parents=True, exist_ok=True)
    return ota_dir


def _wrap_payload(payload, output_file, metadata):
    args = argparse.Namespace(
        vendor_id=metadata["vendor_id"],
        product_id=metadata["product_id"],
        version=metadata["version"],
        version_str=metadata["version_str"],
        digest_algorithm=metadata["digest_algorithm"],
        min_version=None,
        max_version=None,
        release_notes=None,
        input_files=[str(payload)],
        output_file=str(output_file),
    )
    ota_image_tool.validate_header_attributes(args)
    ota_image_tool.generate_image(args)
    return args.output_file


def _run_flash_wrapper(flash_script, metadata):
    cmd = [
        sys.executable,
        str(flash_script),
        "--build-ota",
        "--vendor-id",
        str(metadata["vendor_id"]),
        "--product-id",
        str(metadata["product_id"]),
        "--version",
        str(metadata["version"]),
        "--version-str",
        metadata["version_str"],
        "--digest-algorithm",
        metadata["digest_algorithm"],
    ]
    subprocess.run(cmd, cwd=MATTER_ROOT, check=True, stderr=subprocess.STDOUT)


def build_iot_sdk_ota(flash_script, project_config):
    metadata = _metadata(project_config)
    flash_script = Path(flash_script)
    _run_flash_wrapper(flash_script, metadata)

    ota_dir = flash_script.parent / "ota_images"
    payloads = sorted(path for path in ota_dir.iterdir() if path.suffix in (".ota", ".hash"))
    generated = sorted(ota_dir.glob("*.matter"))
    if not payloads and not generated:
        raise FileNotFoundError("No OTA images generated in %s" % ota_dir)

    renamed = []
    for payload in payloads:
        old_matter = Path("%s.matter" % payload)
        matter_target = ota_dir / _matter_output_name(payload, metadata)

        if old_matter.exists():
            if old_matter != matter_target and matter_target.exists():
                matter_target.unlink()
            if old_matter != matter_target:
                old_matter.rename(matter_target)
            renamed.append(str(matter_target))
        else:
            renamed.append(_wrap_payload(payload, matter_target, metadata))

        payload.unlink()

    for image in generated:
        if not image.exists():
            continue
        target = ota_dir / _matter_output_name(image.with_suffix(""), metadata)
        if image != target:
            if target.exists():
                target.unlink()
            image.rename(target)
        renamed.append(str(target))
    return renamed


def build_bouffalo_sdk_ota(build_out_dir, output_dir, project_config):
    metadata = _metadata(project_config)
    ota_dir = _ensure_clean_ota_dir(output_dir)
    payloads = sorted(glob.glob(str(Path(build_out_dir) / "*.ota")))
    if not payloads:
        raise FileNotFoundError("No Bouffalo SDK OTA payloads found in %s" % build_out_dir)

    generated = []
    for payload in payloads:
        output_file = ota_dir / _matter_output_name(payload, metadata)
        generated.append(_wrap_payload(payload, output_file, metadata))
    return generated


def main():
    parser = argparse.ArgumentParser(description="Generate Bouffalo Matter OTA images after a firmware build.")
    parser.add_argument("--mode", choices=("iot-sdk", "bouffalo-sdk"), required=True)
    parser.add_argument("--flash-script", help="Generated Bouffalo flash.py wrapper, for IoT SDK builds.")
    parser.add_argument("--build-out-dir", help="Bouffalo SDK build_out directory containing *.ota payloads.")
    parser.add_argument("--output-dir", help="Directory where ota_images should be created.")
    parser.add_argument("--project-config", required=True, help="CHIPProjectConfig.h fallback metadata source.")
    args = parser.parse_args()

    logging.basicConfig(level=logging.INFO, format="%(levelname)s %(message)s", stream=sys.stdout)
    if not _auto_ota_enabled():
        log.info("Bouffalo auto OTA generation disabled by %s", AUTO_OTA_ENV)
        return 0

    if args.mode == "iot-sdk":
        if not args.flash_script:
            parser.error("--flash-script is required for iot-sdk mode")
        generated = build_iot_sdk_ota(args.flash_script, args.project_config)
    else:
        if not args.build_out_dir or not args.output_dir:
            parser.error("--build-out-dir and --output-dir are required for bouffalo-sdk mode")
        generated = build_bouffalo_sdk_ota(args.build_out_dir, args.output_dir, args.project_config)

    for image in generated:
        log.info("Matter OTA image generated: %s", image)

    return 0


if __name__ == "__main__":
    sys.exit(main())
