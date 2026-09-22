/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "CoreS3Chime.h"

#include "bsp/m5stack_core_s3.h"
#include "driver/i2c_master.h"
#include "driver/i2s_std.h"
#include "sdkconfig.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstring>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <iterator>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <utility>

// esp_err_t counterpart of ReturnErrorOnFailure. The helpers used here log the failing
// register themselves, so the macro only carries the error out.
#define EspReturnErrorOnFailure(expr)                                                                                              \
    do                                                                                                                             \
    {                                                                                                                              \
        esp_err_t __err = (expr);                                                                                                  \
        if (__err != ESP_OK)                                                                                                       \
        {                                                                                                                          \
            return __err;                                                                                                          \
        }                                                                                                                          \
    } while (false)

namespace chip::app {

namespace {

// Audio output on the M5Stack CoreS3
// ==================================
//
// Path: ESP32-S3 I2S -> AW88298 class-D amplifier -> onboard 1 W speaker.
//
//   BCLK  GPIO34   shared with ES7210 capture codec
//   LRCK  GPIO33   shared with ES7210 capture codec
//   DOUT  GPIO13   amplifier only
//   MCLK  GPIO0    used by ES7210 only, unused here
//
// I2C, internal bus: AW88298 0x36, AW9523 IO expander 0x58, AXP2101 PMU 0x34.
// AW9523 P0_2 enables the amplifier, P1_7 enables the SY7088 step-up converter.
//
// Required, each verified by reverting it alone on hardware:
//
//  1. AXP2101 register 0x90 with ALDO3, BLDO1, BLDO2 enabled (0xbf). ESP-BSP sets 0x8b.
//     Without these rails the amplifier acknowledges I2C and locks its PLL, but latches
//     under-voltage lockout and does not start its output stage. Set bits only: clearing
//     a rail used by display, touch or storage resets those peripherals.
//
//  2. 48 kHz output rate. At 22050 Hz the PLL locks but the output stage does not start.
//
// Checked while debugging silent output and found to not affect audio: I2S peripheral
// number (NUM_0 and NUM_1 both work) and MCLK output (works driven or undriven, left
// undriven).
//
// SYSST, register 0x01:
//   bit 0  PLLS   PLL locked to bit clock
//   bit 4  CLKS   clock detected
//   bit 9  BSTS   output stage running
//   bit 14 UVLS   under-voltage lockout
// PLLS and CLKS only indicate the digital input is clocked; sound requires BSTS. A
// working reference reads 0x0211 during playback.
//
// ESP-BSP audio helpers are unused: bsp_audio_init() also allocates and enables an RX
// channel on the same pins, and the esp_codec_dev AW88298 driver writes an I2SCTRL value
// for 16-bit audio that contradicts its own frame setting.

// Required by the amplifier, see above.
constexpr uint32_t kSampleRateHz = 48000;
constexpr float kPi              = 3.14159265358979323846f;
constexpr size_t kChunkSamples   = 256;

// Sine lookup table sized so that the top bits of a 32-bit phase accumulator index it
// directly. 256 entries keep the table at 1 KB while staying well below the noise floor
// of a chime.
constexpr size_t kSineTableSize    = 256;
constexpr uint32_t kSineTableShift = 24;

constexpr uint8_t kAw88298Address = 0x36;
constexpr uint8_t kAw9523Address  = 0x58;
constexpr uint8_t kAxp2101Address = 0x34;

constexpr uint16_t kAw88298ChipId = 0x1852;

// AW9523 output ports; P0_2 is the AW88298 PA enable and P1_7 enables the SY7088 step-up
// converter that feeds the amplifier power stage.
constexpr uint8_t kAw9523OutputPort0 = 0x02;
constexpr uint8_t kAw9523OutputPort1 = 0x03;
constexpr uint8_t kAw88298PaEnable   = 0x04;
constexpr uint8_t kSy7088BoostEnable = 0x80;

// AXP2101 LDO enable register. ALDO3, BLDO1 and BLDO2 are left off by the BSP but are
// enabled by the M5Unified board setup that the factory firmware uses.
constexpr uint8_t kAxp2101LdoEnable         = 0x90;
constexpr uint8_t kAxp2101AmplifierRailBits = 0x34;

// Synthesis parameters for one chime. A chime with both frequencies equal is a single
// note; otherwise the second note starts halfway through.
struct ChimeTone
{
    float firstFrequencyHz;
    float secondFrequencyHz;
    float durationSec;
    bool pulse;
};

// Chime ID is the index into both tables; keep them in sync.
const Chime::Sound kCoreS3Sounds[] = {
    { 0, "Ding Dong"_span },
    { 1, "Ring Ring"_span },
    { 2, "Alert Beep"_span },
};

constexpr ChimeTone kCoreS3Tones[] = {
    { 880.0f, 660.0f, 1.0f, false },  // Ding Dong: two notes
    { 1000.0f, 1000.0f, 1.0f, true }, // Ring Ring: pulsed single note
    { 440.0f, 440.0f, 0.5f, false },  // Alert Beep: short single note
};

static_assert(std::size(kCoreS3Tones) == std::size(kCoreS3Sounds));

// A device attached to the internal I2C bus, detached again when it goes out of scope.
// Register accesses go through this object so that bring-up code can return early without
// leaking the handle.
class ScopedI2cDevice
{
public:
    ScopedI2cDevice() = default;

