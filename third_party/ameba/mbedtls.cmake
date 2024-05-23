cmake_minimum_required(VERSION 3.6)

project(mbedtls)

set(dir "${sdk_root}/component/common/application/matter/mbedtls")
set(dir_mbedtlschip "${ameba_matter_root}/third_party/mbedtls/repo/library")

list(
    APPEND ${list}

    # mbedtls-chip
    ${dir}/net_sockets.c
    ${dir_mbedtlschip}/aes.c
    ${dir_mbedtlschip}/aesni.c
    ${dir_mbedtlschip}/arc4.c
    ${dir_mbedtlschip}/aria.c
    ${dir_mbedtlschip}/asn1parse.c
    ${dir_mbedtlschip}/asn1write.c
    ${dir_mbedtlschip}/base64.c
    ${dir_mbedtlschip}/bignum.c
    ${dir_mbedtlschip}/blowfish.c
    ${dir_mbedtlschip}/camellia.c
    ${dir_mbedtlschip}/ccm.c
    ${dir_mbedtlschip}/certs.c
    ${dir_mbedtlschip}/chacha20.c
    ${dir_mbedtlschip}/chachapoly.c
    ${dir_mbedtlschip}/cipher.c
    ${dir_mbedtlschip}/cipher_wrap.c
    ${dir_mbedtlschip}/cmac.c
    ${dir_mbedtlschip}/constant_time.c
    ${dir_mbedtlschip}/ctr_drbg.c
    ${dir_mbedtlschip}/debug.c
    ${dir_mbedtlschip}/des.c
    ${dir_mbedtlschip}/dhm.c
    ${dir_mbedtlschip}/ecdh.c
    ${dir_mbedtlschip}/ecdsa.c
    ${dir_mbedtlschip}/ecjpake.c
    ${dir_mbedtlschip}/ecp.c
    ${dir_mbedtlschip}/ecp_curves.c
    ${dir_mbedtlschip}/entropy.c
    ${dir_mbedtlschip}/entropy_poll.c
    ${dir_mbedtlschip}/error.c
    ${dir_mbedtlschip}/gcm.c
    ${dir_mbedtlschip}/havege.c
    ${dir_mbedtlschip}/hkdf.c
    ${dir_mbedtlschip}/hmac_drbg.c
    ${dir_mbedtlschip}/md2.c
    ${dir_mbedtlschip}/md4.c
    ${dir_mbedtlschip}/md5.c
    ${dir_mbedtlschip}/md.c
    ${dir_mbedtlschip}/memory_buffer_alloc.c
    ${dir_mbedtlschip}/nist_kw.c
    ${dir_mbedtlschip}/oid.c
    ${dir_mbedtlschip}/padlock.c
    ${dir_mbedtlschip}/pem.c
    ${dir_mbedtlschip}/pk.c
    ${dir_mbedtlschip}/pkcs11.c
    ${dir_mbedtlschip}/pkcs12.c
    ${dir_mbedtlschip}/pkcs5.c
    ${dir_mbedtlschip}/pkparse.c
    ${dir_mbedtlschip}/pk_wrap.c
    ${dir_mbedtlschip}/pkwrite.c
    ${dir_mbedtlschip}/platform.c
    ${dir_mbedtlschip}/platform_util.c
    ${dir_mbedtlschip}/poly1305.c
    ${dir_mbedtlschip}/ripemd160.c
    ${dir_mbedtlschip}/rsa.c
    ${dir_mbedtlschip}/rsa_internal.c
    ${dir_mbedtlschip}/sha1.c
    ${dir_mbedtlschip}/sha256.c
    ${dir_mbedtlschip}/sha512.c
    ${dir_mbedtlschip}/ssl_cache.c
    ${dir_mbedtlschip}/ssl_ciphersuites.c
    ${dir_mbedtlschip}/ssl_cli.c
    ${dir_mbedtlschip}/ssl_cookie.c
    ${dir_mbedtlschip}/ssl_srv.c
    ${dir_mbedtlschip}/ssl_ticket.c
    ${dir_mbedtlschip}/ssl_tls.c
    ${dir_mbedtlschip}/threading.c
    ${dir_mbedtlschip}/timing.c
    ${dir_mbedtlschip}/version.c
    ${dir_mbedtlschip}/version_features.c
    ${dir_mbedtlschip}/x509.c
    ${dir_mbedtlschip}/x509_create.c
    ${dir_mbedtlschip}/x509_crl.c
    ${dir_mbedtlschip}/x509_crt.c
    ${dir_mbedtlschip}/x509_csr.c
    ${dir_mbedtlschip}/x509write_crt.c
    ${dir_mbedtlschip}/x509write_csr.c
    ${dir_mbedtlschip}/xtea.c
)

