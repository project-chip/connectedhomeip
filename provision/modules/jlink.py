from modules.parameters import Types, Formats, ID
import modules.channel as _base
import modules.util as _util
import pylink
import time
# https://pylink.readthedocs.io/en/latest/installation.html
# pip install pylink-square


class JLinkChannel(_base.Channel):
    DEFAULT_PORT = 19020

    def __init__(self, paths, args, conn) -> None:
        super().__init__(_base.Channel.RTT)
        self.ip_addr = conn.ip_addr
        self.port = conn.port or JLinkChannel.DEFAULT_PORT
        self.serial_num = conn.serial_num
        self.part_number = args.str(ID.kDevice)
        lib_path = args.str(ID.kPylinkLib)
        if lib_path is None:
            self.link = pylink.JLink()
        else:
            self.link=pylink.JLink(lib=pylink.library.Library(dllpath=lib_path))
        self.link.disable_dialog_boxes()
        self.support_dir = paths.support()


    def open(self):
        if self.serial_num:
            print("* Open SERIAL connection {} to {}\n".format(self.serial_num, self.part_number))
            self.link.open(serial_no=self.serial_num)
        elif self.ip_addr:
            ip_str = "{}:{}".format(self.ip_addr, self.port)
            print("* Open TCP connection {} to {}\n".format(ip_str, self.part_number))
            self.link.open(ip_addr=ip_str)
        else:
            print("* Open DEFAULT connection to {}\n".format(self.part_number))
            self.link.open()

        self.link.exec_command("JLinkDevicesXMLPath {}/".format(self.support_dir))
        self.link.set_tif(interface=pylink.JLinkInterfaces.SWD)
        self.link.connect(chip_name=self.part_number, speed="auto", verbose=True)
        # print("Start...")
        self.link.rtt_start()


    def close(self):
        print("* Connection closed.\n")
        self.link.rtt_stop()
        self.link.close()


    def write(self, data):
        # print("Send({})...".format(len(data)))
        sent = 0
        while sent < len(data):
            sent = self.link.rtt_write(0, data[sent:])
        # print("SENT({})".format(sent))
        return sent

    def read(self):
        # print("Read...")
        chunk = []
        data = []
        while (len(chunk) > 0) or (0 == len(data)):
            chunk = self.link.rtt_read(0, 1024)
            data.extend(chunk)
        # print("RECEIVED({}): {}".format(len(data), data))
        return bytearray(data)


    def reset(self, do_halt = False):
        print("{}> reset {}\n".format(_util.MARGIN, do_halt and 'HALT' or ''))
        self.link.reset(halt=do_halt)


    def flash(self, firmware_path, address):
        print("{}> flash(0x{:08x}) {}\n".format(_util.MARGIN, address, firmware_path))
        self.open()
        self.link.flash_file(firmware_path, address)
        self.reset()
        self.close()