    // Every chime peripheral sits on the internal bus at 400 kHz with a 7-bit address, so
    // the address is the only thing callers choose. Invalid if the bus rejects the device.
    explicit ScopedI2cDevice(uint8_t address)
    {
        const i2c_device_config_t config = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address  = address,
            .scl_speed_hz    = 400000,
            .scl_wait_us     = 0,
            .flags           = { .disable_ack_check = 0 },
        };

        esp_err_t err = i2c_master_bus_add_device(bsp_i2c_get_handle(), &config, &mDevice);
        if (err != ESP_OK)
        {
            ChipLogError(DeviceLayer, "CoreS3Chime: Failed to attach 0x%02x to the I2C bus: %s", address, esp_err_to_name(err));
            mDevice = nullptr;
        }
    }

    ScopedI2cDevice(ScopedI2cDevice && other) : mDevice(other.mDevice) { other.mDevice = nullptr; }
    ScopedI2cDevice & operator=(ScopedI2cDevice && other)
    {
        if (this != &other)
        {
            Reset();
            mDevice       = other.mDevice;
            other.mDevice = nullptr;
        }
        return *this;
    }

    ScopedI2cDevice(const ScopedI2cDevice &)             = delete;
    ScopedI2cDevice & operator=(const ScopedI2cDevice &) = delete;

    ~ScopedI2cDevice() { Reset(); }

    bool IsValid() const { return mDevice != nullptr; }

    void Reset()
    {
        if (mDevice != nullptr)
        {
            i2c_master_bus_rm_device(mDevice);
            mDevice = nullptr;
        }
    }

    esp_err_t ReadRegister(uint8_t reg, uint8_t & value) const
    {
        return i2c_master_transmit_receive(mDevice, &reg, 1, &value, 1, 100);
    }

    esp_err_t WriteRegister(uint8_t reg, uint8_t value) const
    {
        const uint8_t payload[2] = { reg, value };
        return i2c_master_transmit(mDevice, payload, sizeof(payload), 100);
    }

    // The amplifier registers are 16 bits wide, most significant byte first.
    esp_err_t ReadWordRegister(uint8_t reg, uint16_t & value) const
    {
        uint8_t raw[2] = { 0, 0 };
        esp_err_t err  = i2c_master_transmit_receive(mDevice, &reg, 1, raw, sizeof(raw), 100);
        value          = static_cast<uint16_t>((static_cast<uint16_t>(raw[0]) << 8) | raw[1]);
        return err;
    }

    esp_err_t WriteWordRegister(uint8_t reg, uint16_t value) const
    {
        const uint8_t payload[3] = { reg, static_cast<uint8_t>(value >> 8), static_cast<uint8_t>(value & 0xFF) };
        return i2c_master_transmit(mDevice, payload, sizeof(payload), 100);
    }

    // Read-modify-write of a single register, leaving the bits the caller did not name
    // alone. The register is named in the log: the caller only sees an esp_err_t.
    esp_err_t SetRegisterBits(uint8_t reg, uint8_t bits) const
    {
        uint8_t value = 0;
        esp_err_t err = ReadRegister(reg, value);
        if (err == ESP_OK && (value & bits) != bits)
        {
            err = WriteRegister(reg, static_cast<uint8_t>(value | bits));
        }

        if (err != ESP_OK)
        {
            ChipLogError(DeviceLayer, "CoreS3Chime: Failed to set 0x%02x bits in register 0x%02x: %s", bits, reg,
                         esp_err_to_name(err));
        }
        return err;
    }

