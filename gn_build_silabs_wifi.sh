#!/usr/bin/env bash

set -ex

## BRD4161A
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug/ BRD4161A disable_lcd=false use_external_flash=true | tee lighting-app.build.log

./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug/disable_lcd_true_use_external_flash_true_SiWx917 BRD4161A --wifi SiWx917 chip_enable_ble_rs911x=true disable_lcd=true use_external_flash=true | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug/disable_lcd_true_use_external_flash_false_SiWx917 BRD4161A --wifi SiWx917 chip_enable_ble_rs911x=true disable_lcd=true use_external_flash=false | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug/disable_lcd_false_use_external_flash_true_SiWx917 BRD4161A --wifi SiWx917 chip_enable_ble_rs911x=true disable_lcd=false use_external_flash=true | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug/disable_lcd_false_use_external_flash_false_SiWx917 BRD4161A --wifi SiWx917 chip_enable_ble_rs911x=true disable_lcd=false use_external_flash=false | tee lighting-app.build.log

./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug/disable_lcd_true_use_external_flash_true_rs9116 BRD4161A --wifi rs9116 chip_enable_ble_rs911x=true disable_lcd=true use_external_flash=true | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug/disable_lcd_true_use_external_flash_false_rs9116 BRD4161A --wifi rs9116 chip_enable_ble_rs911x=true disable_lcd=true use_external_flash=false | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug/disable_lcd_false_use_external_flash_true_rs9116 BRD4161A --wifi rs9116 chip_enable_ble_rs911x=true disable_lcd=false use_external_flash=true | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug/disable_lcd_false_use_external_flash_false_rs9116 BRD4161A --wifi rs9116 chip_enable_ble_rs911x=true disable_lcd=false use_external_flash=false | tee lighting-app.build.log

./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug/disable_lcd_true_use_external_flash_true_wf200 BRD4161A --wifi wf200 disable_lcd=true use_external_flash=true chip_logging=false silabs_log_enabled=false is_debug=false | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug/disable_lcd_true_use_external_flash_false_wf200 BRD4161A --wifi wf200 disable_lcd=true use_external_flash=false chip_logging=false silabs_log_enabled=false is_debug=false | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug/disable_lcd_false_use_external_flash_true_wf200 BRD4161A --wifi wf200 disable_lcd=false use_external_flash=true chip_logging=false silabs_log_enabled=false is_debug=false | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug/disable_lcd_false_use_external_flash_false_wf200 BRD4161A --wifi wf200 disable_lcd=false use_external_flash=false chip_logging=false silabs_log_enabled=false is_debug=false | tee lighting-app.build.log

## BRD4187C
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug/ BRD4187C disable_lcd=false use_external_flash=true | tee lighting-app.build.log

./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug/disable_lcd_true_use_external_flash_true_SiWx917 BRD4187C --wifi SiWx917 chip_enable_ble_rs911x=true disable_lcd=true use_external_flash=true | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug/disable_lcd_true_use_external_flash_false_SiWx917 BRD4187C --wifi SiWx917 chip_enable_ble_rs911x=true disable_lcd=true use_external_flash=false | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug/disable_lcd_false_use_external_flash_true_SiWx917 BRD4187C --wifi SiWx917 chip_enable_ble_rs911x=true disable_lcd=false use_external_flash=true | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug/disable_lcd_false_use_external_flash_false_SiWx917 BRD4187C --wifi SiWx917 chip_enable_ble_rs911x=true disable_lcd=false use_external_flash=false | tee lighting-app.build.log

./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug/disable_lcd_true_use_external_flash_true_rs9116 BRD4187C --wifi rs9116 chip_enable_ble_rs911x=true disable_lcd=true use_external_flash=true | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug/disable_lcd_true_use_external_flash_false_rs9116 BRD4187C --wifi rs9116 chip_enable_ble_rs911x=true disable_lcd=true use_external_flash=false | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug/disable_lcd_false_use_external_flash_true_rs9116 BRD4187C --wifi rs9116 chip_enable_ble_rs911x=true disable_lcd=false use_external_flash=true | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug/disable_lcd_false_use_external_flash_false_rs9116 BRD4187C --wifi rs9116 chip_enable_ble_rs911x=true disable_lcd=false use_external_flash=false | tee lighting-app.build.log

./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug/disable_lcd_true_use_external_flash_true_wf200 BRD4187C --wifi wf200 disable_lcd=true use_external_flash=true | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug/disable_lcd_true_use_external_flash_false_wf200 BRD4187C --wifi wf200 disable_lcd=true use_external_flash=false | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug/disable_lcd_false_use_external_flash_true_wf200 BRD4187C --wifi wf200 disable_lcd=false use_external_flash=true | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug/disable_lcd_false_use_external_flash_false_wf200 BRD4187C --wifi wf200 disable_lcd=false use_external_flash=false | tee lighting-app.build.log
