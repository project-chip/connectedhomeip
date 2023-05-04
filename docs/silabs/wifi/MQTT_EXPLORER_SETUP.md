# Set Up MQTT Explorer
    Download and install the MQTT Explorer from https://mqtt-explorer.com/
    Setting up MQTT Explorer for testing.
 
 ![MQTT Explorer settings](./images/mqtt_explorer_1.png)
## Connecting to MQTT Server
### Connecting to AWS 
   - Host : Your Host name (examples: a2m21kovu9tcsh-ats.iot.ap-southeast-1.amazonaws.com)
   - Port : 8883
   - Make sure you enable `Validate Certificate and Encryption`
   - Click on the `Advanced Settings` 
  
 ![Silicon Labs - DIC design](./images/mqtt_explorer_2.png)

  - Add topics as shown (#, light/*, test_publish )
  - MQTT Client ID depends on the certificate set that you will use.
  - Add the Certificate, following step 7 in [AWS installation](./AWS_CONFIGURATION_REGISTERATION.md)

 ![Silicon Labs - DIC design](./images/mqtt_explorer_5.png)

  - Once the above steps are done, try connecting to AWS.
 
 ![Silicon Labs - DIC design](./images/mqtt_explorer_3.png)
  
### Connecting to Mosquitto Connection
   - Host : Your Mosquitto ip address
   - Port : 8883
   - Make sure you enable Validate Certificate and Encryption
   - Click on the `Advanced Settings` 
 
 ![Silicon Labs - DIC design](./images/mqtt_explorer_2.png)
 
  - Add topics as shown (#, light/*, test_publish )
  - MQTT Client ID depends on the certificate set that you will use.
  - Add the Certificate, following step 5 in [openssl certificate create](./OPENSSL_CERTIFICATE_CREATION.md)

 ![Silicon Labs - DIC design](./images/mqtt_explorer_5.png)