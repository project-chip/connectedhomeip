from ChipDeviceCtrl import ChipDeviceController

discriminator = 0xF00
setup_code = 20202021
node_id = 1
local_node_id = 112233 
ipaddr="127.0.0.1"

chip_controller = ChipDeviceController()
chip_controller.ConnectBLE(discriminator, setup_code, node_id)
chip_controller.CloseBLEConnection()
chip_controller.ConnectIP(ipaddr, setup_code, node_id)