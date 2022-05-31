# CHIP Tizen Lighting Example

## Building binary

Activating environment

```sh
source ./scripts/activate.sh
```

Generating tizen-arm-light

```sh
gn gen --check \
	--fail-on-unused-args \
	--export-compile-commands \
	--root=$PW_PROJECT_ROOT/examples/lighting-app/tizen \
	"--args=target_os=\"tizen\" target_cpu=\"arm\" tizen_sdk_root=\"$TIZEN_SDK_ROOT\" sysroot=\"$TIZEN_SDK_SYSROOT\"" \
	$PW_PROJECT_ROOT/out/tizen-arm-light
```

Building tizen-arm-light

```sh
ninja -C $PW_PROJECT_ROOT/out/tizen-arm-light
```

## Preparing Tizen SDK certificate

For packaging the Tizen APP, there is a need to generate an author certificate
and security profile using the commands described below. Change password and
author data as needed.

```sh
tizen certificate --alias=CHIP --name=CHIP --email=chip@tizen.org --password=chiptizen

tizen security-profiles add --active --name=CHIP --author=$HOME/tizen-sdk-data/keystore/author/author.p12 --password=chiptizen
```

## Packaging APP

```sh
ninja -C $PW_PROJECT_ROOT/out/tizen-arm-light chip-lighting-app:tpk
```

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
