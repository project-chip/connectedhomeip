ls -l /dev/ttyUSB*
sudo chmod 777 /dev/ttyUSB0
idf.py build
idf.py -p /dev/ttyUSB0 erase-flash 
idf.py -p /dev/ttyUSB0 flash monitor
