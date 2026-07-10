#pragma once
/**
 * @file STM32F723Disco.h
 * @brief Device definition for STM32F723E Discovery board
 */

#include "AudioBoard.h"

namespace audio_driver {

/**
 * @brief Pins the STM32F723E-Discovery board using the WM8994 codec
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
class PinsSTM32F723DiscoClass : public DriverDeviceInfo {
 public:
  PinsSTM32F723DiscoClass() {
    // add i2c codec pins: scl, sda, port, frequency
    // ST's own reference driver runs this bus in Fast Mode (400kHz, their
    // DISCOVERY_I2Cx_TIMING = 0x40912732) and is reliably clean across cold
    // power-on, unlike our previous 100kHz Standard Mode default - testing
    // whether the STM32duino-computed Standard Mode timing was marginal.
    addI2C(PinFunction::CODEC, PB8, PB9, -1, 400000);
    // add i2s pins: mclk, bck, ws, data_out, data_in (SAI2, block A = out, block B = in)
    addI2S(PinFunction::CODEC, PI4, PI5, PI7, PI6, PG10);

    // add other pins
    addPin(PinFunction::KEY, PA0, PinLogic::InputActiveHigh);  // user button (also this MCU's WKUP pin - must stay an input)
    addPin(PinFunction::LED, PA7, PinLogic::Output, 0);  // red
    addPin(PinFunction::LED, PB1, PinLogic::Output, 1);  // green
  }
};

/// @ingroup audio_driver
static PinsSTM32F723DiscoClass PinsSTM32F723Disco;

/// @ingroup audio_driver
static AudioBoard STM32F723Disco{AudioDriverWM8994, PinsSTM32F723Disco};

}  // namespace audio_driver
