//---------------------------------------------------------------------------

#include "orthospelling_data.h"
#include "../writer.h"

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

bool OrthospellingData::ResolveStroke(StenoStroke stroke, Context context,
                                      size_t startingIndex) const {
  for (size_t i = startingIndex; i < letters.count; ++i) {
    const Letter &letter = letters[i];
    if ((letter.stroke & stroke) == letter.stroke) {
      Context remainingContext = context;
      remainingContext.Add(&letter);
      const StenoStroke remainingStroke = stroke & ~letter.stroke;
      if (remainingStroke.IsEmpty()) {
        return true;
      }
      if (ResolveStroke(remainingStroke, remainingContext, i + 1)) {
        return true;
      }
    }
  }
  return false;
}

uint32_t OrthospellingData::Context::FindLowestOrder() const {
  const Letter **letters = this->letters;
  uint32_t result = (*letters)->order;
  for (++letters; *letters; ++letters) {
    if ((*letters)->order < result) {
      result = (*letters)->order;
    }
  }
  return result;
}

void OrthospellingData::Context::WriteToBuffer(BufferWriter &writer) {
  while (!IsEmpty()) {
    const uint32_t order = FindLowestOrder();

    const Letter **output = letters;
    for (const Letter **input = letters; *input; ++input) {
      if ((*input)->order == order) {
        writer.WriteString((*input)->data);
      } else {
        *output++ = *input;
      }
    }
    *output = nullptr;
  }
}

//---------------------------------------------------------------------------
