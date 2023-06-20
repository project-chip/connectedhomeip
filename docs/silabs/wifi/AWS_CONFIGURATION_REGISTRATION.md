# Amazon Web Services (AWS)
- Amazon Web Services offers reliable, scalable, and inexpensive cloud computing services. Refer to [AWS Documentation](https://aws.amazon.com/what-is-aws/) for more details.
## AWS CA Certitifcate Registration

1. Create a CA certificate, a client certificate and a client key using the [Openssl Certificate Creation](./OPENSSL_CERTIFICATE_CREATION.md) documentation.
2. Open [AWS](https://aws.amazon.com/)
3. Login using your AWS credentials
4. Register the CA Certificate in AWS:
    - Go to `Security -> Certificate Authorities` and `Register CA Certificate`
    - Select `Register CA` in the Multi-account mode
    - Choose the CA certificate that you previously created in the Openssl Certificate Creation (CA.crt) step in the CA certificate registration, and set the CA status to `Active` and the `Automatic certificate registration` option to `ON`
    - Register the CA.
5. Go to `Security -> Policies` and select `Create Policy`. Enter the policy name (ex: `DIC_POLICY`) and in the policy statements select `JSON` and replace the contents with the JSON provided below:
```
{
  "Version": "2012-10-17",
  "Statement": [
    {
      "Effect": "Allow",
      "Action": "*",
      "Resource": "*"
    }
  ]
}
```
Once done, select `Create`.

6. Steps to generate the certificate for your Matter application to use in the `dic_config.h` header.
    - Go to `All Devices -> Things` and select `Create Things`.
    - Select `Create Single Thing` and click **Next**.
    - Specify thing properties Info -> Give the thing a name (Note: Client ID) and click **Next**.
    - Configure the device certificate - optional Info -> Use my certificate.
    - Certificate details -> Choose `CA is registered with AWS IOT` and Select the CA that registered with AWS in Step 4.
    - Certificate -> Choose file (Choose Client certificate generated in Openssl Certificate Creation ex: `device.crt`) and set the certificate status to `Active`. Click **Next**.
    - Use the policy(ex: `DIC_POLICY`) created in AWS Certificate Creation.

7. Repeat Step 5 to create a new thing to use in MQTT Explorer using the certificate created for MQTT explorer ( from Openssl Certificate Creation ex: `explorer.crt`)
    - NOTE:
      - Thing name must be unique as it will be used as CLIENT ID. 
  
8. Copy the contents of [AWS_CA CERT](https://www.amazontrust.com/repository/AmazonRootCA1.pem) and create a .pem file to use as a SERVER CERTIFICATE in MQTT Explorer.
