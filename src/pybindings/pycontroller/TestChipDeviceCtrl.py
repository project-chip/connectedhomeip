from ChipDeviceCtrl import ChipDeviceController

discriminator = 0xF00
setup_code = 20202021
node_id = 1
local_node_id = 112233 
ipaddr="127.0.0.1"
fabricID = 5544332211
chip_controller = ChipDeviceController()
r = chip_controller.ResolveNode(fabricID, 1)

print(r)

print(chip_controller.GetAddressAndPort(node_id))