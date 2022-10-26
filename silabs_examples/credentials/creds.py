import sys, getopt
import subprocess
import time
import os

_temp_dir = './temp/'
_csr_pem =  _temp_dir + 'csr.pem'
_dac_der =  _temp_dir + 'dac_cert.der'
_dac_pem =  _temp_dir + 'dac_cert.pem'
_pai_der =  _temp_dir + 'pai_cert.der'
_header_filename =  _temp_dir + 'efr32_creds.h'
_header_template = 'efr32_creds.tmpl'


class PageInfo:
    def __init__(self, address, size):
        self.address = address
        self.size = size


def printUsage():
    print('Usage:')
    print('\tpython3 ./creds.py -p <serial_port> -S <serial_number> -B <board_num> -N <common_name> -V <vendor_id> -P <product_id> -C <pai_cert_pem> -K <pai_key_pem> -D <cert_declaration>')


def roundNearest(n, multiple):
    if n % multiple:
        n = n + (multiple - n % multiple)
    return n


def pageInfo(board):
    # MG12    
    if (('brd4161a' == board) or ('brd4162a' == board) or ('brd4163a' == board) or
        ('brd4164a' == board) or ('brd4166a' == board) or ('brd4170a' == board) or
        ('brd4304a' == board)):
        return PageInfo(0x000FF800, 0x800)
    # MG24
    if (('brd2601b' == board) or ('brd2703a' == board) or ('brd4186a' == board) or
        ('brd4186c' == board) or ('brd4187a' == board) or ('brd4187c' == board) or
        ('brd4304a' == board)):
        return PageInfo(0x0817E000, 0x2000)
    # MGM24
    if ('brd4316a' == board) or ('brd4317a' == board) or ('brd4319a' == board):
        return PageInfo(0x0817E000, 0x2000)
    return None


def execute(desc, args):
    print("\n{}\n  {}\n".format(desc, ' '.join(args)))
    complete = subprocess.run(args)
    if 0 != complete.returncode:
        print("ERROR: {}".format(complete.returncode))
        exit(complete.returncode)


def prepareApps(board, serial_num):

    device_dir = "device/{}".format(board)
    if not os.path.exists(device_dir):
        print("ERROR: Board not supported: '{}'".format(board))

    # Build device app
    execute("Building device app", ["make", "-C", device_dir, "-f", "device-creds.Makefile" ])

    # Flash device app
    device_bin = device_dir + '/build/debug/device-creds.s37'
    execute('Flashing device app', [ "commander", "flash", device_bin, "--serialno", serial_num ])
    time.sleep(2)

    # Build host app
    execute('Building host app', ["make", "-C", "host/app/", "-f", "host-creds.Makefile" ])


def generateFiles(port, serial_num, common_name, vendor_id, product_id, pai_pem, pai_key):

    # Create temporary dir
    execute('Creating temp dir', ["mkdir", "-p", _temp_dir ])

    # Generate CSR
    execute('Requesting CSR', ["./host/app/build/debug/host-creds", "-p", port, "-f", _csr_pem, "-N", common_name, "-V", vendor_id, "-P", product_id ])
    subprocess.run([ "cat", _csr_pem])

    # Generate DAC
    execute('Generating DAC (from device CSR)', 
        ["openssl", "x509", "-sha256", "-req", "-days", "18250",
        "-extensions", "v3_ica", "-extfile", "./csa_openssl.cnf",
        "-set_serial", serial_num,
        "-CA", pai_pem, "-CAkey", pai_key,
        "-in", _csr_pem, "-outform", "der", "-out", _dac_der])

    # Format DAC to PEM (for debugging)
    subprocess.run(["openssl", "x509", "-inform", "der", "-in", _dac_der, "-out", _dac_pem])

    # Format PAI to DER
    execute('Parsing PAI', ["openssl", "x509", "-outform", "der", "-in", pai_pem, "-out", _pai_der])


