## Python tool to generate and parse Matter onboarding codes

Generates and parses Manual Pairing Code and QR Code

#### Installing the package

```
python3 -m pip install matter-setup-payload
```

#### Example usage from the command line

-   Parse

```
matter-setup-payload parse MT:U9VJ0OMV172PX813210
matter-setup-payload parse 34970112332
```

-   Generate

```
matter-setup-payload generate --help
matter-setup-payload generate -d 3840 -p 20202021
matter-setup-payload generate -d 3840 -p 20202021 --vendor-id 65521 --product-id 32768 -cf 0 -dm 2
```

#### Example usage from the python script

```
from matter_setup_payload import CommissioningFlow, SetupPayload

# Parse a setup payload
qrcode = "MT:U9VJ0OMV172PX813210"
manual_pairingcode = "34970112332"

parsed_manual = SetupPayload.parse(manual_pairingcode)
parsed_qr = SetupPayload.parse(qrcode)

# API to print parsed payloads
print(f"Parsed Manual: {parsed_manual.p_print()}")
print(f"Parsed QR: {parsed_qr.p_print()}")

# Generate a setup payload
payload = SetupPayload(
    discriminator = 3840,
    pincode = 20202021,
    rendezvous=4,
    flow=CommissioningFlow.Standard,
    vid=0,
    pid=0
)

manual_code = payload.generate_manualcode()
qr_code = payload.generate_qrcode()

print(f"Generated Manual Code: {manual_code}")
print(f"Generated QR Code: {qr_code}")
```
