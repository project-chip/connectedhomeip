"""PCSCPart10: PC/SC Part 10 (pinpad)

__author__ = "Ludovic Rousseau"

Copyright 2009-2010 Ludovic Rosseau
Author: Ludovic Rousseau, mailto:ludovic.rousseau@free.fr

This file is part of pyscard.

pyscard is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

pyscard is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with pyscard; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
"""

from smartcard.scard import SCARD_CTL_CODE, SCARD_SHARE_DIRECT

# constants defined in PC/SC v2 Part 10
CM_IOCTL_GET_FEATURE_REQUEST = SCARD_CTL_CODE(3400)

FEATURE_VERIFY_PIN_START = 0x01
FEATURE_VERIFY_PIN_FINISH = 0x02
FEATURE_MODIFY_PIN_START = 0x03
FEATURE_MODIFY_PIN_FINISH = 0x04
FEATURE_GET_KEY_PRESSED = 0x05
FEATURE_VERIFY_PIN_DIRECT = 0x06
FEATURE_MODIFY_PIN_DIRECT = 0x07
FEATURE_MCT_READER_DIRECT = 0x08
FEATURE_MCT_UNIVERSAL = 0x09
FEATURE_IFD_PIN_PROPERTIES = 0x0A
FEATURE_ABORT = 0x0B
FEATURE_SET_SPE_MESSAGE = 0x0C
FEATURE_VERIFY_PIN_DIRECT_APP_ID = 0x0D
FEATURE_MODIFY_PIN_DIRECT_APP_ID = 0x0E
FEATURE_WRITE_DISPLAY = 0x0F
FEATURE_GET_KEY = 0x10
FEATURE_IFD_DISPLAY_PROPERTIES = 0x11
FEATURE_GET_TLV_PROPERTIES = 0x12
FEATURE_CCID_ESC_COMMAND = 0x13

Features = {
    "FEATURE_VERIFY_PIN_START": FEATURE_VERIFY_PIN_START,
    "FEATURE_VERIFY_PIN_FINISH": FEATURE_VERIFY_PIN_FINISH,
    "FEATURE_MODIFY_PIN_START": FEATURE_MODIFY_PIN_START,
    "FEATURE_MODIFY_PIN_FINISH": FEATURE_MODIFY_PIN_FINISH,
    "FEATURE_GET_KEY_PRESSED": FEATURE_GET_KEY_PRESSED,
    "FEATURE_VERIFY_PIN_DIRECT": FEATURE_VERIFY_PIN_DIRECT,
    "FEATURE_MODIFY_PIN_DIRECT": FEATURE_MODIFY_PIN_DIRECT,
    "FEATURE_MCT_READER_DIRECT": FEATURE_MCT_READER_DIRECT,
    "FEATURE_MCT_UNIVERSAL": FEATURE_MCT_UNIVERSAL,
    "FEATURE_IFD_PIN_PROPERTIES": FEATURE_IFD_PIN_PROPERTIES,
    "FEATURE_ABORT": FEATURE_ABORT,
    "FEATURE_SET_SPE_MESSAGE": FEATURE_SET_SPE_MESSAGE,
    "FEATURE_VERIFY_PIN_DIRECT_APP_ID": FEATURE_VERIFY_PIN_DIRECT_APP_ID,
    "FEATURE_MODIFY_PIN_DIRECT_APP_ID": FEATURE_MODIFY_PIN_DIRECT_APP_ID,
    "FEATURE_WRITE_DISPLAY": FEATURE_WRITE_DISPLAY,
    "FEATURE_GET_KEY": FEATURE_GET_KEY,
    "FEATURE_IFD_DISPLAY_PROPERTIES": FEATURE_IFD_DISPLAY_PROPERTIES,
    "FEATURE_GET_TLV_PROPERTIES": FEATURE_GET_TLV_PROPERTIES,
    "FEATURE_CCID_ESC_COMMAND": FEATURE_CCID_ESC_COMMAND,
}

