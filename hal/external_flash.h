//---------------------------------------------------------------------------

#pragma once

//---------------------------------------------------------------------------

class ExternalFlash {
public:
#if JAVELIN_PLATFORM_NRF5_SDK
  static void Begin();
  static void End();
#else
  static void Begin() {}
  static void End() {}
#endif
};

class ExternalFlashSentry {
public:
  ExternalFlashSentry() { ExternalFlash::Begin(); }
  ~ExternalFlashSentry() { ExternalFlash::End(); }
};

//---------------------------------------------------------------------------
