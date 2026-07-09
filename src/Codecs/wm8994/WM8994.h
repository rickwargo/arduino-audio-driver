/**
 ******************************************************************************
 * @file    wm8994.h
 * @author  MCD Application Team
 * @version V2.0.0
 * @date    24-June-2015
 * @brief   This file contains all the functions prototypes for the
 *          wm8994.c driver.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

#pragma once

/* Includes ------------------------------------------------------------------*/
// #include "../Common/audio.h"
#include <stddef.h>
#include <stdint.h>

#include "Codecs/CodecConstants.h"
#include "DriverCommon.h"
#include "Platforms/API_Delay.h"
#include "Platforms/API_I2C.h"
#include "stdbool.h"


namespace audio_driver {

/**
 * @brief Header-only driver class for the WM8994 audio codec chip
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
class WM8994 {
 public:
  enum AudioFrequency : uint32_t {
    AUDIO_FREQUENCY_192K = 192000,
    AUDIO_FREQUENCY_96K = 96000,
    AUDIO_FREQUENCY_48K = 48000,
    AUDIO_FREQUENCY_44K = 44100,
    AUDIO_FREQUENCY_32K = 32000,
    AUDIO_FREQUENCY_22K = 22050,
    AUDIO_FREQUENCY_16K = 16000,
    AUDIO_FREQUENCY_11K = 11025,
    AUDIO_FREQUENCY_8K = 8000
  };
  static constexpr uint16_t OUTPUT_DEVICE_SPEAKER = 0x0001;
  static constexpr uint16_t OUTPUT_DEVICE_HEADPHONE = 0x0002;
  static constexpr uint16_t OUTPUT_DEVICE_BOTH = 0x0003;
  static constexpr uint16_t OUTPUT_DEVICE_AUTO = 0x0004;
  static constexpr uint16_t INPUT_DEVICE_DIGITAL_MICROPHONE_1 = 0x0100;
  static constexpr uint16_t INPUT_DEVICE_DIGITAL_MICROPHONE_2 = 0x0200;
  static constexpr uint16_t INPUT_DEVICE_INPUT_LINE_1 = 0x0300;
  static constexpr uint16_t INPUT_DEVICE_INPUT_LINE_2 = 0x0400;
  static constexpr uint8_t DEFAULT_VOLMIN = 0x00;
  static constexpr uint8_t DEFAULT_VOLMAX = 0xFF;
  static constexpr uint8_t DEFAULT_VOLSTEP = 0x04;
  static constexpr int AUDIO_PAUSE = 0;
  static constexpr int AUDIO_RESUME = 1;
  static constexpr int CODEC_PDWN_HW = 1;
  static constexpr int CODEC_PDWN_SW = 2;
  static constexpr int AUDIO_MUTE_ON = 1;
  static constexpr int AUDIO_MUTE_OFF = 0;
  static constexpr uint32_t WM8994_ID = 0x8994;
  static constexpr uint16_t WM8994_CHIPID_ADDR = 0x00;
  static constexpr int WM8994_ADDR = 0x1A;

  WM8994() = default;

  /// Defines the I2C bus instance to be used
  void setWire(i2c_bus_handle_t handle) { i2c_handle = handle; }

  /// Defines the I2C device address
  void setAddress(int addr) {
    if (addr > 0) i2c_addr = addr;
  }

  /**
   * @brief Initializes the audio codec and the control interface.
   * @param OutputInputDevice: can be OUTPUT_DEVICE_SPEAKER, OUTPUT_DEVICE_HEADPHONE,
   *  OUTPUT_DEVICE_BOTH, OUTPUT_DEVICE_AUTO, INPUT_DEVICE_DIGITAL_MICROPHONE_1,
   *  INPUT_DEVICE_DIGITAL_MICROPHONE_2, INPUT_DEVICE_INPUT_LINE_1 or
   *  INPUT_DEVICE_INPUT_LINE_2.
   * @param Volume: Initial volume level (from 0 (Mute) to 100 (Max))
   * @param AudioFreq: Audio Frequency
   * @retval 0 if correct communication, else wrong communication
   */
  uint32_t init(uint16_t OutputInputDevice, uint8_t Volume,
                uint32_t AudioFreq) {
    uint32_t counter = 0;
    uint16_t output_device = OutputInputDevice & 0xFF;
    uint16_t input_device = OutputInputDevice & 0xFF00;
    uint16_t power_mgnt_reg_1 = 0;

    /* wm8994 Errata Work-Arounds */
    counter += writeReg16(0x102, 0x0003);
    counter += writeReg16(0x817, 0x0000);
    counter += writeReg16(0x102, 0x0000);

    /* Enable VMID soft start (fast), Start-up Bias Current Enabled */
    counter += writeReg16(0x39, 0x006C);

    /* Enable bias generator, Enable VMID */
    counter += writeReg16(0x01, 0x0003);

    /* Add Delay */
    delayMs(100);

    /* Path Configurations for output */
    if (output_device > 0) {
      output_enabled = 1;
      switch (output_device) {
        case OUTPUT_DEVICE_SPEAKER:
          /* Enable DAC1 (Left), Enable DAC1 (Right),
          Disable DAC2 (Left), Disable DAC2 (Right)*/
          counter += writeReg16(0x05, 0x0C0C);

          /* Enable the AIF1 Timeslot 0 (Left) to DAC 1 (Left) mixer path */
          counter += writeReg16(0x601, 0x0000);

          /* Enable the AIF1 Timeslot 0 (Right) to DAC 1 (Right) mixer path */
          counter += writeReg16(0x602, 0x0000);

          /* Disable the AIF1 Timeslot 1 (Left) to DAC 2 (Left) mixer path */
          counter += writeReg16(0x604, 0x0002);

          /* Disable the AIF1 Timeslot 1 (Right) to DAC 2 (Right) mixer path */
          counter += writeReg16(0x605, 0x0002);
          break;

        case OUTPUT_DEVICE_HEADPHONE:
          /* Disable DAC1 (Left), Disable DAC1 (Right),
          Enable DAC2 (Left), Enable DAC2 (Right)*/
          counter += writeReg16(0x05, 0x0303);

          /* Enable the AIF1 Timeslot 0 (Left) to DAC 1 (Left) mixer path */
          counter += writeReg16(0x601, 0x0001);

          /* Enable the AIF1 Timeslot 0 (Right) to DAC 1 (Right) mixer path */
          counter += writeReg16(0x602, 0x0001);

          /* Disable the AIF1 Timeslot 1 (Left) to DAC 2 (Left) mixer path */
          counter += writeReg16(0x604, 0x0000);

          /* Disable the AIF1 Timeslot 1 (Right) to DAC 2 (Right) mixer path */
          counter += writeReg16(0x605, 0x0000);
          break;

        case OUTPUT_DEVICE_BOTH:
          /* Enable DAC1 (Left), Enable DAC1 (Right),
          also Enable DAC2 (Left), Enable DAC2 (Right)*/
          counter += writeReg16(0x05, 0x0303 | 0x0C0C);

          /* Enable the AIF1 Timeslot 0 (Left) to DAC 1 (Left) mixer path */
          counter += writeReg16(0x601, 0x0001);

          /* Enable the AIF1 Timeslot 0 (Right) to DAC 1 (Right) mixer path */
          counter += writeReg16(0x602, 0x0001);

          /* Enable the AIF1 Timeslot 1 (Left) to DAC 2 (Left) mixer path */
          counter += writeReg16(0x604, 0x0002);

          /* Enable the AIF1 Timeslot 1 (Right) to DAC 2 (Right) mixer path */
          counter += writeReg16(0x605, 0x0002);
          break;

        case OUTPUT_DEVICE_AUTO:
        default:
          /* Disable DAC1 (Left), Disable DAC1 (Right),
          Enable DAC2 (Left), Enable DAC2 (Right)*/
          counter += writeReg16(0x05, 0x0303);

          /* Enable the AIF1 Timeslot 0 (Left) to DAC 1 (Left) mixer path */
          counter += writeReg16(0x601, 0x0001);

          /* Enable the AIF1 Timeslot 0 (Right) to DAC 1 (Right) mixer path */
          counter += writeReg16(0x602, 0x0001);

          /* Disable the AIF1 Timeslot 1 (Left) to DAC 2 (Left) mixer path */
          counter += writeReg16(0x604, 0x0000);

          /* Disable the AIF1 Timeslot 1 (Right) to DAC 2 (Right) mixer path */
          counter += writeReg16(0x605, 0x0000);
          break;
      }
    } else {
      output_enabled = 0;
    }

    /* Path Configurations for input */
    if (input_device > 0) {
      input_enabled = 1;
      switch (input_device) {
        case INPUT_DEVICE_DIGITAL_MICROPHONE_2:
          /* Enable AIF1ADC2 (Left), Enable AIF1ADC2 (Right)
           * Enable DMICDAT2 (Left), Enable DMICDAT2 (Right)
           * Enable Left ADC, Enable Right ADC */
          counter += writeReg16(0x04, 0x0C30);

          /* Enable AIF1 DRC2 Signal Detect & DRC in AIF1ADC2 Left/Right Timeslot 1 */
          counter += writeReg16(0x450, 0x00DB);

          /* Disable IN1L, IN1R, IN2L, IN2R, Enable Thermal sensor & shutdown */
          counter += writeReg16(0x02, 0x6000);

          /* Enable the DMIC2(Left) to AIF1 Timeslot 1 (Left) mixer path */
          counter += writeReg16(0x608, 0x0002);

          /* Enable the DMIC2(Right) to AIF1 Timeslot 1 (Right) mixer path */
          counter += writeReg16(0x609, 0x0002);

          /* GPIO1 pin configuration GP1_DIR = output, GP1_FN = AIF1 DRC2 signal detect */
          counter += writeReg16(0x700, 0x000E);
          break;

        case INPUT_DEVICE_INPUT_LINE_1:
          /* Enable AIF1ADC1 (Left), Enable AIF1ADC1 (Right)
           * Enable Left ADC, Enable Right ADC */
          counter += writeReg16(0x04, 0x0303);

          /* Enable AIF1 DRC1 Signal Detect & DRC in AIF1ADC1 Left/Right Timeslot 0 */
          counter += writeReg16(0x440, 0x00DB);

          /* Enable IN1L and IN1R, Disable IN2L and IN2R, Enable Thermal sensor & shutdown */
          counter += writeReg16(0x02, 0x6350);

          /* Enable the ADCL(Left) to AIF1 Timeslot 0 (Left) mixer path */
          counter += writeReg16(0x606, 0x0002);

          /* Enable the ADCR(Right) to AIF1 Timeslot 0 (Right) mixer path */
          counter += writeReg16(0x607, 0x0002);

          /* GPIO1 pin configuration GP1_DIR = output, GP1_FN = AIF1 DRC1 signal detect */
          counter += writeReg16(0x700, 0x000D);
          break;

        case INPUT_DEVICE_DIGITAL_MICROPHONE_1:
        case INPUT_DEVICE_INPUT_LINE_2:
        default:
          /* Actually, no other input devices supported */
          counter++;
          break;
      }
    } else {
      input_enabled = 0;
    }

    /*  Clock Configurations */
    switch (AudioFreq) {
      case AUDIO_FREQUENCY_8K:
        /* AIF1 Sample Rate = 8 (KHz), ratio=256 */
        counter += writeReg16(0x210, 0x0003);
        break;

      case AUDIO_FREQUENCY_16K:
        /* AIF1 Sample Rate = 16 (KHz), ratio=256 */
        counter += writeReg16(0x210, 0x0033);
        break;

      case AUDIO_FREQUENCY_48K:
        /* AIF1 Sample Rate = 48 (KHz), ratio=256 */
        counter += writeReg16(0x210, 0x0083);
        break;

      case AUDIO_FREQUENCY_96K:
        /* AIF1 Sample Rate = 96 (KHz), ratio=256 */
        counter += writeReg16(0x210, 0x00A3);
        break;

      case AUDIO_FREQUENCY_11K:
        /* AIF1 Sample Rate = 11.025 (KHz), ratio=256 */
        counter += writeReg16(0x210, 0x0013);
        break;

      case AUDIO_FREQUENCY_22K:
        /* AIF1 Sample Rate = 22.050 (KHz), ratio=256 */
        counter += writeReg16(0x210, 0x0043);
        break;

      case AUDIO_FREQUENCY_44K:
        /* AIF1 Sample Rate = 44.1 (KHz), ratio=256 */
        counter += writeReg16(0x210, 0x0073);
        break;

      default:
        /* AIF1 Sample Rate = 48 (KHz), ratio=256 */
        counter += writeReg16(0x210, 0x0083);
        break;
    }
    /* AIF1 Word Length = 16-bits, AIF1 Format = I2S (Default Register Value) */
    counter += writeReg16(0x300, 0x4010);

    /* slave mode */
    counter += writeReg16(0x302, 0x0000);

    /* Enable the DSP processing clock for AIF1, Enable the core clock */
    counter += writeReg16(0x208, 0x000A);

    /* Enable AIF1 Clock, AIF1 Clock Source = MCLK1 pin */
    counter += writeReg16(0x200, 0x0001);

    if (output_device > 0) /* Audio output selected */
    {
      if (output_device == OUTPUT_DEVICE_HEADPHONE) {
        // Missing vs. the official ST BSP wm8994_Init(): without this, the
        // headphone path never actually gets brought up by WM8994's internal
        // write-sequencer, so I2C/DAC/mixer config can all succeed
        // (indistinguishable in logs/DMA behavior) while HPOUT stays silent.

        /* Select DAC1 (Left) to Left Headphone Output PGA (HPOUT1LVOL) path */
        counter += writeReg16(0x2D, 0x0100);

        /* Select DAC1 (Right) to Right Headphone Output PGA (HPOUT1RVOL) path */
        counter += writeReg16(0x2E, 0x0100);

        /* Startup sequence for Headphone.
         * ST's own STM32F769I-DISCO errata (confirmed on their community
         * forum) says the "reference" 300ms/50ms cold/warm delays here only
         * hold for the 8/16/32/48/96kHz family. The write-sequencer's
         * internal ROM runs off a different derived clock for the
         * 11.025/22.05/44.1kHz family, so cold start needs ~325-350ms there
         * - if writes to the output-mixer/volume/AIF-mute registers
         * (0x1C/0x1D/0x2D/0x2E/0x420/0x422) follow too soon, the codec
         * silently ignores them (no NACK, no error) while it's still
         * settling. */
        bool is_441k_family = (AudioFreq == AUDIO_FREQUENCY_11K) ||
                              (AudioFreq == AUDIO_FREQUENCY_22K) ||
                              (AudioFreq == AUDIO_FREQUENCY_44K);
        uint32_t cold_wait_ms = is_441k_family ? 700 : 600;
        uint32_t warm_wait_ms = is_441k_family ? 120 : 100;

        if (is_cold_startup) {
          counter += writeReg16(0x110, 0x8100);
          is_cold_startup = false;
          delayMs(cold_wait_ms);
        } else {
          /* Headphone Warm Start-Up */
          counter += writeReg16(0x110, 0x8108);
          delayMs(warm_wait_ms);
        }

        /* Diagnostic only: WSEQ_BUSY (Write Sequencer Ctrl (2), reg 0x111,
         * bit 8) clears almost immediately regardless of family - it only
         * reflects the write-sequencer's own ROM step-through, not the
         * analog charge-pump/DC-servo settling that continues in the
         * background for the delay above, so it must not be used to
         * shorten the wait. */
        uint16_t seq_status = readReg16(0x111);
        AD_LOGD("write sequencer: busy=%d after trigger (0x111=0x%04X)",
                (seq_status & 0x0100) != 0, seq_status);

        /* Soft un-Mute the AIF1 Timeslot 0 DAC1 path L&R */
        counter += writeReg16(0x420, 0x0000);
      }

      /* Analog Output Configuration */

      /* Enable SPKRVOL PGA, Enable SPKMIXR, Enable SPKLVOL PGA, Enable SPKMIXL */
      counter += writeReg16(0x03, 0x0300);

      /* Left Speaker Mixer Volume = 0dB */
      counter += writeReg16(0x22, 0x0000);

      /* Speaker output mode = Class D, Right Speaker Mixer Volume = 0dB ((0x23, 0x0100) = class AB)*/
      counter += writeReg16(0x23, 0x0000);

      /* Unmute DAC2 (Left) to Left Speaker Mixer (SPKMIXL) path,
      Unmute DAC2 (Right) to Right Speaker Mixer (SPKMIXR) path */
      counter += writeReg16(0x36, 0x0300);

      /* Enable bias generator, Enable VMID, Enable SPKOUTL, Enable SPKOUTR */
      counter += writeReg16(0x01, 0x3003);

      /* Headphone/Speaker Enable */

      /* Enable Class W, Class W Envelope Tracking = AIF1 Timeslot 0 */
      counter += writeReg16(0x51, 0x0005);

      /* Enable bias generator, Enable VMID, Enable HPOUT1 (Left) and Enable HPOUT1 (Right) input stages */
      /* idem for Speaker */
      power_mgnt_reg_1 |= 0x0303 | 0x3003;
      counter += writeReg16(0x01, power_mgnt_reg_1);

      /* Enable HPOUT1 (Left) and HPOUT1 (Right) intermediate stages */
      counter += writeReg16(0x60, 0x0022);

      /* Enable Charge Pump */
      counter += writeReg16(0x4C, 0x9F25);

      /* Add Delay */
      delayMs(100);

      /* Select DAC1 (Left) to Left Headphone Output PGA (HPOUT1LVOL) path */
      counter += writeReg16(0x2D, 0x0001);

      /* Select DAC1 (Right) to Right Headphone Output PGA (HPOUT1RVOL) path */
      counter += writeReg16(0x2E, 0x0001);

      /* Enable Left Output Mixer (MIXOUTL), Enable Right Output Mixer (MIXOUTR) */
      /* idem for SPKOUTL and SPKOUTR */
      counter += writeReg16(0x03, 0x0030 | 0x0300);

      /* Enable DC Servo and trigger start-up mode on left and right channels */
      counter += writeReg16(0x54, 0x0033);

      /* Add Delay */
      delayMs(514);

      AD_LOGD("before clamp removal: 0x01=0x%X 0x2D=0x%X", readReg16(0x01),
              readReg16(0x2D));

      /* Enable HPOUT1 (Left) and HPOUT1 (Right) intermediate and output stages. Remove clamps */
      counter += writeReg16(0x60, 0x00EE);

      AD_LOGD("after clamp removal: 0x01=0x%X 0x2D=0x%X", readReg16(0x01),
              readReg16(0x2D));

      /* Unmutes */

      /* Unmute DAC 1 (Left) */
      counter += writeReg16(0x610, 0x00C0);

      /* Unmute DAC 1 (Right) */
      counter += writeReg16(0x611, 0x00C0);

      /* Unmute the AIF1 Timeslot 0 DAC path */
      counter += writeReg16(0x420, 0x0010);

      /* Unmute DAC 2 (Left) */
      counter += writeReg16(0x612, 0x00C0);

      /* Unmute DAC 2 (Right) */
      counter += writeReg16(0x613, 0x00C0);

      /* Unmute the AIF1 Timeslot 1 DAC2 path */
      counter += writeReg16(0x422, 0x0010);

      /* Volume Control */
      setVolume(Volume);
    }

    if (input_device > 0) /* Audio input selected */
    {
      if ((input_device == INPUT_DEVICE_DIGITAL_MICROPHONE_1) ||
          (input_device == INPUT_DEVICE_DIGITAL_MICROPHONE_2)) {
        /* Enable Microphone bias 1 generator, Enable VMID */
        power_mgnt_reg_1 |= 0x0013;
        counter += writeReg16(0x01, power_mgnt_reg_1);

        /* ADC oversample enable */
        counter += writeReg16(0x620, 0x0002);

        /* AIF ADC2 HPF enable, HPF cut = voice mode 1 fc=127Hz at fs=8kHz */
        counter += writeReg16(0x411, 0x3800);
      } else if ((input_device == INPUT_DEVICE_INPUT_LINE_1) ||
                 (input_device == INPUT_DEVICE_INPUT_LINE_2)) {
        /* Enable normal bias generator, Enable VMID */
        power_mgnt_reg_1 |= 0x0003;
        counter += writeReg16(0x01, power_mgnt_reg_1);

        /* Disable mute on IN1L, IN1L Volume = +0dB */
        counter += writeReg16(0x18, 0x000B);

        /* Disable mute on IN1R, IN1R Volume = +0dB */
        counter += writeReg16(0x1A, 0x000B);

        /* Disable mute on IN1L_TO_MIXINL, Gain = +0dB */
        counter += writeReg16(0x29, 0x0025);

        /* Disable mute on IN1R_TO_MIXINL, Gain = +0dB */
        counter += writeReg16(0x2A, 0x0025);

        /* IN1LN_TO_IN1L, IN1LP_TO_VMID, IN1RN_TO_IN1R, IN1RP_TO_VMID */
        counter += writeReg16(0x28, 0x0011);

        /* AIF ADC1 HPF enable, HPF cut = hifi mode fc=4Hz at fs=48kHz */
        counter += writeReg16(0x410, 0x1800);
      }
      /* Volume Control */
      setVolume(Volume);
    }
    /* Return communication control value */
    return counter;
  }

