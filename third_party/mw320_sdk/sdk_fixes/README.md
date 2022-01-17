Before  build the Matter NXP MW320 example, you need to install MW320 mcuxpresso at http://mcuxpresso.nxp.com.
Please follow the install step:
1. Create MW320 folder
    mkdir ~/Desktop/MW320
2. Set SDK environment
    export MW320_FOLDER=~/Desktop/MW320
3. Running patch_mw320_sdk.sh
    ./third_party/mw320_sdk/sdk_fixes/patch_mw320_sdk.sh
4. Set SDK environment
    source ./third_party/mw320_sdk/sdk_fixes/set_env.sh

MW320_SDK_ROOT will be configured to build MW320 Matter application. Please go to examples folder to build MW320 Matter.
lighting-app:
    cd $MW320_LIGHTING
    source third_party/connectedhomeip/scripts/activate.sh
    gn gen out/mw320 --args='chip_bypass_rendezvous=true is_debug=false treat_warnings_as_errors=false'
    ninja -v -C out/mw320
    $MW320_SDK_ROOT/tools/mw_img_conv/src/mw_img_conv mcufw $MW320_LIGHTING/out/mw320/chip-mw320-light-example.bin $MW320_LIGHTING/out/mw320/chip-mw320-light-example.mcufw.bin 0x1F000100
    cp $MW320_LIGHTING/out/mw320/chip-mw320-light-example.mcufw.bin $MW320_FOLDER/mw320_matter_openocd/Matter/chip-mw320-light-example.mcufw.bin
    
    sudo $MW320_FOLDER/mw320_matter_openocd/flashprog.py -l $MW320_FOLDER/mw320_matter_openocd/Matter/layout.txt --boot2 $MW320_FOLDER/mw320_matter_openocd/Matter/boot2.bin
    sudo $MW320_FOLDER/mw320_matter_openocd/flashprog.py --wififw $MW320_FOLDER/mw320_matter_openocd/Matter/mw32x_uapsta_W14.88.36.p172.bin
    sudo $MW320_FOLDER/mw320_matter_openocd/flashprog.py --mcufw $MW320_FOLDER/mw320_matter_openocd/Matter/chip-mw320-light-example.mcufw.bin -r
    
    wifi connect nxp_matter_ces nxp12345
    ./chip-tool pairing bypass FD17:BBF9:BE59:0:250:43FF:FE22:55E7 5540
    ./chip-tool onoff read on-off 1
    

