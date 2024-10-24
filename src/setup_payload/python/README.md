## Python tool to generate and parse Matter onboarding codes

Generates and parses Manual Pairing Code and QR Code

#### example usage:

-   Parse

```
./SetupPayload.py parse MT:U9VJ0OMV172PX813210
./SetupPayload.py parse 34970112332
```

-   Generate

```
./SetupPayload.py generate --help
./SetupPayload.py generate -d 3840 -p 20202021
./SetupPayload.py generate -d 3840 -p 20202021 --vendor-id 65521 --product-id 32768 -cf 0 -dm 2
```

For more details please refer Matter Specification