  /// Deinitializes the audio codec.
  void deinit() {
    /* Deinitialize Audio Codec interface */
  }

  /// Get the WM8994 ID.
  uint32_t readId() { return ((uint32_t)readReg16(WM8994_CHIPID_ADDR)); }

  /// Reads back and logs (AD_LOGI) every register this driver writes to -
  /// useful to confirm a register actually retained a value instead of
  /// trusting a successful I2C ACK as proof.
  void dumpRegisters() {
    static const struct {
      uint16_t reg;
      const char *name;
    } regs[] = {
        {0x00, "SW Reset / Chip ID"},
        {0x01, "Power Management (1)"},
        {0x02, "Power Management (2)"},
        {0x03, "Power Management (3)"},
        {0x04, "Power Management (4)"},
        {0x05, "Power Management (5)"},
        {0x18, "Left Line Input 1&2 Volume"},
        {0x1A, "Right Line Input 1&2 Volume"},
        {0x1C, "Left Output Volume"},
        {0x1D, "Right Output Volume"},
        {0x22, "Left OPGA Volume"},
        {0x23, "Right OPGA Volume"},
        {0x26, "Speaker Volume Left"},
        {0x27, "Speaker Volume Right"},
        {0x28, "Input Mixer (2)"},
        {0x29, "Input Mixer (3)"},
        {0x2A, "Input Mixer (4)"},
        {0x2D, "Output Mixer (1)"},
        {0x2E, "Output Mixer (2)"},
        {0x36, "Output Mixer (5)"},
        {0x39, "Antipop (1)"},
        {0x51, "Class W (1)"},
        {0x54, "DC Servo (1)"},
        {0x60, "Analogue HP (1)"},
        {0x4C, "Charge Pump (1)"},
        {0x102, "Clocking (1) - errata"},
        {0x110, "Write Sequencer Ctrl (1)"},
        {0x200, "AIF1 Clocking (1)"},
        {0x208, "Clocking (1)"},
        {0x210, "AIF1 Rate"},
        {0x300, "AIF1 Control (1)"},
        {0x302, "AIF1 Master/Slave"},
        {0x410, "AIF1 ADC1 Filters"},
        {0x411, "AIF1 ADC2 Filters"},
        {0x420, "AIF1 DAC1 Filters (1)"},
        {0x422, "AIF1 DAC2 Filters (1)"},
        {0x440, "AIF1 DRC1 (1)"},
        {0x450, "AIF1 DRC2 (1)"},
        {0x601, "AIF1 DAC1 Left Mixer Routing"},
        {0x602, "AIF1 DAC1 Right Mixer Routing"},
        {0x604, "AIF1 DAC2 Left Mixer Routing"},
        {0x605, "AIF1 DAC2 Right Mixer Routing"},
        {0x606, "DAC1 Left Mixer Routing"},
        {0x607, "DAC1 Right Mixer Routing"},
        {0x608, "DAC2 Left Mixer Routing"},
        {0x609, "DAC2 Right Mixer Routing"},
        {0x610, "DAC1 Left Volume"},
        {0x611, "DAC1 Right Volume"},
        {0x612, "DAC2 Left Volume"},
        {0x613, "DAC2 Right Volume"},
        {0x620, "Oversampling"},
        {0x700, "GPIO 1"},
        {0x817, "Clocking - errata"},
    };
    AD_LOGI("WM8994 register dump (%d registers):",
            (int)(sizeof(regs) / sizeof(regs[0])));
    for (auto &r : regs) {
      AD_LOGI("  0x%04X %-28s = 0x%04X", r.reg, r.name, readReg16(r.reg));
    }
  }

