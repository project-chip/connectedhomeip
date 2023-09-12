#!/usr/bin/env bash

set -e

## BRD4161A
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug BRD4161A --wifi SiWx917 disable_lcd=true use_external_flash=true | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug BRD4161A --wifi SiWx917 disable_lcd=true use_external_flash=false | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug BRD4161A --wifi SiWx917 disable_lcd=false use_external_flash=true | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug BRD4161A --wifi SiWx917 disable_lcd=false use_external_flash=false | tee lighting-app.build.log

./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug BRD4161A --wifi rs9116 disable_lcd=true use_external_flash=true | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug BRD4161A --wifi rs9116 disable_lcd=true use_external_flash=false | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug BRD4161A --wifi rs9116 disable_lcd=false use_external_flash=true | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug BRD4161A --wifi rs9116 disable_lcd=false use_external_flash=false | tee lighting-app.build.log

./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug BRD4161A --wifi wf200 disable_lcd=true use_external_flash=true chip_logging=false silabs_log_enabled=false is_debug=false | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug BRD4161A --wifi wf200 disable_lcd=true use_external_flash=false chip_logging=false silabs_log_enabled=false is_debug=false | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug BRD4161A --wifi wf200 disable_lcd=false use_external_flash=true chip_logging=false silabs_log_enabled=false is_debug=false | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug BRD4161A --wifi wf200 disable_lcd=false use_external_flash=false chip_logging=false silabs_log_enabled=false is_debug=false | tee lighting-app.build.log

## BRD4187C
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug BRD4187C --wifi SiWx917 disable_lcd=true use_external_flash=true | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug BRD4187C --wifi SiWx917 disable_lcd=true use_external_flash=false | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug BRD4187C --wifi SiWx917 disable_lcd=false use_external_flash=true | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug BRD4187C --wifi SiWx917 disable_lcd=false use_external_flash=false | tee lighting-app.build.log

./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug BRD4187C --wifi rs9116 disable_lcd=true use_external_flash=true | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug BRD4187C --wifi rs9116 disable_lcd=true use_external_flash=false | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug BRD4187C --wifi rs9116 disable_lcd=false use_external_flash=true | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug BRD4187C --wifi rs9116 disable_lcd=false use_external_flash=false | tee lighting-app.build.log

./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug BRD4187C --wifi wf200 disable_lcd=true use_external_flash=true | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug BRD4187C --wifi wf200 disable_lcd=true use_external_flash=false | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug BRD4187C --wifi wf200 disable_lcd=false use_external_flash=true | tee lighting-app.build.log
./scripts/examples/gn_silabs_example.sh ./examples/lighting-app/silabs ./out/debug BRD4187C --wifi wf200 disable_lcd=false use_external_flash=false | tee lighting-app.build.log
