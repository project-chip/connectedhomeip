#!/usr/bin/env bash
for target in \
      linux-arm64-chip-cert-clang \
      linux-arm64-all-clusters-clang \
      linux-arm64-chip-tool-ipv6only-clang \
      linux-arm64-chip-tool-nodeps-ipv6only \
      linux-arm64-lock-clang \
      linux-arm64-minmdns-clang \
      linux-arm64-light-rpc-ipv6only-clang \
      linux-arm64-thermostat-no-ble-clang \
      linux-arm64-lit-icd-no-ble-clang \
      linux-arm64-fabric-admin-clang-rpc \
      linux-arm64-fabric-bridge-no-ble-clang-rpc \
      linux-arm64-fabric-sync-no-ble-clang \
      linux-arm64-camera-controller-clang \ \
      linux-arm64-camera-clang \
; \
do
  echo "Test $target"
done