# properties returned by FEATURE_GET_TLV_PROPERTIES
PCSCv2_PART10_PROPERTY_wLcdLayout = 1
PCSCv2_PART10_PROPERTY_bEntryValidationCondition = 2
PCSCv2_PART10_PROPERTY_bTimeOut2 = 3
PCSCv2_PART10_PROPERTY_wLcdMaxCharacters = 4
PCSCv2_PART10_PROPERTY_wLcdMaxLines = 5
PCSCv2_PART10_PROPERTY_bMinPINSize = 6
PCSCv2_PART10_PROPERTY_bMaxPINSize = 7
PCSCv2_PART10_PROPERTY_sFirmwareID = 8
PCSCv2_PART10_PROPERTY_bPPDUSupport = 9
PCSCv2_PART10_PROPERTY_dwMaxAPDUDataSize = 10
PCSCv2_PART10_PROPERTY_wIdVendor = 11
PCSCv2_PART10_PROPERTY_wIdProduct = 12

Properties = {
    "PCSCv2_PART10_PROPERTY_wLcdLayout": PCSCv2_PART10_PROPERTY_wLcdLayout,
    "PCSCv2_PART10_PROPERTY_bEntryValidationCondition": PCSCv2_PART10_PROPERTY_bEntryValidationCondition,  # pylint: disable=line-too-long
    "PCSCv2_PART10_PROPERTY_bTimeOut2": PCSCv2_PART10_PROPERTY_bTimeOut2,
    "PCSCv2_PART10_PROPERTY_wLcdMaxCharacters": PCSCv2_PART10_PROPERTY_wLcdMaxCharacters,
    "PCSCv2_PART10_PROPERTY_wLcdMaxLines": PCSCv2_PART10_PROPERTY_wLcdMaxLines,
    "PCSCv2_PART10_PROPERTY_bMinPINSize": PCSCv2_PART10_PROPERTY_bMinPINSize,
    "PCSCv2_PART10_PROPERTY_bMaxPINSize": PCSCv2_PART10_PROPERTY_bMaxPINSize,
    "PCSCv2_PART10_PROPERTY_sFirmwareID": PCSCv2_PART10_PROPERTY_sFirmwareID,
    "PCSCv2_PART10_PROPERTY_bPPDUSupport": PCSCv2_PART10_PROPERTY_bPPDUSupport,
    "PCSCv2_PART10_PROPERTY_dwMaxAPDUDataSize": PCSCv2_PART10_PROPERTY_dwMaxAPDUDataSize,
    "PCSCv2_PART10_PROPERTY_wIdVendor": PCSCv2_PART10_PROPERTY_wIdVendor,
    "PCSCv2_PART10_PROPERTY_wIdProduct": PCSCv2_PART10_PROPERTY_wIdProduct,
}

# we already have:       Features['FEATURE_x'] = FEATURE_x
# we will now also have: Features[FEATURE_x] = 'FEATURE_x'
for k in list(Features.keys()):
    Features[Features[k]] = k

for k in list(Properties.keys()):
    Properties[Properties[k]] = k


def parseFeatureRequest(response):
    """Get the list of Part10 features supported by the reader.

    @param response: result of L{CM_IOCTL_GET_FEATURE_REQUEST} command

    @rtype: list
    @return: a list of list C{[[tag1, value1], [tag2, value2]]}
    """
    features = []
    while len(response) > 0:
        tag = response[0]
        control = (
            ((((response[2] << 8) + response[3]) << 8) + response[4]) << 8
        ) + response[5]
        try:
            features.append([Features[tag], control])
        except KeyError:
            pass
        del response[:6]
    return features


def getFeatureRequest(cardConnection):
    """Get the list of Part10 features supported by the reader.

    @param cardConnection: L{CardConnection} object

    @rtype: list
    @return: a list of list C{[[tag1, value1], [tag2, value2]]}
    """
    response = cardConnection.control(CM_IOCTL_GET_FEATURE_REQUEST, [])
    return parseFeatureRequest(response)