    esp_err_t ClearRegisterBits(uint8_t reg, uint8_t bits) const
    {
        uint8_t value = 0;
        esp_err_t err = ReadRegister(reg, value);
        if (err == ESP_OK && (value & bits) != 0)
        {
            err = WriteRegister(reg, static_cast<uint8_t>(value & ~bits));
        }

        if (err != ESP_OK)
        {
            ChipLogError(DeviceLayer, "CoreS3Chime: Failed to clear 0x%02x bits in register 0x%02x: %s", bits, reg,
                         esp_err_to_name(err));
        }
        return err;
    }

private:
    i2c_master_dev_handle_t mDevice = nullptr;
};

esp_err_t ConfigureExpanderOutputs()
{
    ScopedI2cDevice expander(kAw9523Address);
    VerifyOrReturnError(expander.IsValid(), ESP_FAIL);

    // P1_7 must be a GPIO (LED-mode bit set) configured as an output (direction bit clear)
    // before the output latch can drive the SY7088 enable line high.
    EspReturnErrorOnFailure(expander.SetRegisterBits(0x13, kSy7088BoostEnable));
    EspReturnErrorOnFailure(expander.ClearRegisterBits(0x05, kSy7088BoostEnable));
    EspReturnErrorOnFailure(expander.SetRegisterBits(kAw9523OutputPort1, kSy7088BoostEnable));

    // P0_2 is the AW88298 PA enable. It is driven here rather than through
    // bsp_feature_enable(BSP_FEATURE_SPEAKER), which would also power the ES7210.
    EspReturnErrorOnFailure(expander.SetRegisterBits(0x12, kAw88298PaEnable));
    EspReturnErrorOnFailure(expander.ClearRegisterBits(0x04, kAw88298PaEnable));
    EspReturnErrorOnFailure(expander.SetRegisterBits(kAw9523OutputPort0, kAw88298PaEnable));

    return ESP_OK;
}

// The BSP brings up only a subset of the AXP2101 rails (0x8b), while the M5Unified board
// setup used by the factory firmware enables 0xbf. With ALDO3 / BLDO1 / BLDO2 off the
// AW88298 reports under-voltage lockout and its boost stage never starts, so the amplifier
// clocks correctly but produces no output. Only set bits here: clearing a rail that the
// display, touch or storage drivers depend on browns those peripherals out.
esp_err_t EnableAmplifierSupplyRails()
{
    ScopedI2cDevice pmu(kAxp2101Address);
    VerifyOrReturnError(pmu.IsValid(), ESP_FAIL);

    return pmu.SetRegisterBits(kAxp2101LdoEnable, kAxp2101AmplifierRailBits);
}

// The AW88298 selects its sample rate from a fixed table, indexed by the low nibble of
// register 0x06. Table entries are the rate in units of 2205 Hz, so 48000 Hz maps to
// entry 22 at index 8.
uint16_t AmplifierRateIndex()
{
    constexpr uint8_t kRateTable[] = { 4, 5, 6, 8, 10, 11, 15, 20, 22, 44 };
    const uint32_t scaledRate      = (kSampleRateHz + 1102) / 2205;

    uint16_t index = 0;
    while (index < (sizeof(kRateTable) - 1) && scaledRate > kRateTable[index])
    {
        ++index;
    }
    return index;
}

// Register 0x0C holds the output attenuation in its high byte, 0x00 for 0 dB down to 0xC0
// for -96 dB in 0.5 dB steps. The low byte is a fixed configuration field.
uint16_t AmplifierVolumeRegister()
{
    constexpr uint16_t kMaxAttenuationSteps = 0xC0;
    constexpr uint16_t kVolumeLowByte       = 0x64;

    uint16_t steps = static_cast<uint16_t>(CONFIG_CHIME_ATTENUATION_DB * 2);
    if (steps > kMaxAttenuationSteps)
    {
        steps = kMaxAttenuationSteps;
    }
    return static_cast<uint16_t>((steps << 8) | kVolumeLowByte);
}

// Returns an invalid device if the amplifier does not come up: the caller decides when
// the configured amplifier becomes visible to the rest of the file.
ScopedI2cDevice InitializeAmplifier()
{
    ScopedI2cDevice amplifier(kAw88298Address);
    VerifyOrReturnValue(amplifier.IsValid(), ScopedI2cDevice());

    // A soft reset clears any latched protection state (for example UVLS from a previous
    // power-up attempt) that would otherwise keep the output stage disabled.
    esp_err_t err = amplifier.WriteWordRegister(0x00, 0x55AA);
    VerifyOrReturnValue(err == ESP_OK, ScopedI2cDevice(),
                        ChipLogError(DeviceLayer, "CoreS3Chime: Failed to reset the amplifier: %s", esp_err_to_name(err)));
    vTaskDelay(pdMS_TO_TICKS(10));

    // Register values follow the M5Unified CoreS3 bring-up sequence, which programs the
    // amplifier before any I2S clock is present. Register 0x06 encodes channel select
    // (left), Philips I2S, 16-bit words and 32 BCK per frame in the upper bits, with the
    // sample rate selected by the low nibble.
    const std::pair<uint8_t, uint16_t> kSetupRegisters[] = {
        { 0x61, 0x0673 },                                               // Boost mode disabled
        { 0x04, 0x4040 },                                               // I2SEN=1 AMPPD=0 PWDN=0
        { 0x05, 0x0008 },                                               // RMSE=0 HAGCE=0 HDCCE=0 HMUTE=0
        { 0x06, static_cast<uint16_t>(0x14C0 | AmplifierRateIndex()) }, // Format and sample rate
        { 0x0C, AmplifierVolumeRegister() },                            // Volume
    };

    for (const auto & [reg, value] : kSetupRegisters)
    {
        err = amplifier.WriteWordRegister(reg, value);
        VerifyOrReturnValue(
            err == ESP_OK, ScopedI2cDevice(),
            ChipLogError(DeviceLayer, "CoreS3Chime: Failed to write amplifier register 0x%02x: %s", reg, esp_err_to_name(err)));
    }

    uint16_t chipId = 0;
    err             = amplifier.ReadWordRegister(0x00, chipId);
    VerifyOrReturnValue(err == ESP_OK, ScopedI2cDevice(),
                        ChipLogError(DeviceLayer, "CoreS3Chime: Failed to read the amplifier id: %s", esp_err_to_name(err)));
    VerifyOrReturnValue(
        chipId == kAw88298ChipId, ScopedI2cDevice(),
        ChipLogError(DeviceLayer, "CoreS3Chime: Unexpected amplifier id 0x%04x, expected 0x%04x", chipId, kAw88298ChipId));

    return amplifier;
}

// SYSST bit 0 reports PLL lock. The amplifier only starts switching once it has locked
// onto the incoming bit clock, so retry the enable until it reports a locked PLL.
bool WaitForAmplifierLock(const ScopedI2cDevice & amplifier)
{
    constexpr uint16_t kPllLocked = 0x0001;
    constexpr int kMaxAttempts    = 10;

    uint16_t status = 0;
    for (int attempt = 0; attempt < kMaxAttempts; ++attempt)
    {
        vTaskDelay(std::max<TickType_t>(1, pdMS_TO_TICKS(2)));
        if (amplifier.ReadWordRegister(0x01, status) == ESP_OK && (status & kPllLocked) != 0)
        {
            return true;
        }
        amplifier.WriteWordRegister(0x04, 0x4040);
    }

    ChipLogError(DeviceLayer, "CoreS3Chime: Amplifier PLL did not lock, sysst=0x%04x", status);
    return false;
}

// Phase is a 32-bit fraction of a full cycle, so one sample advances by
// frequency / sampleRate of 2^32.
uint32_t FrequencyToPhaseIncrement(float frequency)
{
    constexpr float kPhaseRange = 4294967296.0f;
    return static_cast<uint32_t>((frequency * kPhaseRange) / static_cast<float>(kSampleRateHz));
}

// Owns the output path: the I2S channel, the oscillator table, the buffer staged for the
// I2S driver and the claim that keeps two chimes from overlapping. Hardware bring-up is
// deferred to the first chime, so a device that never chimes never powers the amplifier.
class ChimePlayer
{
public:
    static ChimePlayer & Instance()
    {
        static ChimePlayer player;
        return player;
    }

