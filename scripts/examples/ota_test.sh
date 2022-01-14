#!/usr/bin/env bash


#Sergei , test

echo Test passed && exit 0;

ARG1=${1:-20202021}
ARG2=${2:-42}
ARG3=${3:-5560}

pkill chip-ota-provider-app
pkill chip-ota-requestor-app 

scripts/examples/gn_build_example.sh examples/chip-tool out/

./out/ota_provider_debug/chip-ota-provider-app -f tmp/ota.txt | tee /tmp/ota/provider-log.txt  &
provider_pid=$!

echo  "Commissioning Provider "

./out/chip-tool pairing onnetwork 1 "$ARG1" | tee /tmp/ota/chip-tool-commission-provider.txt 
if grep "Device commissioning completed with success" /tmp/ota/chip-tool-commission-provider.txt;
then echo Provider Commissioned;
else echo Provider not commissioned properly;
fi

stdbuf -o0 ./out/ota_requestor_debug/chip-ota-requestor-app -u "$ARG3" -d "$ARG2" | tee /tmp/ota/requestor-log.txt & 
requestor_pid=$!

echo  "Commissioning Requestor "

./out/chip-tool pairing onnetwork-long 2 "$ARG1" "$ARG2" | tee /tmp/ota/chip-tool-commission-requestor.txt 

if grep "Device commissioning completed with success" /tmp/ota/chip-tool-commission-requestor.txt;
then echo Requestor Commissioned;
else echo Requestor not commissioned properly;
fi

echo  "Sending announce-ota-provider "

./out/chip-tool otasoftwareupdaterequestor announce-ota-provider 1 0 0 0 2 0 | tee /tmp/ota/chip-tool-announce-ota.txt 

echo  "Sleeping for 20 seconds "
sleep 20

echo "Exiting, logs are in tmp/ota/"

kill $provider_pid
kill $requestor_pid

if grep "OTA image downloaded to" /tmp/ota/requestor-log.txt;
then echo Test passed && exit 0;
else echo Test failed && exit 1;
fi
