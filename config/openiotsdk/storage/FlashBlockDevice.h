/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 */

#ifndef IOTSDK_FLASH_BLOCKDEVICE_H
#define IOTSDK_FLASH_BLOCKDEVICE_H

#include "iotsdk/BlockDevice.h"

#include <stdint.h>

namespace iotsdk {
namespace storage {
/** BlockDevice using the flash memory for TF-M application
 *
 */
class FlashBlockDevice final : public BlockDevice
{
public:
    /** Creates a FlashBlockDevice
     *  @param address  Physical address where the block device start
     *  @param size     The block device size
     */
    FlashBlockDevice(uint32_t address, uint32_t size);

    ~FlashBlockDevice();

    /** Initialize a block device
     *
     *  This method must be called before attempting any further block device operations.
     *
     *  @return         bd_status::OK on success or an error status on failure
     */
    bd_status init() override;

    /** Deinitialize a block device
     *
     *  @return         bd_status::OK on success or an error status on failure
     */
    bd_status deinit() override;

    /** Read blocks from a block device
     *
     *  If a failure occurs, it is not possible to determine how many bytes succeeded.
     *
     *  @param buffer   Buffer to write blocks to
     *  @param addr     Address of block to begin reading from
     *  @param size     Size to read in bytes, must be a multiple of the read block size
     *  @return         bd_status::OK on success or an error status on failure
     */
    bd_status read(void * buffer, bd_addr_t addr, bd_size_t size) override;

    /** Program blocks to a block device
     *
     *  The blocks must have been erased prior to being programmed.
     *
     *  If a failure occurs, it is not possible to determine how many bytes succeeded.
     *
     *  @param buffer   Buffer of data to write to blocks
     *  @param addr     Address of block to begin writing to
     *  @param size     Size to write in bytes, must be a multiple of the program block size
     *  @return         bd_status::OK on success or an error status on failure
     */
    bd_status program(const void * buffer, bd_addr_t addr, bd_size_t size) override;

    /** Erase blocks on a block device
     *
     *  The state of an erased block is undefined until it has been programmed,
     *  unless get_erase_value returns a non-negative byte value.
     *
     *  @param addr     Address of block to begin erasing
     *  @param size     Size to erase in bytes, must be a multiple of the erase block size
     *  @return         bd_status::OK on success or an error status on failure
     */
    bd_status erase(bd_addr_t addr, bd_size_t size) override;

    /** Get the size of a readable block
     *
     *  @return         Size of a readable block in bytes
     */
    bd_size_t get_read_size() const override;

    /** Get the size of a programmable block
     *
     *  @return         Size of a programmable block in bytes
     *  @note Must be a multiple of the read size
     */
    bd_size_t get_program_size() const override;

    /** Get the size of an erasable block for the whole device
     *
     *  @return         Size of an erasable block in bytes for the whole device
     *  @note Must be a multiple of the program size, or 0 if no common erase size exists
     *        across all regions or the underlying implementation does not provide this
     *        information in which case you need to call get_erase_size(bd_addr_t) instead
     *  @note For FlashBlockDevice, this always returns 0 because the underlying
     *        MCU-Driver-HAL flash API does not indicate whether all sectors have the same
     *        size
     */
    bd_size_t get_erase_size() const override;

    /** Get the size of an erasable block given address
     *
     *  @param addr     Address within the erasable block
     *  @return         Size of an erasable block in bytes
     *  @note Must be a multiple of the program size
     */
    bd_size_t get_erase_size(bd_addr_t addr) const override;

    /** Get the value of storage when erased
     *
     *  If get_erase_value returns a non-negative byte value, the underlying
     *  storage is set to that value when erased, and storage containing
     *  that value can be programmed without another erase.
     *
     *  @return         The value of storage when erased, or -1 if you can't
     *                  rely on the value of the erased storage
     */
    int get_erase_value() const override;

    /** Get the total size of the underlying device
     *
     *  @return         Size of the underlying device in bytes
     */
    bd_size_t size() const override;

    /** Convenience function for checking block read validity
     *
     *  @param addr     Address of block to begin reading from
     *  @param size     Size to read in bytes
     *  @return         bd_status::OK if read is valid for underlying block device
     */
    bd_status is_valid_read(bd_addr_t addr, bd_size_t size) const override;

    /** Convenience function for checking block program validity
     *
     *  @param addr     Address of block to begin writing to
     *  @param size     Size to write in bytes
     *  @return         bd_status::OK if program is valid for underlying block device
     */
    bd_status is_valid_program(bd_addr_t addr, bd_size_t size) const override;

    /** Convenience function for checking block erase validity
     *
     *  @param addr     Address of block to begin erasing
     *  @param size     Size to erase in bytes
     *  @return         bd_status::OK if erase is valid for underlying block device
     */
    bd_status is_valid_erase(bd_addr_t addr, bd_size_t size) const override;

    /** Get the BlockDevice class type.
     *
     *  @return         A string represent the BlockDevice class type.
     */
    const char * get_type() const override;

private:
    // Device configuration
    uint32_t _base;
    uint32_t _size;
    bool _is_initialized;
};

} // namespace storage
} // namespace iotsdk

#endif /* IOTSDK_FLASH_BLOCKDEVICE_H */
