import os
from pathlib import Path
import serial

from pw_hdlc_lite.rpc import HdlcRpcClient

# Point the script to the .proto file with our RPC services.
PROTO = Path(os.environ['PW_ROOT'], 'pw_rpc/pw_rpc_protos/echo.proto')

MOBLY_CONTROLLER_CONFIG_NAME = 'ChipDevice'

class Error(Exception):
  """This is the Exception class defined for all errors."""

class ChipDevice:
  def __init__(self, device_tty, baud):
    self.pw_rpc_client = HdlcRpcClient(serial.Serial(device_tty, baud), [PROTO])

  def rpcs(self):
    return self.pw_rpc_client.rpcs().pw.rpc

def create(configs):
  """Initializes the CHIP devices based on the testbed configuration.

  Args:
    configs: a list of testbed configs.

  Returns:
    a list of device objects
  """
  objs = []
  for config in configs:
    _validate_config(config)
    device = ChipDevice(config.get('device_tty'),
                        config.get('baud'))
    objs.append(device)
  return objs


def destroy(unused_objs):
  """Destroys the wearable objects.

  Args:
    unused_objs: a list of device objects.
  """
  pass


def _validate_config(config):
  """Verifies that a config dict for a CHIP device is valid.

  Args:
    config: A dict that is the configuration for a CHIP device.

  Raises:
    chip_device.Error: Config file is not valid.
  """
  required_keys = ['device_tty', 'baud']  # A placeholder.
  for key in required_keys:
    if key not in config:
      raise Error('Required key %s missing from config %s' % (key, config))
