#!/usr/bin/env python3
"""
Matter Telink Manual Firmware Processing Tool
Location: scripts/tools/telink/manual_process.py
Co-located with process_binaries.py for consistent project structure

This script manually executes the complete firmware processing pipeline,
fully reusing existing project logic. All generated files are prefixed
with 'manual_' to avoid conflicts with automated build outputs.

=== Usage Examples ===

Usage 1 (process raw firmware, full pipeline):
  python3 manual_process.py zephyr.bin <config_path> [--output-dir OUTPUT_DIR]
  Example:
    python3 manual_process.py \\
        /home/ubuntu/connectedhomeip/examples/light-switch-app/telink/build_tl3238x_retention_dual_sw_v1/zephyr/zephyr.bin \\
        /home/ubuntu/connectedhomeip/examples/light-switch-app/telink/build_tl3238x_retention_dual_sw_v1/zephyr/.config

Usage 2 (process already signed firmware, start from compression):
  python3 manual_process.py zephyr.signed.bin <config_path> --already-signed
  Example:
    python3 manual_process.py \\
        /home/ubuntu/connectedhomeip/examples/light-switch-app/telink/build_tl3238x_retention_dual_sw_v1/zephyr/zephyr.signed.bin \\
        /home/ubuntu/connectedhomeip/examples/light-switch-app/telink/build_tl3238x_retention_dual_sw_v1/zephyr/.config --already-signed

Usage 3 (with manual parameters, no .config file needed):
  python3 manual_process.py <firmware_path> [--output-dir OUTPUT_DIR] \\
      --rom-start-offset 512 --flash-load-size 1179648 \\
      --lzma-dict-size 32768 --signing-key bootloader/mcuboot/root-rsa-2048.pem \\
      --vendor-id 65521 --product-id 32772 [--already-signed]

Usage 4 (only LZMA compression, single step):
  python3 manual_process.py input.signed.bin --only-lzma --output-dir ./output --lzma-dict-size 32768

Usage 5 (4MB Flash chips: tlsr9528a/tlsr9268j/tlsr9268m, simplified pipeline):
  python3 manual_process.py zephyr.bin <config_path> --chip-family tlsr9528a
  This executes only 2 steps: zephyr.bin -> zephyr.signed.bin -> matter.ota (skips LZMA)

Processing Steps:
1. Input firmware -> manual_zephyr.signed.bin (First MCUBoot signature)
2. manual_zephyr.signed.bin -> manual_zephyr.signed.lzma.bin (LZMA compression)
3. manual_zephyr.signed.lzma.bin -> manual_zephyr.signed.lzma.signed.bin (Second MCUBoot signature)
4. manual_zephyr.signed.lzma.signed.bin -> manual_matter.ota (Final Matter OTA package)

Manual step-by-step execution (for debugging):
If you want to manually execute each step of the pipeline, here are the exact commands:

1. First MCUBoot signature (zephyr.bin -> manual_zephyr.signed.bin):

   python3 /home/ubuntu/zephyrproject/bootloader/mcuboot/scripts/imgtool.py sign \
       --version 0.0.0+0 \
       --align 1 \
       --header-size 512 \
       --slot-size 1179648 \
       --key /home/ubuntu/zephyrproject/bootloader/mcuboot/ecdsa_private_key.pem \
       ./zephyr.bin \
       ./output/manual_zephyr.signed.bin

2. LZMA compression (manual_zephyr.signed.bin -> manual_zephyr.signed.lzma.bin):

2.1 Manual method using Python directly

    python3 << 'EOF'
    import lzma
    import os

    input_file = "./output/manual_zephyr.signed.bin"
    output_file = "./output/manual_zephyr.signed.lzma.bin"

    dict_size = 32768

    with open(input_file, "rb") as f:
        data = f.read()

    compressor = lzma.LZMACompressor(
        format=lzma.FORMAT_RAW,
        filters=[{
            "id": lzma.FILTER_LZMA1,
            "dict_size": dict_size,
            "lc": 1,
            "lp": 2,
            "pb": 0,
            "mode": lzma.MODE_NORMAL,
            "mf": lzma.MF_BT4,
            "depth": 0
        }]
    )

    compressed = compressor.compress(data) + compressor.flush()
    with open(output_file, "wb") as f:
        f.write(compressed)

    original = os.path.getsize(input_file)
    compressed_size = os.path.getsize(output_file)
    print(f"LZMA compression completed: Original {original} Bytes -> Compressed {compressed_size} Bytes, save {((1-compressed_size/original)*100):.1f}%")
    EOF

2.2 Simple method using manual_process.py

   python3 manual_process.py \
       ./output/manual_zephyr.signed.bin \
       --only-lzma \
       --output-dir ./output \
       --lzma-dict-size 32768

3. Second MCUBoot signature for compressed firmware (adds --pad-header 0x200 or 512):

   python3 /home/ubuntu/zephyrproject/bootloader/mcuboot/scripts/imgtool.py sign \
       --pad-header \
       --version 0.0.0+0 \
       --align 1 \
       --header-size 512 \
       --slot-size 1179648 \
       --key /home/ubuntu/zephyrproject/bootloader/mcuboot/ecdsa_private_key.pem \
       ./output/manual_zephyr.signed.lzma.bin \
       ./output/manual_zephyr.signed.lzma.signed.bin

4. Generate Matter OTA image:

   python3 /home/ubuntu/connectedhomeip/src/app/ota_image_tool.py create \
       --vendor-id 0xfff1 \
       --product-id 0x8004 \
       --version 2 \
       --version-str "2.0.0" \
       --digest-algorithm sha256 \
       ./output/manual_zephyr.signed.lzma.signed.bin \
       ./output/manual_matter.ota
"""

