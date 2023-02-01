## Python tool to generate Matter onboarding codes

Generates and parses Manual Pairing Code and QR Code

#### example usage:

```
./generate_setup_payload.py -h
./generate_setup_payload.py -d 3840 -p 20202021 -cf 0 -dm 2 -vid 65521 -pid 32768
./parse_setup_payload.py -q MT:Y.K9042C00KA0648G00
```

-   Output

```
Manualcode : 34970112332
QRCode     : MT:Y.K9042C00KA0648G00
```

For more details please refer Matter Specification

---
