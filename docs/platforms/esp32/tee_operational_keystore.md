# Operational (NOC) keys in ESP-TEE

On SoCs that support ESP-TEE (Trusted Execution Environment, e.g. ESP32-C6),
the Matter **operational (NOC) private key** can be held entirely inside the
secure world. The key is generated in ESP-TEE secure storage, the CSR is signed
there, and every CASE signature is produced there. The private key is never
present in application RAM or in plaintext in flash.

This is the operational-key counterpart to the Device Attestation Certificate
(DAC) key protection described in
[`secure_cert_partition.md`](secure_cert_partition.md) §1.7. The two are
independent and can be enabled together — with both on, **neither** the DAC key
nor any NOC key ever leaves the TEE.

## 1. Enabling

The TEE operational keystore is wired automatically whenever ESP-TEE is enabled;
there is no separate Matter option to turn on.

```
CONFIG_SECURE_ENABLE_TEE=y
```

At server start-up (`Esp32AppServer::Init`) the application installs an
`ESP32TEEOperationalKeystore` into `initParams.operationalKeystore`, replacing
the default software `PersistentStorageOperationalKeystore`. Confirm it is
active in the device log:

```
chip[SVR]: Operational keystore: ESP-TEE secure storage
```

The bundled `examples/lighting-app/esp32/sdkconfig.defaults.esp32c6_tee`
enables ESP-TEE (and the PBKDF2 TEE DAC), so a lighting-app built with that
defaults file gets the TEE operational keystore as well.

### Requirements

-   A target with ESP-TEE support and the `tee_sec_storage` component
    (`esp_tee_sec_storage_*` APIs).
-   A `secure_storage` NVS partition for the TEE (present in the
    `partitions_tee.csv` layout used by the `_tee` defaults).

## 2. How keys are stored

`OperationalKeystore` has fail-safe semantics: a key created by
`NewOpKeypairForFabric` is *pending* and must survive a fail-safe expiry
(`RevertPendingKeypair`) without disturbing the previously committed key, and is
only made permanent by `CommitOpKeypairForFabric`.

The software keystore keeps the pending key in RAM and writes it to storage only
on commit. A TEE key **cannot** live in RAM — `esp_tee_sec_storage_gen_key`
persists it immediately. To preserve the fail-safe semantics anyway, each fabric
uses **two secure-storage slots** plus a small persisted pointer:

| Item | Secure-storage / NVS id | Contents |
|------|-------------------------|----------|
| Slot A | `opk-<fabricIndex>-A` | a `SECP256R1` key in TEE secure storage |
| Slot B | `opk-<fabricIndex>-B` | a `SECP256R1` key in TEE secure storage |
| Active pointer | KVS `tso/<fabricIndex>` (1 byte) | `'A'` or `'B'` — the committed slot |

Lifecycle → slot mapping:

-   **NewOpKeypairForFabric** — generate into the *inactive* slot (the one the
    pointer does **not** name), so the committed key stays usable. Build and sign
    the CSR in the TEE.
-   **ActivateOpKeypairForFabric** — verify the pending slot's public key matches
    the incoming NOC public key.
-   **CommitOpKeypairForFabric** — write the active pointer to the pending slot
    (the atomic commit point), then delete the superseded slot's key (rotation).
-   **RevertPendingKeypair** — clear the pending (inactive) slot's key; the
    committed key and pointer are untouched.
-   **RemoveOpKeypairForFabric** — delete both slot keys and the pointer.
-   **SignWithOpKeypair** — sign with the pending slot if a pending key is active
    for the fabric, otherwise with the committed slot.

Because signing goes through `esp_tee_sec_storage_ecdsa_sign`, the CASE signature
is computed inside the TEE. The only application-visible NVS record per fabric is
the 1-byte `tso/<idx>` pointer; the key material lives in the `secure_storage`
partition, which the REE cannot read.

### CSR generation

The CSR is **not** built through the mbedTLS opaque-key wrapper. The public key
is read from the TEE, the PKCS#10 `CertificationRequestInfo` is assembled from
fixed P-256 ASN.1 templates, and its signature is produced with the same raw
`esp_tee_sec_storage_ecdsa_sign` primitive used for CASE. This keeps a single
signing path and avoids depending on the mbedTLS TEE-pk integration.

