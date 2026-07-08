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
    addI2C(PinFunction::CODEC, PB8, PB9);
    // add i2s pins: mclk, bck, ws, data_out, data_in (SAI2, block A = out, block B = in)
    addI2S(PinFunction::CODEC, PI4, PI5, PI7, PI6, PG10);

    // add other pins
    addPin(PinFunction::KEY, PA0, PinLogic::Output);     // user button
    addPin(PinFunction::LED, PA7, PinLogic::Output, 0);  // red
    addPin(PinFunction::LED, PB1, PinLogic::Output, 1);  // green
  }
};

/// @ingroup audio_driver
static PinsSTM32F723DiscoClass PinsSTM32F723Disco;

/// @ingroup audio_driver
static AudioBoard STM32F723Disco{AudioDriverWM8994, PinsSTM32F723Disco};

}  // namespace audio_driver
