#include "QRCodeSetupPayloadGenerator.h"
#include "SetupCodeUtils.h"

#include <iostream>
#include <vector>

using namespace chip;
using namespace std;





void QRCodeSetupPayloadGenerator::_resetBitSet() 
{
    payloadBitsIndex = kTotalPayloadDataSizeInBits;
    payloadBits.reset();
}

// Populates numberOfBits starting from LSB of input into payloadBits
void QRCodeSetupPayloadGenerator::_populateInteger(uint64_t input, size_t numberOfBits) 
{
    int numberOfBitsModified = 0;
    
    payloadBitsIndex -= numberOfBits;
    int currentIndex = payloadBitsIndex;

    if (enableDebugLogs) {
        printf("\nStart index %d", currentIndex);
    }
    int endIndex = currentIndex + (numberOfBits - 1);

    if (enableDebugLogs) {
        printf("\nEnd index %lu", currentIndex + (numberOfBits - 1));
        printf ("\nLength %lu\n", numberOfBits);
    }

    int itercount = 1;
    while (currentIndex <= endIndex) {
        //cout << itercount << "-- " << currentIndex << endl;
        itercount++;
        input & 1 ? payloadBits.set(currentIndex) : payloadBits.reset(currentIndex);
        currentIndex++;
        input /= 2;
    }
}

void QRCodeSetupPayloadGenerator::_populateVersion()
{
    if (enableDebugLogs) {
        cout << __func__ << "\n";
    }
    _populateInteger(payload.version, kVersionFieldLengthInBits);
}

void QRCodeSetupPayloadGenerator::_populateVendorID()
{
    if (enableDebugLogs) {
        cout << __func__ << "\n";
    }
    _populateInteger(payload.vendorID, kVendorIDFieldLengthInBits);
}

void QRCodeSetupPayloadGenerator::_populateProductID()
{
    if (enableDebugLogs) {
        cout << __func__ << "\n";
    }
    _populateInteger(payload.productID, kProductIDFieldLengthInBits);
}


void QRCodeSetupPayloadGenerator::_populateCustomFlowRequiredField()
{
    if (enableDebugLogs) {
        cout << __func__ << "\n";
    }
    _populateInteger(payload.requiresCustomFlow, kCustomFlowRequiredFieldLengthInBits);
}

void QRCodeSetupPayloadGenerator::_populateRendezVousInfo()
{
    if (enableDebugLogs) {
        cout << __func__ << "\n";
    }
    _populateInteger(payload.rendezvousInformation, kRendezvousInfoFieldLengthInBits);
}

void QRCodeSetupPayloadGenerator::_populateDiscriminator()
{
    if (enableDebugLogs) {
        cout << __func__ << "\n";
    }
    _populateInteger(payload.discriminator, kPayloadDiscriminatorFieldLengthInBits);
}

void QRCodeSetupPayloadGenerator::_populateSetupPIN()
{
    if (enableDebugLogs) {
        cout << __func__ << "\n";
    }
    _populateInteger(payload.setUpPINCode, kSetupPINCodeFieldLengthInBits);
}

void QRCodeSetupPayloadGenerator::_populateReservedField()
{
    if (enableDebugLogs) {
        cout << __func__ << "\n";
    }
    _populateInteger(0, kReservedFieldLengthInBits);
}

void QRCodeSetupPayloadGenerator::_generateBitSet()
{
    _resetBitSet();
    _populateVersion();
    _populateVendorID();
    _populateProductID();
    _populateCustomFlowRequiredField();
    _populateRendezVousInfo();
    _populateDiscriminator();
    _populateSetupPIN();
    _populateReservedField();
}

string QRCodeSetupPayloadGenerator::getQRCodePayloadBinaryRepresentation()
{
    _generateBitSet();
    return payloadBits.to_string();
}

// This function assumes bits.size() % 8 == 0
// TODO: Can this method be written in a more elegant way?
vector <uint16_t> _uint16ArrayFromBits(bitset<kTotalPayloadDataSizeInBits> bits)
{
    vector<uint16_t> resultVector;
    size_t numberOfBits = bits.size();
    size_t numberOfBytes = numberOfBits / 8;
    bool oddNumOfBytes = (numberOfBytes % 2 == 1) ? true : false; 
    
    // Traversing in reverse, hence startIndex > endIndex
    int endIndex = 0;
    int startIndex = bits.size() - 1;
    
    /*
    Every 2 bytes (16 bits) of binary source data are encoded to 3 characters of the Base-45 alphabet. 
    If an odd number of bytes are to be encoded, the remaining single byte will be encoded 
    to 2 characters of the Base-45 alphabet.
    */
    if (oddNumOfBytes) { 
         endIndex = 8;
    }

    while (startIndex > endIndex ) { 
        int currentIntegerIndex = startIndex;
        uint16_t result = 0;
        for (int i = currentIntegerIndex; i > currentIntegerIndex - 16; i--) {
            result = result << 1;
            result = result | bits.test(i);
        }
        resultVector.push_back(result);
        startIndex -= 16;
    }

    // If we have odd number of bytes append the last byte.
    if (oddNumOfBytes) {
        uint16_t result = 0;
        for (int i = 7; i >= 0 ; i--) {
            result = result << 1;
            result = result & bits.test(i);
        }
        resultVector.push_back(result);
    }
    return resultVector;
}   

string QRCodeSetupPayloadGenerator::getQRCodePayloadBase45Representation()
{
    _generateBitSet();
    vector<uint16_t> integerArray = _uint16ArrayFromBits(payloadBits);
    string result;
    if (enableDebugLogs) {
        cout << "\nEncoding bit set " << payloadBits << "\n";
    }
    for (int idx = 0; idx < integerArray.size(); idx++) {
        if (enableDebugLogs) {
            printf("\nEncoding %d\n", integerArray[idx]);
        }
        result += base45EncodedString(integerArray[idx], 3);
    }
    return result;
}