def writeCredentials(serial_num, page, cd_file):

    # Calculate offsets
    
    pai_stats = os.stat(_pai_der)
    dac_stats = os.stat(_dac_der)
    cd_stats = os.stat(cd_file)

    pai_offset = 0
    dac_offset = roundNearest(pai_offset + pai_stats.st_size, 64)
    cd_offset = roundNearest(dac_offset + dac_stats.st_size, 64)
    end_offset = roundNearest(cd_offset + cd_stats.st_size, 1024)

    # Generate header

    with open(_header_template, 'r') as tf:
        header = tf.read()
        header = header.replace('{{pai_offset}}', hex(pai_offset))
        header = header.replace('{{pai_size}}', str(pai_stats.st_size))
        header = header.replace('{{dac_offset}}', hex(dac_offset))
        header = header.replace('{{dac_size}}', str(dac_stats.st_size))
        header = header.replace('{{cd_offset}}', hex(cd_offset))
        header = header.replace('{{cd_size}}', str(cd_stats.st_size))
        # Write header
        with open(_header_filename, 'w') as hf:
            hf.write(header)

    # Flash

    cd_address = page.address + cd_offset
    pai_address = page.address + pai_offset
    dac_address = page.address + dac_offset
    print("PAI:\t{} + {}\t\t= {} ({})".format(hex(page.address), hex(pai_offset), hex(pai_address), pai_stats.st_size))
    print("DAC:\t{} + {}\t= {} ({})".format(hex(page.address), hex(dac_offset), hex(dac_address), dac_stats.st_size))
    print("DC:\t{} + {}\t= {} ({})".format(hex(page.address), hex(cd_offset), hex(cd_address), cd_stats.st_size))

    
    execute('Erasing Page', ["commander", "device", "pageerase", "--range", "{}:+{}".format(hex(page.address), page.size), "--serialno", serial_num])
    execute('Flashing PAI', ["commander", "flash", _pai_der, "--binary", "--address", hex(page.address + pai_offset), "--serialno", serial_num])
    execute('Flashing DAC', ["commander", "flash", _dac_der, "--binary", "--address", hex(page.address + dac_offset), "--serialno", serial_num])
    execute('Flashing CD', ["commander", "flash", cd_file, "--binary", "--address", hex(page.address + cd_offset), "--serialno", serial_num])

    # Print
    subprocess.run(["commander", "readmem", "--range", "{}:+{}".format(hex(page.address), end_offset), "--serialno", serial_num])


def main(argv):
    port = None
    serial_num = None
    board = None
    cert_file = None
    key_file = None
    common_name = None
    vendor_id = None
    product_id = None
    cd_file = None
    
    # Parse arguments

    try:
        opts, args = getopt.getopt(argv,"p:S:B:N:V:P:C:K:D:", ["port=", "serial_num=", "board=", "name=", "vendor=", "product=", "pai_cert=", "pai_key=", "cd="])
    except getopt.GetoptError:
        printUsage();
        sys.exit(2)

    for opt, arg in opts:
        if opt == '-h':
            printUsage();
            sys.exit()

        elif opt in ("-p", "--port"):
            port = arg

        elif opt in ("-S", "--serial"):
            serial_num = arg

        elif opt in ("-B", "--board"):
            board = arg

        elif opt in ("-N", "--name"):
            common_name = arg

        elif opt in ("-V", "--vendor"):
            vendor_id = arg

        elif opt in ("-P", "--product"):
            product_id = arg

        elif opt in ("-C", "--pai_cert"):
            cert_file = arg

        elif opt in ("-K", "--pai_key"):
            key_file = arg

        elif opt in ("-D", "--cd"):
            cd_file = arg

    # Validate arguments

    if (board is None) or (port is None) or (serial_num is None) or (common_name is None) or (vendor_id is None) or (product_id is None) or (cert_file is None) or (key_file is None) or (cd_file is None):
        printUsage();
        sys.exit(2)

    # Check environment

    if os.getenv('ARM_GCC_DIR') is None:
        print("ERROR: ARM_GCC_DIR not set")
        exit(1)

    if os.getenv('BASE_SDK_PATH') is None:
        print("ERROR: BASE_SDK_PATH not set")
        exit(1)

    # Get flash base address

    page_info = pageInfo(board)
    if page_info is None:
        print("ERROR: Board not supported: '{}'".format(board))
        exit(1)

    # Check input files

    if not os.path.exists(cd_file):
        print("File not found: '{}'".format(cd_file))

    if not os.path.exists(cert_file):   
        print("File not found: '{}'".format(cert_file))

    if not os.path.exists(key_file):
        print("File not found: '{}'".format(key_file))

    # Prepare apps
    prepareApps(board, serial_num)

    # Generate files
    generateFiles(port, serial_num, common_name, vendor_id, product_id, cert_file, key_file)

    # Write files
    writeCredentials(serial_num, page_info, cd_file)


main(sys.argv[1:])
