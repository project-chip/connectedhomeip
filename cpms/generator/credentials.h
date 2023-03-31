#pragma once


#include <stddef.h>
#include <stdint.h>
#include <psa/crypto.h>


class Credentials
{
public:
    Credentials();
    Credentials(psa_key_id_t key_id);
    // Key management
    int destroyKey();
    int generateKey();
    int importKey(const uint8_t * asn1, size_t size);
    int ImportRawKey(const uint8_t * key, size_t size);
    psa_key_id_t getKeyId() { return _key_id; }

    int generateCSR(const char *common_name, uint16_t vendor_id, uint16_t product_id, char * buffer, size_t max_size, size_t * size);
    static int parseCert(const uint8_t * cert_data, size_t cert_size, uint8_t *serial_num, size_t serial_max, size_t &serial_size);

private:
    psa_key_id_t _key_id = 0;
};
