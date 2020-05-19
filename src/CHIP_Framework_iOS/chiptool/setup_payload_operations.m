//
//  setup_payload_operations.m
//  chiptool
//
//  Created by Shana Azria on 15/05/2020.
//  Copyright © 2020 CHIP. All rights reserved.
//

#import "setup_payload_operations.h"
#import "CHIPToolCommandManager.h"
#import "CHIPSetupPayloadHelper.h"

typedef NS_ENUM(NSInteger, SetupPayloadCodeType) {
    SetupPayloadCodeTypeQR = 0,
    SetupPayloadCodeTypeManual = 1,
};

static int do_setup_payload_operation_generate_code(SetupPayloadCodeType type, const char * filePathUTF8String)
{
    int result = 0;

    NSString * filePath = filePathUTF8String ? [NSString stringWithUTF8String:filePathUTF8String] : nil;

    if (!filePath) {
        CHIPPrintf(@"Empty file path supplied");
        return 2;
    } else {
        CHIPPrintf(@"Loading payload at filepath: %@", filePath);
        NSError * error;
        NSString * code;
        switch (type) {
        case SetupPayloadCodeTypeQR:
            code = [CHIPSetupPayloadHelper generateQRCodeFromFilePath:filePath error:&error];
            break;
        case SetupPayloadCodeTypeManual:
            code = [CHIPSetupPayloadHelper generateManualCodeFromFilePath:filePath error:&error];
            break;
        default:
            CHIPPrintf(@"Invalid code options");
            return 2;
        }
        if (error) {
            CHIPPrintf(@"Failed to generate code with error.");
            return 2;
        } else {
            CHIPPrintf(@"Generated code %@", code);
        }
    }

    return result;
}

int setup_payload_operation_generate_qr_code(int argc, char * const * argv)
{
    int ch;
    const char * filePath = NULL;

    while ((ch = getopt(argc, argv, "f:")) != -1) {
        switch (ch) {
        case 'f':
            filePath = optarg;
            break;

        case '?':
        default:
            return 2; /* @@@ Return 2 triggers usage message. */
        }
    }

    return do_setup_payload_operation_generate_code(SetupPayloadCodeTypeQR, filePath);
}

int setup_payload_operation_generate_manual_code(int argc, char * const * argv)
{
    int ch;
    const char * filePath = NULL;

    while ((ch = getopt(argc, argv, "f:")) != -1) {
        switch (ch) {
        case 'f':
            filePath = optarg;
            break;

        case '?':
        default:
            return 2; /* @@@ Return 2 triggers usage message. */
        }
    }

    return do_setup_payload_operation_generate_code(SetupPayloadCodeTypeManual, filePath);
}
