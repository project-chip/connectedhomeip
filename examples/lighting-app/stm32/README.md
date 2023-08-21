-   Build the example application:

          cd ~/connectedhomeip
          ./scripts/examples/gn_stm32_example.sh ./examples/bridge-app/stm32/ ./out/bridge-app NUCLEO-H753ZI

*   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip
          $ rm -rf ./out/

    OR use GN/Ninja directly

          $ cd ~/connectedhomeip/examples/bridge-app/stm32
          $ git submodule update --init
          $ source third_party/connectedhomeip/scripts/activate.sh
          $ export stm32_BOARD=NUCLEO-H753ZI
          $ gn gen out/debug
          $ ninja -C out/debug

*   To delete generated executable, libraries and object files use:

          $ cd ~/connectedhomeip/examples/bridge-app/stm32
          $ rm -rf out/
