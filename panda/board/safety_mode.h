#pragma once

static inline uint16_t get_supported_safety_mode(uint16_t mode, bool has_canfd) {
  return ((!has_canfd) && (mode == SAFETY_HYUNDAI_CANFD)) ? SAFETY_SILENT : mode;
}