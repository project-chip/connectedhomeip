import subprocess
# from chip.testing.matter_testing import MatterBaseTest


class JointFabricController():
    def __init__(self, app):
        # super()._init__()
        self.jfc_app = subprocess.Popen(app, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    def commission_device(self, commissioning_method="onnetwork", node_id=1, setup_pin_code=20202021):
        if commissioning_method == "onnetwork":
            self.jfc_app.stdin.write(f"pairing {node_id} onnetwork {setup_pin_code}\n".encode())


if __name__ == "__main__":
    jfc = JointFabricController(app="/home/nxa14934/e2e-jf/jfc-app")
    pass