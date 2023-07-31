#pragma once

#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

#include <peripheral_io.h>

namespace example {
class LedManager
{
public:
    explicit LedManager(chip::EndpointId endpointId);
    void SetOnOff(bool on);
    CHIP_ERROR Init();

private:
    void InitOnOff();

    // Numbers of GPIO used to control LED in order: RED, GREEN, BLUE
    static constexpr int number_of_pins       = 3;
    static constexpr int pins[number_of_pins] = { 20, 19, 18 };
    peripheral_gpio_h gpio[number_of_pins]    = {};

    const chip::EndpointId mEndpointId;

    static LedManager sLedMgr;
};

} // namespace example