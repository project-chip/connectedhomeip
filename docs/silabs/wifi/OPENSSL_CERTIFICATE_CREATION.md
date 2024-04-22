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
5. To generate CA verification certification key:
   - `openssl genrsa -out verification_cert.key 2048`
6. To generate Certificate Signing Request (CSR) file:
   - `openssl req -new -key verification_cert.key -out verification_cert_csr.csr`
   **NOTE:** 
	- The information that the openssl command prompts you for is shown in the following example. Except for the Common Name field, you can enter your own values or keep them blank.
	- In the Common Name field, paste the registration code from the Verification certificate container of the AWS IoT console. Reference Image [to add registration code](./images/openssl_csr_common_name_ref.png).
7. To generate CA verification certificate:
   - `openssl x509 -req -in verification_cert_csr.csr -CA CA.crt -CAkey CA.key -CAcreateserial -out verification_cert.pem -days 500 -sha256`
8. Repeat step 3 and 4 to create an additional set of certificate to use in MQTT explorer (ex: `explorer.crt` and `explorer.key`). (Create with different name for Identification).
