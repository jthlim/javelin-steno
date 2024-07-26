//---------------------------------------------------------------------------

#include "orthospelling_data.h"

//---------------------------------------------------------------------------

const char *OrthospellingData::GetStarterDefinition(StenoStroke stroke) const {
  for (const Starter &starter : starters) {
    if (starter.stroke == stroke) {
      return starter.definition;
    }
  }
  return nullptr;
}

const bool OrthospellingData::IsExit(StenoStroke stroke) const {
  for (const Exit &exit : exits) {
    if ((stroke & exit.mask) == exit.stroke) {
      return true;
    }
  }
  return false;
}

bool OrthospellingData::ConvertToText(StenoStroke stroke,
                                      OrthospellingContext context) const {
  for (const Definition &definition : definitions) {
    if ((definition.stroke & stroke) == definition.stroke) {
      const StenoStroke remainingStroke = stroke & ~definition.stroke;
      const char *s = definition.data;
      char *p = context.buffers[definition.priority];
      while (*s) {
        *p++ = *s++;
      }
      *p = '\0';
      if (remainingStroke.IsEmpty()) {
        return true;
      }

      OrthospellingContext remainingContext = context;
      remainingContext.buffers[definition.priority] = p;
      if (ConvertToText(remainingStroke, remainingContext)) {
        return true;
      }
    }
  }
  return false;
}

//---------------------------------------------------------------------------