import argparse
import lzma
import os
import subprocess
import sys

# Configure tool paths here - edit these to match your environment
CONFIG = {
    # Path to MCUBoot imgtool.py
    'imgtool_path': '/home/ubuntu/zephyrproject/bootloader/mcuboot/scripts/imgtool.py',
    # Path to Matter ota_image_tool.py
    'ota_tool_path': '/home/ubuntu/connectedhomeip/src/app/ota_image_tool.py',
}


def parse_arguments():
    """Parse command line arguments"""
    parser = argparse.ArgumentParser(
        description='Manual Matter firmware processing tool for Telink platforms'
    )
    parser.add_argument(
        'firmware_path',
        help='Path to the input firmware file (can be raw zephyr.bin or already signed zephyr.signed.bin)'
    )
    parser.add_argument(
        'config_path',
        nargs='?',
        help='Path to the .config file containing build configuration (optional if manual parameters are provided)'
    )
    parser.add_argument(
        '--output-dir',
        default=None,
        help='Directory to write output files (defaults to same directory as input firmware)'
    )
    parser.add_argument(
        '--already-signed',
        action='store_true',
        help='Input firmware is already signed (zephyr.signed.bin), skip first MCUBoot signature step'
    )
    parser.add_argument(
        '--only-lzma',
        action='store_true',
        help='Only perform LZMA compression: input.signed.bin -> output/manual_input.signed.lzma.bin'
    )
    parser.add_argument(
        '--chip-family',
        choices=['tl3238c', 'tlsr9528a', 'tlsr9268j', 'tlsr9268m'],
        default='tl3238c',
        help='Chip family to process for: tl3238c (2MB Flash, full pipeline), tlsr9528a/tlsr9268m (4MB Flash, skip LZMA and second signature)'
    )
    # Tool path overrides - can override the CONFIG defaults
    parser.add_argument(
        '--imgtool-path',
        help='Override configured path to MCUBoot imgtool.py',
        default=None
    )
    parser.add_argument(
        '--ota-tool-path',
        help='Override configured path to Matter ota_image_tool.py',
        default=None
    )
    # Manual parameters - used if config_path is not provided

    def hex_or_int(value):
        try:
            return int(value, 0)
        except ValueError:
            raise argparse.ArgumentTypeError(f"invalid integer value: '{value}'")

    parser.add_argument(
        '--rom-start-offset',
        type=hex_or_int,
        help='ROM start offset (CONFIG_ROM_START_OFFSET), supports decimal or 0x-prefixed hex'
    )
    parser.add_argument(
        '--flash-load-size',
        type=hex_or_int,
        help='Flash load size (CONFIG_FLASH_LOAD_SIZE), supports decimal or 0x-prefixed hex'
    )
    parser.add_argument(
        '--lzma-dict-size',
        type=hex_or_int,
        help='LZMA dictionary size (CONFIG_COMPRESS_LZMA_DICTIONARY_SIZE), supports decimal or 0x-prefixed hex'
    )
    parser.add_argument(
        '--signing-key',
        help='MCUBoot signing key path (CONFIG_MCUBOOT_SIGNATURE_KEY_FILE)'
    )
    parser.add_argument(
        '--vendor-id',
        type=hex_or_int,
        help='Matter device vendor ID (CONFIG_CHIP_DEVICE_VENDOR_ID), supports decimal or 0x-prefixed hex'
    )
    parser.add_argument(
        '--product-id',
        type=hex_or_int,
        help='Matter device product ID (CONFIG_CHIP_DEVICE_PRODUCT_ID), supports decimal or 0x-prefixed hex'
    )
    parser.add_argument(
        '--software-version',
        type=int,
        default=1,
        help='Matter device software version (CONFIG_CHIP_DEVICE_SOFTWARE_VERSION, default: 1)'
    )
    parser.add_argument(
        '--software-version-str',
        default='1.0.0',
        help='Matter device software version string (CONFIG_CHIP_DEVICE_SOFTWARE_VERSION_STRING, default: "1.0.0")'
    )
    args = parser.parse_args()

    # Override CONFIG paths if command line arguments are provided
    if args.imgtool_path:
        CONFIG['imgtool_path'] = args.imgtool_path
    if args.ota_tool_path:
        CONFIG['ota_tool_path'] = args.ota_tool_path

    # Validate that if no config_path is provided, required manual parameters are present
    if args.config_path is None:
        if args.only_lzma:
            # Only LZMA mode only requires lzma-dict-size
            required_manual = [args.lzma_dict_size]
            if not all(required_manual):
                parser.error(
                    "When using --only-lzma and config_path is not provided, must specify --lzma-dict-size"
                )
        else:
            # Full pipeline requires all manual parameters
            required_manual = [
                args.rom_start_offset,
                args.flash_load_size,
                args.lzma_dict_size,
                args.signing_key,
                args.vendor_id,
                args.product_id
            ]
            if not all(required_manual):
                parser.error(
                    "When config_path is not provided, all manual parameters must be specified: "
                    "--rom-start-offset, --flash-load-size, --lzma-dict-size, --signing-key, "
                    "--vendor-id, --product-id"
                )

    return args