  /**
   * @brief Start the audio Codec play feature.
   * @note For this codec no Play options are required.
   * @retval 0 if correct communication, else wrong communication
   */
  uint32_t play(uint16_t *pBuffer, uint16_t Size) {
    uint32_t counter = 0;

    /* Resumes the audio file playing */
    /* Unmute the output first */
    counter += setMute(AUDIO_MUTE_OFF);

    return counter;
  }

  /// Pauses playing on the audio codec.
  uint32_t pause() {
    uint32_t counter = 0;

    /* Pause the audio file playing */
    /* Mute the output first */
    counter += setMute(AUDIO_MUTE_ON);

    /* Put the Codec in Power save mode */
    counter += writeReg16(0x02, 0x01);

    return counter;
  }

  /// Resumes playing on the audio codec.
  uint32_t resume() {
    uint32_t counter = 0;

    /* Resumes the audio file playing */
    /* Unmute the output first */
    counter += setMute(AUDIO_MUTE_OFF);

    return counter;
  }

  /**
   * @brief Stops audio Codec playing. It powers down the codec.
   * @param CodecPdwnMode: selects the power down mode.
   *          - CODEC_PDWN_SW: only mutes the audio codec. When resuming from this
   *                           mode the codec keeps the previous initialization
   *                           (no need to re-Initialize the codec registers).
   *          - CODEC_PDWN_HW: Physically power down the codec. When resuming from
   *                           this mode, the codec is set to default configuration
   *                           (user should re-Initialize the codec in order to
   *                            play again the audio stream).
   * @retval 0 if correct communication, else wrong communication
   */
  uint32_t stop(uint32_t CodecPdwnMode) {
    uint32_t counter = 0;

    if (output_enabled != 0) {
      /* Mute the output first */
      counter += setMute(AUDIO_MUTE_ON);

      if (CodecPdwnMode == CODEC_PDWN_SW) {
        /* Only output mute required*/
      } else /* CODEC_PDWN_HW */
      {
        /* Mute the AIF1 Timeslot 0 DAC1 path */
        counter += writeReg16(0x420, 0x0200);

        /* Mute the AIF1 Timeslot 1 DAC2 path */
        counter += writeReg16(0x422, 0x0200);

        /* Disable DAC1L_TO_HPOUT1L */
        counter += writeReg16(0x2D, 0x0000);

        /* Disable DAC1R_TO_HPOUT1R */
        counter += writeReg16(0x2E, 0x0000);

        /* Disable DAC1 and DAC2 */
        counter += writeReg16(0x05, 0x0000);

        /* Reset Codec by writing in 0x0000 address register */
        counter += writeReg16(0x0000, 0x0000);

        output_enabled = 0;
      }
    }
    return counter;
  }

