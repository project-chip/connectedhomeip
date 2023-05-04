# Openssl Certificate Creation
  An SSL certificate is an important way to secure user information and protect against hackers.
## Openssl Installation (In ubuntu)
1. To install openssl -  `sudo apt install openssl`

## Certificates Creation
The following commands are used to generate certificates:
1. To generate CA key:
   - `openssl ecparam -name prime256v1 -genkey -noout -out CA.key`
2. To generate CA certificate:
   - `openssl req -new -x509 -days 1826 -key CA.key -out CA.crt`
3. To generate Client key:
   - `openssl ecparam -name prime256v1 -genkey -noout -out device.key`
4. To generate Client certificate (ex: `device.crt` and `device.key`) using CA certficate:
   - `openssl req -new -out device.csr -key device.key`
   - `openssl x509 -req -in device.csr -CA CA.crt -CAkey CA.key -CAcreateserial -out device.crt -days 360`
5. Repeat step 3 and 4 to create an additional set of certificate to use in MQTT explorer (ex: `explorer.crt` and `explorer.key`). (Create with different name for Identification)