def parse_dotconfig(config_path):
    """Parse Kconfig .config file to extract all build configuration parameters"""
    config = {}
    if not os.path.exists(config_path):
        raise FileNotFoundError(f"Config file not found: {config_path}")

    with open(config_path, 'r') as f:
        for line in f:
            line = line.strip()
            if line.startswith('CONFIG_'):
                key, value = line.split('=', 1)
                # Strip quotes from string values
                value = value.strip('"')
                # Convert hexadecimal values
                if value.startswith('0x'):
                    config[key] = int(value, 16)
                # Convert integer values
                elif value.isdigit():
                    config[key] = int(value)
                # String values remain as strings
                else:
                    config[key] = value
    return config


def merge_binaries(input_file1, input_file2, output_file, offset):
    """Reuse merge_binaries function from the original process_binaries.py"""
    with open(output_file, 'r+b' if os.path.exists(output_file) else 'wb') as outfile:
        # Merge first input file at offset 0
        with open(input_file1, 'rb') as infile1:
            data1 = infile1.read()
            outfile.write(data1)
            print(f"Merged {len(data1)} bytes from {input_file1} into {output_file} at offset 0")

        # Fill gaps with 0xFF if there's space between the two binaries
        current_size = outfile.tell()
        if current_size < offset:
            gap_size = offset - current_size
            outfile.write(bytearray([0xFF] * gap_size))
            print(f"Filled gap of {gap_size} bytes with 0xFF in {output_file}")

        # Merge second input file at the specified offset
        with open(input_file2, 'rb') as infile2:
            outfile.seek(offset)
            data2 = infile2.read()
            outfile.write(data2)
            print(f"Merged {len(data2)} bytes from {input_file2} into {output_file} at offset {offset}")


