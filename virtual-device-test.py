from cmath import pi, sin
import io
from operator import ne
import os
import re
import sys
import time
import math
import asyncio
from os import environ
from posixpath import dirname
from websockets import client as ws

import AvhClientAsync
from AvhClientAsync.models import InstanceState
from AvhClientAsync.rest import ApiException
from pprint import pprint

import ssl
ctx = ssl.create_default_context()
ctx.check_hostname = False
ctx.verify_mode = ssl.CERT_NONE

apiEndpoint = "https://app.avh.arm.com/api"
apiToken = "51f6cc92c0c4c08072a0.93da8450ac6e901a6fa2c95b4f044926623bd92eda46c2cdb54caf97f79851c147a3a900e22e7be50363d2c47d857351fd0faec319b58411895a1fe1819392ce"
fw = "chip-tool"
flavor = "rpi4b"
vmName = 'chip-tool'

async def waitForState(instance, state):
  global api_instance

  instanceState = await api_instance.v1_get_instance_state(instance.id)
  while (instanceState != state):
    if (instanceState == 'error'):
      raise Exception('VM entered error state')
    await asyncio.sleep(1)
    instanceState = await api_instance.v1_get_instance_state(instance.id)

ledStates = [ 'off', 'on' ]
async def printLeds(instance):
  state = await api_instance.v1_get_instance_gpios(instance.id)
  ledBank = state['led'].banks[0]
  print('LED6: %s LED7: %s' % (ledStates[ledBank[0]], ledStates[ledBank[1]]) )

async def pressButton(instance):
  await api_instance.v1_set_instance_gpios(instance.id, {
    "button": {
      "bitCount": 1,
      "banks": [
        [1]
      ]
    }
  })
  await api_instance.v1_set_instance_gpios(instance.id, {
    "button": {
      "bitCount": 1,
      "banks": [
        [0]
      ]
    }
  })

exitStatus = 0

async def main():
  global exitStatus
  global api_instance

  configuration = AvhClientAsync.Configuration(
      host = apiEndpoint
  )
  # Enter a context with an instance of the API client
  async with AvhClientAsync.ApiClient(configuration=configuration) as api_client:
    # Create an instance of the API class
    api_instance = AvhClientAsync.ArmApi(api_client)

    # Log In
    token_response = await api_instance.v1_auth_login({
      "apiToken": apiToken,
    })


    print('Logged in')
    configuration.access_token = token_response.token

    print('Finding a project...')
    api_response = await api_instance.v1_get_projects()
    pprint(api_response)
    projectId = api_response[0].id

    print('Getting our model...')
    api_response = await api_instance.v1_get_models()
    chosenModel = None
    for model in api_response:
      if model.flavor.startswith('rpi4b'):
        chosenModel = model
        break

    pprint(chosenModel)

    print('Finding software for our model...')
    api_response = await api_instance.v1_get_model_software(model.model)
    chosenSoftware = None
    for software in api_response:
       pprint(software)
    #  if software.filename.startswith('STM32U5-WiFiBasics'):
    #    # This software package is compatible with our bsp.elf image
    #    chosenSoftware = software
    #    break

    print('Creating a new instance...')
    api_response = await api_instance.v1_create_instance({
      "name": vmName,
      "project": projectId,
      "flavor": chosenModel.flavor,
      #"os": chosenSoftware.version,
      #"osbuild": chosenSoftware.buildid
    })
    instance = api_response

    error = None
    try:
      print('Waiting for VM to create...')
      await waitForState(instance, 'on')

      print('Setting the VM to use the bsp test software')
      #api_response = await api_instance.v1_create_image('fwbinary', 'plain', 
      #  name=fw,
      #  instance=instance.id,
      #  file=os.path.join(sys.path[0], '../assets/' + fw)
      #)
      #pprint(api_response)

      print('Resetting VM to use the new software')
      api_response = await api_instance.v1_reboot_instance(instance.id)
      print('Waiting for VM to finish resetting...')
      await waitForState(instance, 'on')
      print('done')
      print('Logging GPIO initial state:')
      gpios = await api_instance.v1_get_instance_gpios(instance.id)
      pprint(gpios)

      #Figure out how to copy chip-tool fw here
      print('Figure out how to copy fw here...')

    except Exception as e:      
      print('Encountered error; cleaning up...')
      error = e

    if error != None:
      raise error

asyncio.run(asyncio.wait_for(main(), 120))
exit(0)
