#include "chip-zcl.h"

// Callback implementations

ChipZclStatus_t chipZclExternalAttributeReadCallback(uint8_t endpoint, ChipZclClusterId clusterId,
                                                     ChipZclAttributeMetadata * attributeMetadata, uint16_t manufacturerCode,
                                                     uint8_t * buffer, uint16_t maxReadLength)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}

ChipZclStatus_t chipZclExternalAttributeWriteCallback(uint8_t endpoint, ChipZclClusterId clusterId,
                                                      ChipZclAttributeMetadata * attributeMetadata, uint16_t manufacturerCode,
                                                      uint8_t * buffer)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}
ChipZclStatus_t chipZclPreAttributeChangeCallback(uint8_t endpoint, ChipZclClusterId clusterId, ChipZclAttributeId attributeId,
                                                  uint8_t mask, uint16_t manufacturerCode, uint8_t type, uint8_t size,
                                                  uint8_t * value)
{
    return CHIP_ZCL_STATUS_SUCCESS;
}

void chipZclPostAttributeChangeCallback(uint8_t endpoint, ChipZclClusterId clusterId, ChipZclAttributeId attributeId, uint8_t mask,
                                        uint16_t manufacturerCode, uint8_t type, uint8_t size, uint8_t * value)
{}

bool chipZclAttributeReadAccessCallback(uint8_t endpoint, ChipZclClusterId clusterId, uint16_t manufacturerCode,
                                        uint16_t attributeId)
{
    return true;
}

bool chipZclAttributeWriteAccessCallback(uint8_t endpoint, ChipZclClusterId clusterId, uint16_t manufacturerCode,
                                         uint16_t attributeId)
{
    return true;
}
