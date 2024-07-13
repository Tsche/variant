#pragma once

#define SLO_STAMP4(offset, x) x(offset) x(offset + 1) x(offset + 2) x(offset + 3)

#define SLO_STAMP16(offset, x) \
  SLO_STAMP4(offset, x)        \
  SLO_STAMP4(offset + 4, x)    \
  SLO_STAMP4(offset + 8, x)    \
  SLO_STAMP4(offset + 12, x)

#define SLO_STAMP64(offset, x) \
  SLO_STAMP16(offset, x)       \
  SLO_STAMP16(offset + 16, x)  \
  SLO_STAMP16(offset + 32, x)  \
  SLO_STAMP16(offset + 48, x)

#define SLO_STAMP256(offset, x) \
  SLO_STAMP64(offset, x)        \
  SLO_STAMP64(offset + 64, x)   \
  SLO_STAMP64(offset + 128, x)  \
  SLO_STAMP64(offset + 192, x)

#define SLO_STAMP(offset, x) x(SLO_STAMP##offset, offset)