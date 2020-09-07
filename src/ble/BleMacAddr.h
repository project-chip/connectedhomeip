#ifndef CHIP_BLE_ADDR_H_
#define CHIP_BLE_ADDR_H_

#include <array>
#include <cstdint>

namespace chip {
namespace Ble {

using MacAddr = std::array<uint8_t, 6>;

}
} // namespace chip

#endif // CHIP_BLE_ADDR_H_
