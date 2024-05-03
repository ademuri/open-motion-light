#pragma once

#include <cstdint>

// Fake pin definitions for native code

static constexpr uint32_t kPortA = 0;
static constexpr uint32_t kPortB = 16;
static constexpr uint32_t kPortC = 32;

static constexpr uint32_t PA0 = kPortA + 0;
static constexpr uint32_t PA1 = kPortA + 1;
static constexpr uint32_t PA2 = kPortA + 2;
static constexpr uint32_t PA3 = kPortA + 3;
static constexpr uint32_t PA4 = kPortA + 4;
static constexpr uint32_t PA5 = kPortA + 5;
static constexpr uint32_t PA6 = kPortA + 6;
static constexpr uint32_t PA7 = kPortA + 7;
static constexpr uint32_t PA8 = kPortA + 8;
static constexpr uint32_t PA9 = kPortA + 9;
static constexpr uint32_t PA10 = kPortA + 10;
static constexpr uint32_t PA11 = kPortA + 11;
static constexpr uint32_t PA12 = kPortA + 12;
static constexpr uint32_t PA13 = kPortA + 13;
static constexpr uint32_t PA14 = kPortA + 14;
static constexpr uint32_t PA15 = kPortA + 15;

static constexpr uint32_t PB0 = kPortB + 0;
static constexpr uint32_t PB1 = kPortB + 1;
static constexpr uint32_t PB2 = kPortB + 2;
static constexpr uint32_t PB3 = kPortB + 3;
static constexpr uint32_t PB4 = kPortB + 4;
static constexpr uint32_t PB5 = kPortB + 5;
static constexpr uint32_t PB6 = kPortB + 6;
static constexpr uint32_t PB7 = kPortB + 7;
static constexpr uint32_t PB8 = kPortB + 8;
static constexpr uint32_t PB9 = kPortB + 9;
static constexpr uint32_t PB10 = kPortB + 10;
static constexpr uint32_t PB11 = kPortB + 11;
static constexpr uint32_t PB12 = kPortB + 12;
static constexpr uint32_t PB13 = kPortB + 13;
static constexpr uint32_t PB14 = kPortB + 14;
static constexpr uint32_t PB15 = kPortB + 15;

static constexpr uint32_t PC0 = kPortC + 0;
static constexpr uint32_t PC1 = kPortC + 1;
static constexpr uint32_t PC2 = kPortC + 2;
static constexpr uint32_t PC3 = kPortC + 3;
static constexpr uint32_t PC4 = kPortC + 4;
static constexpr uint32_t PC5 = kPortC + 5;
static constexpr uint32_t PC6 = kPortC + 6;
static constexpr uint32_t PC7 = kPortC + 7;
static constexpr uint32_t PC8 = kPortC + 8;
static constexpr uint32_t PC9 = kPortC + 9;
static constexpr uint32_t PC10 = kPortC + 10;
static constexpr uint32_t PC11 = kPortC + 11;
static constexpr uint32_t PC12 = kPortC + 12;
static constexpr uint32_t PC13 = kPortC + 13;
static constexpr uint32_t PC14 = kPortC + 14;
static constexpr uint32_t PC15 = kPortC + 15;

static constexpr uint32_t kPinMax = PC15;
