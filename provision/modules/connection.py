from .util import *

import pylink
# https://pylink.readthedocs.io/en/latest/installation.html
# pip install pylink-square

class Connection:
    DEFAULT_PORT = 19020

    def __init__(self, args, part_number):

        if args.conn.lib_path is None:
            self.link = pylink.JLink()
        else:
            self.link=pylink.JLink(lib=pylink.library.Library(dllpath=args.conn.lib_path))
        self.part_number = part_number

    def open(self, conn):
        if conn.serial_num:
            print("\n▪︎ Open SERIAL connection {} to {}".format(conn.serial_num, self.part_number))
            self.link.open(serial_no=conn.serial_num)
        elif conn.ip_addr:
            port = conn.port or Connection.DEFAULT_PORT
            ip_addr = "{}:{}".format(conn.ip_addr, port)
            print("\n▪︎ Open TCP connection {} to {}".format(ip_addr, self.part_number))
            self.link.open(ip_addr=ip_addr)
        else:
            print("\n▪︎ Open DEFAULT connection to {}".format(self.part_number))
            self.link.open()

        self.link.set_tif(interface=pylink.JLinkInterfaces.SWD)
        self.link.connect(chip_name=self.part_number, speed="auto", verbose=True)

    def start(self):
        self.link.rtt_start()

    def stop(self):
        self.link.rtt_stop()

    def close(self):
        self.link.close()

    def send(self, data):
        nb_sent = 0
        while nb_sent == 0:
            nb_sent = self.link.rtt_write(0, data)
        return nb_sent

    def receive(self):
        data = bytes()
        while len(data) == 0:
            data = self.link.rtt_read(0, 1024)
        return data

    def reset(self, do_halt = False):
        self.link.reset(halt=do_halt)

    def flash(self, ram_addr, stack_addr, img):
        print("∙ addr: {}\n∙ stack: {}\n∙ image size: {}".format(hex(ram_addr), hex(stack_addr), len(img)))
        self.link.memory_write8(addr=ram_addr, data=list(img))
        self.link.register_write(reg_index=13, value=stack_addr) # SP
        self.link.register_write(reg_index=15, value=stack_addr) # PC
        self.link.restart(num_instructions=0, skip_breakpoints=False)
