//---------------------------------------------------------------------------

#pragma once
#include <stdint.h>

//---------------------------------------------------------------------------

struct SysTick {
  volatile uint32_t csr;
  volatile uint32_t rvr;
  volatile uint32_t cvr;
  volatile uint32_t calib;

  void EnableCycleCount() {
    csr |= 5;
    rvr = 0xffffff;
  }
  uint32_t ReadCycleCount() const { return -cvr; }
};

SysTick *const sysTick = (SysTick *)0xE000E010;

//---------------------------------------------------------------------------
