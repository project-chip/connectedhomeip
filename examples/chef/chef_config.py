import os
#
# USER DEFINED INFORMATION
#
configured = False
#configured = True

# NRFConnect

nrfconnectFolder = "/opt/nordic/ncs/"
# nrfConnectSerialDevice = "/dev/tty.usbmodem0*" # Typical MacOS path
nrfConnectSerialDevice = "/dev/ttyACM0"  # Typical Linux path

# ESP32

esp32Folder = os.path.expandvars("$HOME/tools/esp-idf")
esp32SerialDevice = "/dev/ttyUSB0"  # Typical Linux path
# esp32SerialDevice = "/dev/tty.usbserial-*" # Typical MacOS path