def hasFeature(featureList, feature):
    """return the controlCode for a feature or None

    @param feature:     feature to look for
    @param featureList: feature list as returned by L{getFeatureRequest()}

    @return: feature value or None
    """
    for f in featureList:
        if feature in (f[0], Features[f[0]]):
            return f[1]

    return None


def getPinProperties(cardConnection, featureList=None, controlCode=None):
    """return the C{PIN_PROPERTIES} structure

    @param cardConnection: L{CardConnection} object
    @param featureList: feature list as returned by L{getFeatureRequest()}
    @param controlCode: control code for L{FEATURE_IFD_PIN_PROPERTIES}

    @rtype: dict
    @return: a dict"""
    if controlCode is None:
        if featureList is None:
            featureList = getFeatureRequest(cardConnection)
        controlCode = hasFeature(featureList, FEATURE_IFD_PIN_PROPERTIES)

    if controlCode is None:
        return {"raw": []}

    response = cardConnection.control(controlCode, [])
    d = {
        "raw": response,
        "LcdLayoutX": response[0],
        "LcdLayoutY": response[1],
        "EntryValidationCondition": response[2],
        "TimeOut2": response[3],
    }

    return d


def getTlvProperties(cardConnection, featureList=None, controlCode=None):
    """return the C{GET_TLV_PROPERTIES} structure

    @param cardConnection: L{CardConnection} object
    @param featureList: feature list as returned by L{getFeatureRequest()}
    @param controlCode: control code for L{FEATURE_GET_TLV_PROPERTIES}

    @rtype: dict
    @return: a dict"""
    if controlCode is None:
        if featureList is None:
            featureList = getFeatureRequest(cardConnection)
        controlCode = hasFeature(featureList, FEATURE_GET_TLV_PROPERTIES)

    if controlCode is None:
        return {"raw": []}

    response = cardConnection.control(controlCode, [])
    return parseTlvProperties(response)


def parseTlvProperties(response):
    """return the GET_TLV_PROPERTIES structure

    @param response: result of  L{FEATURE_GET_TLV_PROPERTIES}

    @rtype: dict
    @return: a dict"""
    d = {
        "raw": response,
    }

    # create a new list to consume it
    tmp = list(response)
    while tmp:
        tag = tmp[0]
        length = tmp[1]
        data = tmp[2 : 2 + length]

        if PCSCv2_PART10_PROPERTY_sFirmwareID == tag:
            # convert to a string
            data = "".join([chr(c) for c in data])
        # we now suppose the value is an integer
        elif 1 == length:
            # byte
            data = data[0]
        elif 2 == length:
            # 16 bits value
            data = data[1] * 256 + data[0]
        elif 4 == length:
            # 32 bits value
            data = ((data[3] * 256 + data[2]) * 256 + data[1]) * 256 + data[0]

        # store the value in the dictionary
        try:
            d[Properties[tag]] = data
        except KeyError:
            d["UNKNOWN"] = data

        del tmp[0 : 2 + length]

    return d


if __name__ == "__main__":
    # Small sample illustrating the use of PCSCPart10.
    from smartcard.pcsc.PCSCReader import PCSCReader

    cc = PCSCReader.readers()[0].createConnection()
    cc.connect(mode=SCARD_SHARE_DIRECT)

    # print(cc.control(CM_IOCTL_GET_FEATURE_REQUEST))
    _features = getFeatureRequest(cc)
    print(_features)

    print(hasFeature(_features, FEATURE_VERIFY_PIN_START))
    print(hasFeature(_features, FEATURE_VERIFY_PIN_DIRECT))

    properties = getPinProperties(cc)
    print("\nPinProperties:")
    for k, v in list(properties.items()):
        print(f" {k}: {v}")

    print("\nTlvProperties:")
    properties = getTlvProperties(cc)
    for k, v in list(properties.items()):
        print(f" {k}: {v}")
