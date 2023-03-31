

# SoC configuration
socs = {
  'mg12': { 'ram_addr': 0x20000000, 'stacksize': 0x1000 },
  'mg24': { 'ram_addr': 0x20000000, 'stacksize': 0x1000 },
}

class SilabsChip:

    def __init__(self, part_number):
        self.type = part_number
        self.part = part_number.lower()

        # Only MG12 and MG24 are supported in matter currently
        if "EFR32MG12" in part_number:
            self.family = 'mg12'
        elif "EFR32MG24" in part_number:
            self.family = 'mg24'
        else:
            raise Exception('Invalid MCU')

        d = socs[self.family]
        self.ram_addr = d['ram_addr']
        self.stacksize = d['stacksize']

    def __str__(self):
        return "  ∙ family: '{}'\n  ∙ ram_addr: {}\n  ∙ stacksize: {}".format(self.family, self.ram_addr, self.stacksize)