  /**
   * @brief Sets higher or lower the codec volume level.
   * @param Volume: a byte value from 0 to 255 (refer to codec registers
   *         description for more details).
   * @retval 0 if correct communication, else wrong communication
   */
  uint32_t setVolume(uint8_t Volume) {
    uint32_t counter = 0;
    uint8_t convertedvol = wm8994VolumeConvert(Volume);
    AD_LOGD("setVolume: Volume=%d convertedvol=0x%X output_enabled=%d", Volume,
            convertedvol, output_enabled);

    /* Output volume */
    if (output_enabled != 0) {
      if (convertedvol > 0x3E) {
        /* Unmute audio codec */
        counter += setMute(AUDIO_MUTE_OFF);

        /* Left Headphone Volume */
        counter += writeReg16(0x1C, 0x3F | 0x140);

        /* Right Headphone Volume */
        counter += writeReg16(0x1D, 0x3F | 0x140);

        /* Left Speaker Volume */
        counter += writeReg16(0x26, 0x3F | 0x140);

        /* Right Speaker Volume */
        counter += writeReg16(0x27, 0x3F | 0x140);

        AD_LOGD("after volume writes: 0x420=0x%X 0x1C=0x%X", readReg16(0x420),
                readReg16(0x1C));
      } else if (Volume == 0) {
        /* Mute audio codec */
        counter += setMute(AUDIO_MUTE_ON);
      } else {
        /* Unmute audio codec */
        counter += setMute(AUDIO_MUTE_OFF);

        /* Left Headphone Volume */
        counter += writeReg16(0x1C, convertedvol | 0x140);

        /* Right Headphone Volume */
        counter += writeReg16(0x1D, convertedvol | 0x140);

        /* Left Speaker Volume */
        counter += writeReg16(0x26, convertedvol | 0x140);

        /* Right Speaker Volume */
        counter += writeReg16(0x27, convertedvol | 0x140);
      }
    }

    /* Input volume */
    if (input_enabled != 0) {
      convertedvol = volumeInConvert(Volume);

      /* Left AIF1 ADC1 volume */
      counter += writeReg16(0x400, convertedvol | 0x100);

      /* Right AIF1 ADC1 volume */
      counter += writeReg16(0x401, convertedvol | 0x100);

      /* Left AIF1 ADC2 volume */
      counter += writeReg16(0x404, convertedvol | 0x100);

      /* Right AIF1 ADC2 volume */
      counter += writeReg16(0x405, convertedvol | 0x100);
    }
    return counter;
  }

