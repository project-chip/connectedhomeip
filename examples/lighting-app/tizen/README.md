# CHIP Tizen Lighting Example

## Installing TPK

Upload TPK package to device under test (DUT). Install it with `pkgcmd` as
follows:

```sh
pkgcmd -i -t tpk -p org.tizen.matter.example.lighting-1.0.0.tpk
```

## Launching application

For launching Tizen application one should use `app_launcher`. It is possible to
pass user arguments from command line which might be used to control application
behavior. However, passed strings cannot start with "-" (minus) character and
all arguments have to consist of name and value. Boolean options (option without
argument) should have value equal to "true".

e.g.:

```sh
app_launcher --start=org.tizen.matter.example.lighting discriminator 43 wifi true
```
