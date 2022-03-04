FROM connectedhomeip/chip-build AS chip-build
WORKDIR /root/
RUN git clone --depth 1 https://github.com/project-chip/connectedhomeip.git
WORKDIR /root/connectedhomeip/
RUN scripts/build/gn_bootstrap.sh
RUN gn gen out/debug --args='chip_mdns="platform" chip_inet_config_enable_ipv4=false'
RUN ninja -C out/debug
RUN scripts/examples/gn_build_example.sh examples/all-clusters-app/linux/ examples/all-clusters-app/linux/out/all-clusters-app chip_inet_config_enable_ipv4=false
RUN scripts/examples/gn_build_example.sh examples/bridge-app/linux/ examples/bridge-app/linux/out/bridge-app chip_inet_config_enable_ipv4=false
RUN scripts/examples/gn_build_example.sh examples/tv-app/linux/ examples/tv-app/linux/out/tv-app chip_inet_config_enable_ipv4=false
RUN scripts/examples/gn_build_example.sh examples/tv-casting-app/linux/ examples/tv-casting-app/linux/out/tv-casting-app chip_inet_config_enable_ipv4=false
RUN scripts/examples/gn_build_example.sh examples/lighting-app/linux/ examples/lighting-app/linux/out/lighting-app chip_inet_config_enable_ipv4=false
RUN scripts/examples/gn_build_example.sh examples/thermostat/linux/ examples/thermostat/linux/out/thermostat chip_inet_config_enable_ipv4=false
RUN scripts/examples/gn_build_example.sh examples/ota-provider-app/linux examples/ota-provider-app/linux/out/host 'chip_config_network_layer_ble=false'
RUN scripts/examples/gn_build_example.sh examples/ota-requestor-app/linux examples/ota-requestor-app/linux/out/host 'chip_config_network_layer_ble=false'
RUN scripts/examples/gn_build_example.sh examples/door-lock-app/linux/ out/door-lock-app chip_inet_config_enable_ipv4=false

FROM ubuntu:21.10
ENV TZ=America/Los_Angeles
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN apt-get update -y
RUN apt-get install -y libssl-dev libdbus-1-dev libglib2.0-dev libavahi-client-dev avahi-utils iproute2
WORKDIR /root/
COPY --from=chip-build /root/connectedhomeip/out/debug/chip-tool chip-tool
COPY --from=chip-build /root/connectedhomeip/out/debug/chip-shell chip-shell
COPY --from=chip-build /root/connectedhomeip/out/debug/chip-cert chip-cert
COPY --from=chip-build /root/connectedhomeip/examples/all-clusters-app/linux/out/all-clusters-app/chip-all-clusters-app all-cluster-app
COPY --from=chip-build /root/connectedhomeip/examples/lighting-app/linux/out/lighting-app/chip-lighting-app lighting-app
COPY --from=chip-build /root/connectedhomeip/examples/tv-casting-app/linux/out/tv-casting-app/chip-tv-casting-app tv-casting-app
COPY --from=chip-build /root/connectedhomeip/examples/tv-app/linux/out/tv-app/chip-tv-app tv-app
COPY --from=chip-build /root/connectedhomeip/examples/bridge-app/linux/out/bridge-app/chip-bridge-app bridge-app
COPY --from=chip-build /root/connectedhomeip/examples/thermostat/linux/out/thermostat/thermostat-app thermostat-app
COPY --from=chip-build /root/connectedhomeip/examples/ota-requestor-app/linux/out/host/chip-ota-requestor-app ota-requestor-app
COPY --from=chip-build /root/connectedhomeip/examples/ota-provider-app/linux/out/host/chip-ota-provider-app ota-provider-app
COPY --from=chip-build /root/connectedhomeip/out/door-lock-app/chip-door-lock-app door-lock-app
