# Mosquitto
  - Mosquitto is a lightweight open source (EPL/EDL licensed) message broker that implements the MQTT protocol. Refer [Mosquitto](https://Mosquitto.org/) for more details
## Set Up the Mosquitto Connection

### Linux Environment

1. Install Mosquitto, using the command --> `sudo apt install Mosquitto`
2. Copy file from https://github.com/eclipse/Mosquitto/blob/master/Mosquitto.conf and paste in linux machine.
3. Open the Mosquitto.conf file and include password_file (Mosquitto.pwd) in “General configuration”.
4. The password file contains the username and password in hashed format. To create your own username and password for Mosquitto use the following [link](http://surl.li/gnmwr). 
5. In the section “Listeners” change listener `<port no.> <ip address of linux machine>`.
6. In the same section find #protocol mqtt and uncomment it.
7. Follow [Openssl Certificate Creation](./OPENSSL_CERTIFICATE_CREATION.md) to create certificates.
8.  Provide the required certificates path in the “Certificate based SSL/TLS support” and in the same section set the flag require_certification to `true`.
9. In “Security” section change uncomment the flag allow_anonymous false.
10.  Now that your configuration file is set save it and run the following command in terminal to run Mosquitto:-
   - Mosquitto -v -c Mosquitto.conf

### Windows Environment

1. Install Mosquitto using the [mosquitto download](https://Mosquitto.org/download/).
1.  Open the Mosquitto.conf file and include password_file (Mosquitto.pwd) in “General configuration”.
2. The password file contains the username and password in hashed format. To create your own username and password for Mosquitto use the following [link](http://surl.li/gnmwr). 
3. Next in section “Listeners” change listener <port no.> <ip address of linux machine>.
4. In the same section find #protocol mqtt and uncomment it.
5. Follow [Openssl Certificate Creation](./OPENSSL_CERTIFICATE_CREATION.md) to create certificates.
6. Provide the required certificates path in the “Certificate based SSL/TLS support” and in the same section set the flag require_certification to `true`.
7. In “Security” section change uncomment the flag allow_anonymous false.
8. Now that your configuration file is set save it and run the following command in terminal(command prompt) to run Mosquitto:-
   - Mosquitto -v -c Mosquitto.conf