def compress_lzma_firmware(input_file, output_file, dict_size):
    """Reuse compress_lzma_firmware function from the original process_binaries.py"""
    if not os.path.exists(input_file):
        raise FileNotFoundError(f"Input firmware not found: {input_file}")

    with open(input_file, 'rb') as f:
        firmware_data = f.read()

    # LZMA compression parameters (matched to device-side decoder)
    lc = 1  # Literal context bits
    lp = 2  # Literal position bits
    pb = 0  # Position bits

    # Create LZMA compressor with raw format for compatibility with lzma_raw_decoder()
    compressor = lzma.LZMACompressor(
        format=lzma.FORMAT_RAW,
        filters=[
            {
                "id": lzma.FILTER_LZMA1,
                "dict_size": dict_size,
                "lc": lc,
                "lp": lp,
                "pb": pb,
                "mode": lzma.MODE_NORMAL,
                "mf": lzma.MF_BT4,
                "depth": 0
            }
        ]
    )

    # Perform compression
    compressed_data = compressor.compress(firmware_data) + compressor.flush()

    # Write compressed binary to output file
    with open(output_file, 'wb') as f:
        f.write(compressed_data)

    print(f"LZMA compression completed: {input_file} -> {output_file}")
    print(
        f"  Size reduction: {len(firmware_data)} -> {len(compressed_data)} bytes ({(1-len(compressed_data)/len(firmware_data))*100:.1f}% smaller)")


def run_imgtool_sign(input_file, output_file, rom_start_offset, flash_load_size, key_path, zephyr_base, pad_header=False):
    """Reuse MCUBoot imgtool signing logic from CMakeLists.txt and process_binaries.py"""
    if not os.path.exists(input_file):
        raise FileNotFoundError(f"File to sign not found: {input_file}")

    # First try configured path, then fallback to auto-discovery
    imgtool_path = CONFIG['imgtool_path']
    if not os.path.exists(imgtool_path):
        # Fallback to original auto-discovery logic if configured path is invalid
        imgtool_candidates = [
            os.path.join(zephyr_base, '../bootloader/mcuboot/scripts/imgtool.py'),
            os.path.join(zephyr_base, 'bootloader/mcuboot/scripts/imgtool.py'),
            os.path.join(os.getcwd(), 'bootloader/mcuboot/scripts/imgtool.py'),
            os.path.join(os.path.dirname(__file__), 'bootloader/mcuboot/scripts/imgtool.py')
        ]
        for candidate in imgtool_candidates:
            if os.path.exists(candidate):
                imgtool_path = candidate
                break
    if not imgtool_path or not os.path.exists(imgtool_path):
        raise FileNotFoundError(
            f"MCUBoot imgtool not found. Check CONFIG['imgtool_path'] or use --imgtool-path. Last tried: {imgtool_path}")

    # Look for signing key in multiple possible locations
    key_candidates = [
        os.path.join(zephyr_base, '../', key_path),
        os.path.join(zephyr_base, key_path),
        os.path.join(os.getcwd(), key_path),
        os.path.join(os.getcwd(), os.path.basename(key_path)),
        os.path.join(os.path.dirname(__file__), key_path),
        os.path.join(os.path.dirname(__file__), os.path.basename(key_path))
    ]
    full_key_path = None
    for candidate in key_candidates:
        if os.path.exists(candidate):
            full_key_path = candidate
            break
    if not full_key_path:
        raise FileNotFoundError(f"Signing key not found in any of: {key_candidates}")
    if not os.path.exists(full_key_path):
        raise FileNotFoundError(f"Signing key not found: {full_key_path}")

    # Build imgtool command
    cmd = [
        sys.executable, imgtool_path, 'sign',
        '--version', '0.0.0+0',
        '--align', '1',
        '--header-size', str(rom_start_offset),
        '--slot-size', str(flash_load_size),
        '--key', full_key_path
    ]

    # Add --pad-header flag when signing compressed firmware
    if pad_header:
        cmd.append('--pad-header')

    cmd.extend([input_file, output_file])

    print(f"Executing MCUBoot sign command: {' '.join(cmd)}")
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"imgtool stdout: {result.stdout}")
        print(f"imgtool stderr: {result.stderr}")
        raise subprocess.CalledProcessError(result.returncode, cmd)
    print(f"MCUBoot signing completed: {input_file} -> {output_file}")