    bool EnsureInitialized()
    {
        if (mTxChannel != nullptr)
        {
            return true;
        }

        bsp_i2c_init();

        esp_err_t err = EnableAmplifierSupplyRails();
        if (err != ESP_OK)
        {
            ChipLogError(DeviceLayer, "CoreS3Chime: Failed to enable the amplifier supply rails: %s", esp_err_to_name(err));
            return false;
        }
        vTaskDelay(pdMS_TO_TICKS(10));

        err = ConfigureExpanderOutputs();
        if (err != ESP_OK)
        {
            ChipLogError(DeviceLayer, "CoreS3Chime: Failed to configure the expander outputs: %s", esp_err_to_name(err));
            return false;
        }
        vTaskDelay(pdMS_TO_TICKS(20));

        // M5Unified programs the amplifier while the I2S pins are still idle and only then
        // starts the clock; configuring register 0x06 (PLL divider) against a running clock
        // leaves the AW88298 state machine oscillating between locked and unlocked.
        ScopedI2cDevice amplifier = InitializeAmplifier();
        VerifyOrReturnValue(amplifier.IsValid(), false);

        VerifyOrReturnValue(InitializeI2sTxChannel(), false);

        // Nothing reads the amplifier registers after this point, so the device is detached
        // when this function returns. Detaching the I2C device leaves the amplifier
        // configuration in place.
        if (!WaitForAmplifierLock(amplifier))
        {
            i2s_channel_disable(mTxChannel);
            i2s_del_channel(mTxChannel);
            mTxChannel = nullptr;
            return false;
        }
        return true;
    }

