import sys
from modules.util import *

GSDK_DIR = './third_party/silabs/gecko_sdk'

X509_CREATE_PATCH ='''$(OUTPUT_DIR)/project/_/mbedtls/library/x509_create.o: ../mbedtls/library/x509_create.c
	@$(POSIX_TOOL_PATH)echo 'Building ../mbedtls/library/x509_create.c'
	@$(POSIX_TOOL_PATH)mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ ../mbedtls/library/x509_create.c
CDEPS += $(OUTPUT_DIR)/project/_/mbedtls/library/x509_create.d
OBJS += $(OUTPUT_DIR)/project/_/mbedtls/library/x509_create.o
'''

# Patch to reserve the last page of main flash in EFR32
def patchEfr32Linker(part_dir):
    liker_file = "{}/autogen/linkerfile.ld".format(part_dir)
    temp_file = liker_file + '.temp'
    with open(temp_file, 'w') as t:
        with open(liker_file, 'r') as f:
            for line in f.readlines():
                if line.strip().startswith('FLASH'):
                    line = line.replace('0x100000', '0x0ff800')
                elif line.strip().startswith('__main_flash_end__'):
                    line = line.replace('0x100000', 'LENGTH(FLASH)')
                t.writelines([ line ])
    execute([ 'rm', liker_file ])
    execute([ 'mv', temp_file, liker_file ])


# Patch to add VID, PID support in the mbedTLS certificates
def patchProject(part_dir):
    project_file = "{}/generator.project.mak".format(part_dir)
    global X509_CREATE_PATCH
    temp_file = project_file + '.temp'
    with open(temp_file, 'w') as t:
        with open(project_file, 'r') as f:
            for line in f.readlines():
                if line.strip().startswith('BASE_SDK_PATH ='):
                    line = 'BASE_SDK_PATH = ../../../third_party/silabs/gecko_sdk\n'
                t.writelines([ line ])
    execute([ 'rm', project_file ])
    execute([ 'mv', temp_file, project_file ])

def generate(part, part_dir):
    print("▪︎ Generate")
    global GSDK_DIR
    execute([ 'slc', 'generate', '-p', './cpms/generator/generator.slcp', '-d', part_dir, '--with', part.upper(), '--sdk', GSDK_DIR ])

def patch(part, part_dir):
    print("▪︎ Patch")
    if part.startswith('efr32mg12'):
        patchEfr32Linker(part_dir)
    patchProject(part_dir)

def make(part, part_dir):
    print("▪︎ Make")
    image_src = "{}/build/debug/generator.s37".format(part_dir)
    image_dest = "./cpms/images/{}.s37".format(part)
    print("  ∙ from:  {}".format(image_src))
    print("  ∙ to:  {}".format(image_dest))
    execute([ 'make', '-C', part_dir, '-f', 'generator.Makefile' ])
    execute([ 'cp', image_src, image_dest ])

arg_count = len(sys.argv)
if arg_count < 3:
    fail("Missing parameters")

action = sys.argv[1].lower()
parts = sys.argv[2].upper()

for part in parts.split(','):
    part_name = part.lower()
    part_dir = "./cpms/support/{}".format(part_name)
    print("\n◆ {}".format(part))
    print("  ∙ dir:  {}".format(part_dir))
    if 'gen' == action:
        generate(part_name, part_dir)
    elif 'patch' == action:
        patch(part_name, part_dir)
    elif 'make' == action:
        make(part_name, part_dir)
    elif 'build' == action:
        generate(part_name, part_dir)
        patch(part_name, part_dir)
        make(part_name, part_dir)
    else:
        fail("Invalid action '{}'".format(action))