def run_ota_image_tool(input_file, output_file, vendor_id, product_id, version, version_str, chip_root):
    """Reuse Matter OTA image tool logic from ota-image.cmake"""
    if not os.path.exists(input_file):
        raise FileNotFoundError(f"Signed firmware not found: {input_file}")

    # First try configured path, then fallback to auto-discovery
    ota_tool_path = CONFIG['ota_tool_path']
    if not os.path.exists(ota_tool_path):
        # Fallback to original auto-discovery logic if configured path is invalid
        ota_tool_candidates = [
            os.path.join(chip_root, 'src/app/ota_image_tool.py'),
            os.path.join(os.getcwd(), 'src/app/ota_image_tool.py'),
            os.path.join(os.path.dirname(__file__), 'src/app/ota_image_tool.py'),
            os.path.join(os.getcwd(), 'ota_image_tool.py'),
            os.path.join(os.path.dirname(__file__), 'ota_image_tool.py')
        ]
        for candidate in ota_tool_candidates:
            if os.path.exists(candidate):
                ota_tool_path = candidate
                break
    if not ota_tool_path or not os.path.exists(ota_tool_path):
        raise FileNotFoundError(
            f"Matter OTA tool not found. Check CONFIG['ota_tool_path'] or use --ota-tool-path. Last tried: {ota_tool_path}")

    # Build Matter OTA image creation command
    cmd = [
        sys.executable, ota_tool_path, 'create',
        '--vendor-id', str(vendor_id),
        '--product-id', str(product_id),
        '--version', str(version),
        '--version-str', version_str,
        '--digest-algorithm', 'sha256',
        input_file,
        output_file
    ]

    print(f"Executing Matter OTA packaging command: {' '.join(cmd)}")
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"ota_image_tool stdout: {result.stdout}")
        print(f"ota_image_tool stderr: {result.stderr}")
        raise subprocess.CalledProcessError(result.returncode, cmd)
    print(f"Matter OTA packaging completed: {input_file} -> {output_file}")