  /**
   * @brief Enables or disables the mute feature on the audio codec.
   * @param Cmd: AUDIO_MUTE_ON to enable the mute or AUDIO_MUTE_OFF to disable the
   *             mute mode.
   * @retval 0 if correct communication, else wrong communication
   */
  uint32_t setMute(uint32_t Cmd) {
    uint32_t counter = 0;

    if (output_enabled != 0) {
      /* Set the Mute mode */
      if (Cmd == AUDIO_MUTE_ON) {
        /* Soft Mute the AIF1 Timeslot 0 DAC1 path L&R */
        counter += writeReg16(0x420, 0x0200);

        /* Soft Mute the AIF1 Timeslot 1 DAC2 path L&R */
        counter += writeReg16(0x422, 0x0200);
      } else /* AUDIO_MUTE_OFF Disable the Mute */
      {
        /* Unmute the AIF1 Timeslot 0 DAC1 path L&R */
        counter += writeReg16(0x420, 0x0000);

        /* Unmute the AIF1 Timeslot 1 DAC2 path L&R */
        counter += writeReg16(0x422, 0x0000);
      }
    }
    return counter;
  }

  /**
   * @brief Switch dynamically (while audio file is played) the output target
   *         (speaker or headphone).
   * @param Output: specifies the audio output target: OUTPUT_DEVICE_SPEAKER,
   *         OUTPUT_DEVICE_HEADPHONE, OUTPUT_DEVICE_BOTH or OUTPUT_DEVICE_AUTO
   * @retval 0 if correct communication, else wrong communication
   */
  uint32_t setOutputMode(uint8_t Output) {
    uint32_t counter = 0;

    switch (Output) {
      case OUTPUT_DEVICE_SPEAKER:
        /* Enable DAC1 (Left), Enable DAC1 (Right),
        Disable DAC2 (Left), Disable DAC2 (Right)*/
        counter += writeReg16(0x05, 0x0C0C);

        /* Enable the AIF1 Timeslot 0 (Left) to DAC 1 (Left) mixer path */
        counter += writeReg16(0x601, 0x0000);

        /* Enable the AIF1 Timeslot 0 (Right) to DAC 1 (Right) mixer path */
        counter += writeReg16(0x602, 0x0000);

        /* Disable the AIF1 Timeslot 1 (Left) to DAC 2 (Left) mixer path */
        counter += writeReg16(0x604, 0x0002);

        /* Disable the AIF1 Timeslot 1 (Right) to DAC 2 (Right) mixer path */
        counter += writeReg16(0x605, 0x0002);
        break;

      case OUTPUT_DEVICE_HEADPHONE:
        /* Disable DAC1 (Left), Disable DAC1 (Right),
        Enable DAC2 (Left), Enable DAC2 (Right)*/
        counter += writeReg16(0x05, 0x0303);

        /* Enable the AIF1 Timeslot 0 (Left) to DAC 1 (Left) mixer path */
        counter += writeReg16(0x601, 0x0001);

        /* Enable the AIF1 Timeslot 0 (Right) to DAC 1 (Right) mixer path */
        counter += writeReg16(0x602, 0x0001);

        /* Disable the AIF1 Timeslot 1 (Left) to DAC 2 (Left) mixer path */
        counter += writeReg16(0x604, 0x0000);

        /* Disable the AIF1 Timeslot 1 (Right) to DAC 2 (Right) mixer path */
        counter += writeReg16(0x605, 0x0000);
        break;

      case OUTPUT_DEVICE_BOTH:
        /* Enable DAC1 (Left), Enable DAC1 (Right),
        also Enable DAC2 (Left), Enable DAC2 (Right)*/
        counter += writeReg16(0x05, 0x0303 | 0x0C0C);

        /* Enable the AIF1 Timeslot 0 (Left) to DAC 1 (Left) mixer path */
        counter += writeReg16(0x601, 0x0001);

        /* Enable the AIF1 Timeslot 0 (Right) to DAC 1 (Right) mixer path */
        counter += writeReg16(0x602, 0x0001);

        /* Enable the AIF1 Timeslot 1 (Left) to DAC 2 (Left) mixer path */
        counter += writeReg16(0x604, 0x0002);

        /* Enable the AIF1 Timeslot 1 (Right) to DAC 2 (Right) mixer path */
        counter += writeReg16(0x605, 0x0002);
        break;

      default:
        /* Disable DAC1 (Left), Disable DAC1 (Right),
        Enable DAC2 (Left), Enable DAC2 (Right)*/
        counter += writeReg16(0x05, 0x0303);

        /* Enable the AIF1 Timeslot 0 (Left) to DAC 1 (Left) mixer path */
        counter += writeReg16(0x601, 0x0001);

        /* Enable the AIF1 Timeslot 0 (Right) to DAC 1 (Right) mixer path */
        counter += writeReg16(0x602, 0x0001);

        /* Disable the AIF1 Timeslot 1 (Left) to DAC 2 (Left) mixer path */
        counter += writeReg16(0x604, 0x0000);

        /* Disable the AIF1 Timeslot 1 (Right) to DAC 2 (Right) mixer path */
        counter += writeReg16(0x605, 0x0000);
        break;
    }
    return counter;
  }

