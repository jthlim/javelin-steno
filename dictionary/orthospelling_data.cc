//---------------------------------------------------------------------------

#include "orthospelling_data.h"

//---------------------------------------------------------------------------

const OrthospellingData::Starter *
OrthospellingData::GetStarterDefinition(StenoStroke stroke) const {
  for (const Starter &starter : starters) {
    if ((stroke & starter.mask) == starter.stroke) {
      return &starter;
    }
  }
  return nullptr;
}

const bool OrthospellingData::IsExit(StenoStroke stroke) const {
  for (const Exit &exit : exits) {
    if (((stroke & exit.fullMask) == exit.stroke) == exit.polarity) {
      return true;
    }
  }
  return false;
}

bool OrthospellingData::ConvertToText(StenoStroke stroke,
                                      OrthospellingContext context) const {
  for (const Letter &letter : letters) {
    if ((letter.stroke & stroke) == letter.stroke) {
      const StenoStroke remainingStroke = stroke & ~letter.stroke;
      const char *s = letter.data;
      char *p = context.buffers[letter.order];
      while (*s) {
        *p++ = *s++;
      }
      *p = '\0';
      if (remainingStroke.IsEmpty()) {
        return true;
      }

      OrthospellingContext remainingContext = context;
      remainingContext.buffers[letter.order] = p;
      if (ConvertToText(remainingStroke, remainingContext)) {
        return true;
      }
    }
  }
  return false;
}

//---------------------------------------------------------------------------