def main():
    # Parse command line arguments
    args = parse_arguments()

    # Get project root paths - support both project environment and standalone environment
    # In standalone mode, all tools are relative to current working directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    if os.path.exists(os.path.join(script_dir, '../../..', 'src/app/ota_image_tool.py')):
        # Running from within the connectedhomeip project
        chip_root = os.path.abspath(os.path.join(script_dir, '../../..'))
        zephyr_base = os.environ.get('ZEPHYR_BASE', '/home/ubuntu/zephyr')
    else:
        # Running in standalone mode, all tools are in the current directory
        chip_root = os.getcwd()
        zephyr_base = os.getcwd()

    # Set output directory - default to same directory as input firmware
    output_dir = args.output_dir if args.output_dir else os.path.dirname(args.firmware_path)
    os.makedirs(output_dir, exist_ok=True)

    # Validate input firmware exists
    if not os.path.exists(args.firmware_path):
        raise FileNotFoundError(f"Firmware file not found: {args.firmware_path}")

    # Load configuration - either from .config file or manual parameters
    config = {}
    if args.config_path:
        if not os.path.exists(args.config_path):
            raise FileNotFoundError(f"Config file not found: {args.config_path}")
        print("Parsing build configuration from .config file...")
        config = parse_dotconfig(args.config_path)
    else:
        print("Using manually specified configuration parameters...")
        config['CONFIG_ROM_START_OFFSET'] = args.rom_start_offset
        config['CONFIG_FLASH_LOAD_SIZE'] = args.flash_load_size
        config['CONFIG_COMPRESS_LZMA_DICTIONARY_SIZE'] = args.lzma_dict_size
        config['CONFIG_MCUBOOT_SIGNATURE_KEY_FILE'] = args.signing_key
        config['CONFIG_CHIP_DEVICE_VENDOR_ID'] = args.vendor_id
        config['CONFIG_CHIP_DEVICE_PRODUCT_ID'] = args.product_id
        config['CONFIG_CHIP_DEVICE_SOFTWARE_VERSION'] = args.software_version
        config['CONFIG_CHIP_DEVICE_SOFTWARE_VERSION_STRING'] = args.software_version_str

    # Display loaded configuration
    # Only print full configuration if not in only-lzma mode
    if not args.only_lzma:
        print("\n=== Loaded Configuration Parameters ===")
        print(f"ROM_START_OFFSET: {config['CONFIG_ROM_START_OFFSET']} (0x{config['CONFIG_ROM_START_OFFSET']:x})")
        print(f"FLASH_LOAD_SIZE: {config['CONFIG_FLASH_LOAD_SIZE']} (0x{config['CONFIG_FLASH_LOAD_SIZE']:x})")
        print(f"LZMA_DICT_SIZE: {config['CONFIG_COMPRESS_LZMA_DICTIONARY_SIZE']}")
        print(f"Signature key: {config['CONFIG_MCUBOOT_SIGNATURE_KEY_FILE']}")
        print(f"Vendor ID: {config['CONFIG_CHIP_DEVICE_VENDOR_ID']} (0x{config['CONFIG_CHIP_DEVICE_VENDOR_ID']:x})")
        print(f"Product ID: {config['CONFIG_CHIP_DEVICE_PRODUCT_ID']} (0x{config['CONFIG_CHIP_DEVICE_PRODUCT_ID']:x})")
        print(
            f"Software version: {config['CONFIG_CHIP_DEVICE_SOFTWARE_VERSION']} - {config['CONFIG_CHIP_DEVICE_SOFTWARE_VERSION_STRING']}")
        print(f"Output directory: {output_dir}")
        print("========================================")
    else:
        # Minimal output for only-lzma mode
        print("\n=== LZMA Compression Configuration ===")
        print(f"LZMA dictionary size: {config['CONFIG_COMPRESS_LZMA_DICTIONARY_SIZE']}")
        print(f"Input file: {args.firmware_path}")
        print(f"Output directory: {output_dir}")
        print("======================================")

    # Define all output filenames with manual_ prefix
    manual_signed_bin = os.path.join(output_dir, 'manual_zephyr.signed.bin')
    manual_lzma_bin = os.path.join(output_dir, 'manual_zephyr.signed.lzma.bin')
    manual_final_signed = os.path.join(output_dir, 'manual_zephyr.signed.lzma.signed.bin')
    manual_matter_ota = os.path.join(output_dir, 'manual_matter.ota')

    # Handle only-lzma mode first
    if args.only_lzma:
        print("\n=== Only LZMA compression mode activated ===")
        # Derive output filename from input to retain original naming prefix
        input_filename = os.path.basename(args.firmware_path)
        if input_filename.endswith('.bin'):
            output_filename = input_filename[:-4] + '.lzma.bin'
        else:
            output_filename = input_filename + '.lzma.bin'
        manual_lzma_bin = os.path.join(output_dir, output_filename)
        print(f"Input file: {args.firmware_path}")
        print(f"Output file: {manual_lzma_bin}")
        compress_lzma_firmware(
            input_file=args.firmware_path,
            output_file=manual_lzma_bin,
            dict_size=config['CONFIG_COMPRESS_LZMA_DICTIONARY_SIZE']
        )
        print("\n=== LZMA compression completed successfully! Generated file: ===")
        size = os.path.getsize(manual_lzma_bin)
        print(f"{manual_lzma_bin}: {size/1024:.1f} KB")
        sys.exit(0)

    # Execute pipeline based on chip family
    print(f"\n=== Chip family selected: {args.chip_family} ===")
    if args.chip_family in ['tlsr9528a', 'tlsr9268j', 'tlsr9268m']:
        # 4MB Flash chips: simplified pipeline - only two steps
        print("\n=== Using simplified pipeline for 4MB Flash chips (skip LZMA and second signature) ===")
        if args.already_signed:
            print("\n=== Note: Input is already signed, skipping first MCUBoot signature step ===")
            import shutil
            shutil.copy2(args.firmware_path, manual_final_signed)
            print(f"Copied signed input to: {manual_final_signed}")
            input_for_ota = args.firmware_path
            generated_files = [manual_final_signed, manual_matter_ota]
        else:
            print("\n=== Step 1: Only MCUBoot signature (4MB chips don't need compression) ===")
            run_imgtool_sign(
                input_file=args.firmware_path,
                output_file=manual_final_signed,
                rom_start_offset=config['CONFIG_ROM_START_OFFSET'],
                flash_load_size=config['CONFIG_FLASH_LOAD_SIZE'],
                key_path=config['CONFIG_MCUBOOT_SIGNATURE_KEY_FILE'],
                zephyr_base=zephyr_base
            )
            input_for_ota = manual_final_signed
            generated_files = [manual_final_signed, manual_matter_ota]

        # Only generate OTA image as final step
        print("\n=== Step 2: Generate final Matter OTA image ===")
        run_ota_image_tool(
            input_file=input_for_ota,
            output_file=manual_matter_ota,
            vendor_id=config['CONFIG_CHIP_DEVICE_VENDOR_ID'],
            product_id=config['CONFIG_CHIP_DEVICE_PRODUCT_ID'],
            version=config['CONFIG_CHIP_DEVICE_SOFTWARE_VERSION'],
            version_str=config['CONFIG_CHIP_DEVICE_SOFTWARE_VERSION_STRING'],
            chip_root=chip_root
        )
    else:
        # tl3238c: original full pipeline with LZMA compression
        print("\n=== Using full pipeline for 2MB Flash chips (includes LZMA and dual signatures) ===")
        # Determine starting point based on --already-signed flag
        if args.already_signed:
            # Input is already signed: zephyr.signed.bin -> skip first step
            print("\n=== Note: Input is already signed, skipping first MCUBoot signature step ===")
            input_for_compression = args.firmware_path
            # Copy the already signed file to our manual_ prefix file for consistency
            import shutil
            shutil.copy2(args.firmware_path, manual_signed_bin)
            print(f"Copied signed input to: {manual_signed_bin}")
            step_start = 2
        else:
            # Step 1: Input firmware -> manual_zephyr.signed.bin (First MCUBoot signature)
            print("\n=== Step 1: First MCUBoot signature ===")
            run_imgtool_sign(
                input_file=args.firmware_path,
                output_file=manual_signed_bin,
                rom_start_offset=config['CONFIG_ROM_START_OFFSET'],
                flash_load_size=config['CONFIG_FLASH_LOAD_SIZE'],
                key_path=config['CONFIG_MCUBOOT_SIGNATURE_KEY_FILE'],
                zephyr_base=zephyr_base
            )
            input_for_compression = manual_signed_bin
            step_start = 2

        # Step 2: manual_zephyr.signed.bin -> manual_zephyr.signed.lzma.bin (LZMA compression)
        print(f"\n=== Step {step_start}: LZMA compression ===")
        compress_lzma_firmware(
            input_file=input_for_compression,
            output_file=manual_lzma_bin,
            dict_size=config['CONFIG_COMPRESS_LZMA_DICTIONARY_SIZE']
        )

        # Step 3: manual_zephyr.signed.lzma.bin -> manual_zephyr.signed.lzma.signed.bin (Second MCUBoot signature)
        print(f"\n=== Step {step_start + 1}: Second MCUBoot signature (after compression) ===")
        run_imgtool_sign(
            input_file=manual_lzma_bin,
            output_file=manual_final_signed,
            rom_start_offset=config['CONFIG_ROM_START_OFFSET'],
            flash_load_size=config['CONFIG_FLASH_LOAD_SIZE'],
            key_path=config['CONFIG_MCUBOOT_SIGNATURE_KEY_FILE'],
            zephyr_base=zephyr_base,
            pad_header=True  # --pad-header is REQUIRED for compressed firmware
        )

        # Step 4: manual_zephyr.signed.lzma.signed.bin -> manual_matter.ota (Matter OTA packaging)
        print(f"\n=== Step {step_start + 2}: Generate final Matter OTA image ===")
        run_ota_image_tool(
            input_file=manual_final_signed,
            output_file=manual_matter_ota,
            vendor_id=config['CONFIG_CHIP_DEVICE_VENDOR_ID'],
            product_id=config['CONFIG_CHIP_DEVICE_PRODUCT_ID'],
            version=config['CONFIG_CHIP_DEVICE_SOFTWARE_VERSION'],
            version_str=config['CONFIG_CHIP_DEVICE_SOFTWARE_VERSION_STRING'],
            chip_root=chip_root
        )
        generated_files = [manual_signed_bin, manual_lzma_bin, manual_final_signed, manual_matter_ota]

    print("\n=== All steps completed successfully! Generated files: ===")
    for f in generated_files:
        if os.path.exists(f):
            size = os.path.getsize(f)
            print(f"{f}: {size/1024:.1f} KB")