  /**
   * @brief Sets new frequency.
   * @param AudioFreq: Audio frequency used to play the audio stream.
   * @retval 0 if correct communication, else wrong communication
   */
  uint32_t setFrequency(uint32_t AudioFreq) {
    uint32_t counter = 0;

    /*  Clock Configurations */
    switch (AudioFreq) {
      case AUDIO_FREQUENCY_8K:
        /* AIF1 Sample Rate = 8 (KHz), ratio=256 */
        counter += writeReg16(0x210, 0x0003);
        break;

      case AUDIO_FREQUENCY_16K:
        /* AIF1 Sample Rate = 16 (KHz), ratio=256 */
        counter += writeReg16(0x210, 0x0033);
        break;

      case AUDIO_FREQUENCY_48K:
        /* AIF1 Sample Rate = 48 (KHz), ratio=256 */
        counter += writeReg16(0x210, 0x0083);
        break;

      case AUDIO_FREQUENCY_96K:
        /* AIF1 Sample Rate = 96 (KHz), ratio=256 */
        counter += writeReg16(0x210, 0x00A3);
        break;

      case AUDIO_FREQUENCY_11K:
        /* AIF1 Sample Rate = 11.025 (KHz), ratio=256 */
        counter += writeReg16(0x210, 0x0013);
        break;

      case AUDIO_FREQUENCY_22K:
        /* AIF1 Sample Rate = 22.050 (KHz), ratio=256 */
        counter += writeReg16(0x210, 0x0043);
        break;

      case AUDIO_FREQUENCY_44K:
        /* AIF1 Sample Rate = 44.1 (KHz), ratio=256 */
        counter += writeReg16(0x210, 0x0073);
        break;

      default:
        /* AIF1 Sample Rate = 48 (KHz), ratio=256 */
        counter += writeReg16(0x210, 0x0083);
        break;
    }
    return counter;
  }