## 3. Specification notes

Matter Core §6.4.6.1 (Node Operational CSR Procedure) requires the candidate
operational key pair to be:

-   valid only for the duration of the in-progress Fail-Safe Context (2a), and
-   committed to persistent storage only upon successful `AddNOC`/`UpdateNOC`
    with a NOC whose public key matches the candidate (2c).

**Deviation and how it is bounded.** A TEE key is persisted at generation, so the
candidate key is briefly on flash before commit — it cannot be held RAM-only.
The two-slot scheme keeps the *committed* key the sole active key until commit:
the candidate lives in the inactive slot, is cleared by `RevertPendingKeypair`
on fail-safe expiry, and is never selected by `SignWithOpKeypair` unless it was
activated. If power is lost mid-fail-safe, a candidate key can linger in the
inactive slot; it is bounded to at most one inactive slot per fabric and is
overwritten by the next `NewOpKeypairForFabric` (which always targets the
inactive slot) or removed by `RemoveOpKeypairForFabric`. On reboot the RAM
pending state is gone, so the aborted candidate is never treated as active —
matching the intent that a fail-safe that did not complete leaves no usable key.

## 4. Behavior details

-   **ExportOpKeypairForFabric** returns `CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE` —
    TEE keys are non-exportable by design. Migration from another keystore into
    this one is therefore not supported.
-   **CASE ephemeral keys** (`AllocateEphemeralKeypairForCASE`) remain in
    software. These are short-lived per-session keys, not the operational key.

## 5. Verifying on-device

### Self-test (no commissioning required)

Enable the bring-up self-tests:

```
CONFIG_ENABLE_ESP32_TEE_OPKEY_SELFTEST=y
```

At boot the application runs, against the real TEE:

-   `ESP32TEEOpKeySelfTest()` — generate a throwaway key, derive its public key,
    build and verify a CSR, sign and verify a message, then delete the key.
-   `ESP32TEEOperationalKeystoreSelfTest()` — the full keystore lifecycle
    (New → Activate → Commit → Sign → rotate → Revert → Remove) against a
    throwaway fabric index, verifying every signature and transition.

Expected log:

```
chip[SVR]: TEE op-key self-test: PASSED
chip[SVR]: TEE op-keystore lifecycle self-test: PASSED
```

Leave this option off (default) for production builds.

### Live commissioning logs

During a normal commission and operation the keystore logs each operation, so
you can confirm from the device console that NOC keys are handled in the TEE:

```
chip[Crypto]: TEE opkey: generated NOC keypair for fabric 0x1 in TEE secure storage (slot A), CSR signed in TEE
chip[Crypto]: TEE opkey: committed NOC keypair for fabric 0x1 to TEE secure storage (slot A)
chip[Crypto]: TEE opkey: signing (CASE) for fabric 0x1 with TEE key slot A     # Detail level
chip[Crypto]: TEE opkey: cleared TEE secure-storage NOC key slots for fabric 0x1
```

The absence of any software-keystore key write (no operational-key blob in the
application NVS) is the corresponding negative check.

## 6. Source

| File | Role |
|------|------|
| `src/platform/ESP32/ESP32TEEOpKey.{h,cpp}` | Low-level TEE key primitives (generate / public key / CSR / sign / remove) + self-test. |
| `src/platform/ESP32/ESP32TEEOperationalKeystore.{h,cpp}` | `Crypto::OperationalKeystore` implementation (two-slot + pointer scheme) + lifecycle self-test. |
| `examples/platform/esp32/common/Esp32AppServer.cpp` | Installs the keystore into `initParams` under `CONFIG_SECURE_ENABLE_TEE`. |

> **Note (IDF version):** `esp_tee_sec_storage_ecdsa_sign_t` changed in IDF v6.0
> (separate `sign_r`/`sign_s` fields became a single `signature[]`). The code
> handles both via an `ESP_IDF_VERSION` guard, so it builds on IDF 5.5.x (with
> the TEE secure-storage additions) and on IDF 6.0.x.