if __name__ == '__main__':
    print("=== Matter Telink Manual Firmware Processing Tool ===")
    print("\nUsage 1 (process raw firmware, full pipeline):")
    print("  python3 manual_process.py zephyr.bin <config_path> [--output-dir OUTPUT_DIR]")
    print("Example 1:")
    print("  python3 manual_process.py \\")
    print("    /home/ubuntu/connectedhomeip/examples/light-switch-app/telink/build_tl3238x_retention_dual_sw_v1/zephyr/zephyr.bin \\")
    print("    /home/ubuntu/connectedhomeip/examples/light-switch-app/telink/build_tl3238x_retention_dual_sw_v1/zephyr/.config\n")
    print("Usage 2 (process already signed firmware, start from compression):")
    print("  python3 manual_process.py zephyr.signed.bin <config_path> --already-signed")
    print("Example 2:")
    print("  python3 manual_process.py \\")
    print("    /home/ubuntu/connectedhomeip/examples/light-switch-app/telink/build_tl3238x_retention_dual_sw_v1/zephyr/zephyr.signed.bin \\")
    print("    /home/ubuntu/connectedhomeip/examples/light-switch-app/telink/build_tl3238x_retention_dual_sw_v1/zephyr/.config --already-signed\n")
    print("Usage 3 (with manual parameters):")
    print("  python3 manual_process.py <firmware_path> [--output-dir OUTPUT_DIR] \\")
    print("    --rom-start-offset 512 --flash-load-size 1179648 \\")
    print("    --lzma-dict-size 32768 --signing-key bootloader/mcuboot/root-rsa-2048.pem \\")
    print("    --vendor-id 65521 --product-id 32772 [--already-signed]\n")

    try:
        main()
    except Exception as e:
        import traceback
        traceback.print_exc()
        print(f"\nERROR: {str(e)}", file=sys.stderr)
        sys.exit(1)