    // The output path has a single I2S channel and one staging buffer, so a second chime
    // cannot run alongside the first. Claimed on the CHIP thread, released by the playback
    // task when it finishes.
    bool ClaimPlayback()
    {
        bool expected = false;
        return mPlaybackActive.compare_exchange_strong(expected, true, std::memory_order_acq_rel);
    }

    void ReleasePlayback() { mPlaybackActive.store(false, std::memory_order_release); }

    void Play(uint8_t chimeID)
    {
        if (chimeID >= std::size(kCoreS3Tones))
        {
            return;
        }
        const ChimeTone & tone = kCoreS3Tones[chimeID];

        EnsureSineTable();

        const uint32_t totalSamples = static_cast<uint32_t>(tone.durationSec * kSampleRateHz);

        // A two-tone chime switches to the second note halfway through; a single-tone chime
        // never reaches the boundary.
        const uint32_t noteBoundary       = (tone.firstFrequencyHz != tone.secondFrequencyHz) ? (totalSamples / 2) : totalSamples;
        const uint32_t pulsePeriodSamples = kSampleRateHz / 20;

        // exp(-4t) sampled at the output rate is a constant ratio between consecutive
        // samples, so the envelope costs one multiply per sample rather than a call to expf.
        const float envelopeDecay = std::exp(-4.0f / static_cast<float>(kSampleRateHz));

        uint32_t phase          = 0;
        uint32_t phaseIncrement = FrequencyToPhaseIncrement(tone.firstFrequencyHz);
        float envelope          = 1.0f;
        uint32_t produced       = 0;

        while (produced < totalSamples)
        {
            uint32_t count = totalSamples - produced;
            if (count > kChunkSamples)
            {
                count = kChunkSamples;
            }

            for (uint32_t i = 0; i < count; ++i)
            {
                const uint32_t index = produced + i;
                if (index == noteBoundary)
                {
                    phaseIncrement = FrequencyToPhaseIncrement(tone.secondFrequencyHz);
                    phase          = 0;
                    envelope       = 1.0f;
                }

                // Unsigned overflow wraps the phase, so multiplying it yields the harmonics.
                float sample = 0.6f * SineAt(phase) + 0.3f * SineAt(phase * 2) + 0.1f * SineAt(phase * 3);
                sample *= envelope;

                if (tone.pulse && (((index / pulsePeriodSamples) & 1) != 0))
                {
                    sample = 0.0f;
                }

                const int16_t value     = static_cast<int16_t>(sample * 28000.0f);
                mChunkBuffer[2 * i]     = value;
                mChunkBuffer[2 * i + 1] = value;

                phase += phaseIncrement;
                envelope *= envelopeDecay;
            }

            const size_t chunkBytes = count * 2 * sizeof(int16_t);
            size_t written          = 0;
            esp_err_t err           = i2s_channel_write(mTxChannel, mChunkBuffer, chunkBytes, &written, 1000);
            if (err != ESP_OK || written != chunkBytes)
            {
                ChipLogError(DeviceLayer, "CoreS3Chime: I2S write failed (%s, %u/%u bytes), aborting tone", esp_err_to_name(err),
                             static_cast<unsigned>(written), static_cast<unsigned>(chunkBytes));
                return;
            }
            produced += count;
        }

        // Flush a silent tail so the amplifier does not hold the last sample.
        std::memset(mChunkBuffer, 0, sizeof(mChunkBuffer));
        size_t written = 0;
        esp_err_t err  = i2s_channel_write(mTxChannel, mChunkBuffer, sizeof(mChunkBuffer), &written, 1000);
        if (err != ESP_OK)
        {
            ChipLogError(DeviceLayer, "CoreS3Chime: I2S silence write failed: %s", esp_err_to_name(err));
        }
    }

private:
    ChimePlayer() = default;

