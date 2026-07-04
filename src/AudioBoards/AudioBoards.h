#pragma once
// All ESP32 boards using gpios as ints are automatically included in AudioBoard.h
#if defined(ESP32)
#include "AudioKitAC101.h"
#include "AudioKitEs8388v1.h"
#include "AudioKitEs8388v2.h"
#include "Lyrat42.h"    
#include "Lyrat43.h"
#include "LyratMini.h"
#include "M5stackAtomEchoS3R.h"
#include "ESP32S3AISmartSpeaker.h"
#endif

// Import STM32 boards
#if defined(ARDUINO_ARCH_STM32) || defined(STM32)
#include "STM32F411Disco.h"
#endif