  /// Resets wm8994 registers.
  uint32_t reset() {
    uint32_t counter = 0;

    /* Reset Codec by writing in 0x0000 address register */
    counter = writeReg16(0x0000, 0x0000);
    output_enabled = 0;
    input_enabled = 0;

    return counter;
  }
  /// Writes a 16-bit register value over I2C. WM8994 expects both the
  /// register address and the data value big-endian (MSB first) on the
  /// wire - (uint8_t*)&reg/&value would reinterpret the host's in-memory
  /// byte order instead (little-endian on Cortex-M), silently sending
  /// everything byte-swapped. Confirmed live: reading back the chip ID
  /// returned 0x9489 instead of 0x8994 - exactly its bytes swapped.
  void writeReg(uint16_t reg, uint16_t value) {
    uint8_t reg_be[2] = {(uint8_t)(reg >> 8), (uint8_t)(reg & 0xFF)};
    uint8_t value_be[2] = {(uint8_t)(value >> 8), (uint8_t)(value & 0xFF)};
    i2c_bus_write_bytes(i2c_handle, i2c_addr, reg_be, sizeof(reg_be), value_be,
                        sizeof(value_be));
  }

  /// Reads back a register value over I2C (see writeReg's note on byte order)
  uint16_t readReg16(uint16_t reg) {
    uint8_t reg_be[2] = {(uint8_t)(reg >> 8), (uint8_t)(reg & 0xFF)};
    uint8_t value_be[2] = {0, 0};
    i2c_bus_read_bytes(i2c_handle, i2c_addr, reg_be, sizeof(reg_be), value_be,
                       sizeof(value_be));
    return ((uint16_t)value_be[0] << 8) | value_be[1];
  }

