//---------------------------------------------------------------------------

#pragma once
#include "container/sized_list.h"
#include "orthography.h"
#include "stroke.h"

//---------------------------------------------------------------------------

struct StenoSystem {
  const char *name;
  const char *layout;
  StenoStroke undoStroke;
  SizedList<StrokeKey> keys;
  StenoOrthography orthography;

  void Print() const;

  static void Print_Binding(void *context, const char *commandLine);
};

//---------------------------------------------------------------------------
