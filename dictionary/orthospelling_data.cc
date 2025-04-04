//---------------------------------------------------------------------------

#include "orthospelling_data.h"
#include "../writer.h"
#include <stddef.h>

//---------------------------------------------------------------------------

const OrthospellingData::Starter *
OrthospellingData::GetStarterDefinition(StenoStroke stroke) const {
  for (const Starter &starter : starters) {
    if (starter.IsMatch(stroke)) {
      return &starter;
    }
  }
  return nullptr;
}

const bool OrthospellingData::IsExit(StenoStroke stroke) const {
  for (const Exit &exit : exits) {
    if (exit.IsMatch(stroke)) {
      return true;
    }
  }
  return false;
}

bool OrthospellingData::ResolveStroke(StenoStroke stroke, Context context,
                                      size_t startingIndex) const {
  for (size_t i = startingIndex; i < letters.count; ++i) {
    const Letter &letter = letters[i];
    if (!letter.IsMatch(stroke)) {
      continue;
    }

    context.Add(&letter);
    if (i + 1 < letters.count &&
        letter.activation == letters[i + 1].activation) {
      continue;
    }

    stroke &= ~letter.activation.stroke;

    if (stroke.IsEmpty()) {
      context.End();
      return true;
    }
  }
  return false;
}

size_t OrthospellingData::Context::GetCount() const {
  size_t count = 0;
  const Letter **letters = this->letters;
  while (*letters) {
    ++letters;
    ++count;
  }
  return count;
}

static int LetterComparator(const OrthospellingData::Letter **a,
                            const OrthospellingData::Letter **b) {
  if ((*a)->order != (*b)->order) {
    return (*a)->order - (*b)->order;
  }
  return int(intptr_t(a) - intptr_t(b));
}

void OrthospellingData::Context::WriteToBuffer(BufferWriter &writer) {
  const size_t count = GetCount();
  qsort(letters, count, sizeof(*letters),
        (int (*)(const void *, const void *)) & LetterComparator);
  for (const Letter **letter = letters; *letter; ++letter) {
    writer.WriteString((*letter)->data);
  }
}

//---------------------------------------------------------------------------