  /**
   * @brief  Writes a single data value to a codec register.
   * @param  Reg: Register address
   * @param  Value: Data to be written
   * @retval 0 on success, non-zero on verify failure
   */
  uint8_t writeReg16(uint16_t Reg, uint16_t Value) {
    uint32_t result = 0;

    writeReg(Reg, Value);

#ifdef VERIFY_WRITTENDATA
    /* Verify that the data has been correctly written */
    result = (readReg16(Reg) == Value) ? 0 : 1;
#endif /* VERIFY_WRITTENDATA */

    return result;
  }

  static constexpr uint8_t wm8994VolumeConvert(uint8_t volume) {
    return volume > 100 ? 100 : (uint8_t)((volume * 63) / 100);
  }
  static constexpr uint8_t volumeInConvert(uint8_t volume) {
    return volume >= 100 ? 239 : (uint8_t)((volume * 240) / 100);
  }

 protected:
  i2c_bus_handle_t i2c_handle = nullptr;
  int i2c_addr = WM8994_ADDR;
  uint32_t output_enabled = 0;
  uint32_t input_enabled = 0;
  /// First-ever headphone power-up needs the longer (cold) sequencer delay;
  /// see the register 0x110 write in init() below.
  bool is_cold_startup = true;
};

}  // namespace audio_driver

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