    bool InitializeI2sTxChannel()
    {
        i2s_chan_config_t channelConfig = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
        channelConfig.auto_clear        = true;

        if (i2s_new_channel(&channelConfig, &mTxChannel, nullptr) != ESP_OK)
        {
            ChipLogError(DeviceLayer, "CoreS3Chime: Failed to allocate I2S TX channel");
            return false;
        }

        const i2s_std_config_t standardConfig = {
            .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(kSampleRateHz),
            .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
            .gpio_cfg = {
                // Verified on hardware to work either way; left undriven so GPIO0 stays free,
                // matching M5Unified. Only the (unused here) ES7210 capture codec needs MCLK.
                .mclk = I2S_GPIO_UNUSED,
                .bclk = BSP_I2S_SCLK,
                .ws   = BSP_I2S_LCLK,
                .dout = BSP_I2S_DOUT,
                .din  = I2S_GPIO_UNUSED,
                .invert_flags = {
                    .mclk_inv = false,
                    .bclk_inv = false,
                    .ws_inv   = false,
                },
            },
        };

        if (i2s_channel_init_std_mode(mTxChannel, &standardConfig) != ESP_OK || i2s_channel_enable(mTxChannel) != ESP_OK)
        {
            ChipLogError(DeviceLayer, "CoreS3Chime: Failed to configure I2S TX channel");
            i2s_del_channel(mTxChannel);
            mTxChannel = nullptr;
            return false;
        }

        return true;
    }

    void EnsureSineTable()
    {
        if (mSineTableInitialized)
        {
            return;
        }

        for (size_t i = 0; i < kSineTableSize; ++i)
        {
            mSineTable[i] = std::sin((2.0f * kPi * static_cast<float>(i)) / static_cast<float>(kSineTableSize));
        }
        mSineTableInitialized = true;
    }

    float SineAt(uint32_t phase) const { return mSineTable[phase >> kSineTableShift]; }

    i2s_chan_handle_t mTxChannel = nullptr;
    std::atomic<bool> mPlaybackActive{ false };

    // Written only by the playback task. Members rather than locals because that task runs
    // on a small stack.
    bool mSineTableInitialized = false;
    float mSineTable[kSineTableSize];
    int16_t mChunkBuffer[kChunkSamples * 2];
};

void ChimePlaybackTask(void * arg)
{
    ChimePlayer::Instance().Play(static_cast<uint8_t>(reinterpret_cast<uintptr_t>(arg)));
    ChimePlayer::Instance().ReleasePlayback();
    vTaskDelete(nullptr);
}

} // namespace

CoreS3Chime::CoreS3Chime(TimerDelegate & timerDelegate) : Chime(timerDelegate, Span<const Sound>(kCoreS3Sounds)) {}

Protocols::InteractionModel::Status CoreS3Chime::PlayChimeSound(uint8_t chimeID)
{
    auto status = Chime::PlayChimeSound(chimeID);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        return status;
    }

    ChimePlayer & player = ChimePlayer::Instance();
    if (!player.EnsureInitialized())
    {
        return Protocols::InteractionModel::Status::Failure;
    }

    // Refuse an overlapping chime rather than queueing a task that would sit idle for the
    // length of the current sound.
    //
    // This deviates from TC-CHIME-2.4 step 7, which sends three PlayChimeSound commands back to
    // back and expects Success for each, with overlapping audio simply dropped. Restarting or
    // queueing playback is deliberately not implemented: the overlap window is short and a single
    // I2S channel cannot mix two tones.
    if (!player.ClaimPlayback())
    {
        return Protocols::InteractionModel::Status::Busy;
    }

    // Priority 2 sits above the CHIP event loop (priority 1) so playback is not delayed by
    // Matter processing, but below the display task so it cannot stall the UI.
    if (xTaskCreate(ChimePlaybackTask, "chime_play", 4096, reinterpret_cast<void *>(static_cast<uintptr_t>(chimeID)), 2, nullptr) !=
        pdPASS)
    {
        ChipLogError(DeviceLayer, "CoreS3Chime: Failed to start the playback task");
        player.ReleasePlayback();
        return Protocols::InteractionModel::Status::Failure;
    }
    return Protocols::InteractionModel::Status::Success;
}

} // namespace chip::app
