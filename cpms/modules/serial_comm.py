import pylink

# https://pylink.readthedocs.io/en/latest/installation.html
# pip install pylink-square

class SerialComm:
    def __init__(self, part_number, serial_num):
        self.link = pylink.JLink()
        self.part_number = part_number
        self.serial = serial_num

    def open(self):
        # print("‣ Serial open {}; {}".format(self.board, self.serial))
        self.link.open(serial_no=self.serial)
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
