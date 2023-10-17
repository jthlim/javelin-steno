//---------------------------------------------------------------------------

#pragma once
#include "list.h"
#include "pool_allocate.h"
#include <assert.h>
#include <string.h>

//---------------------------------------------------------------------------

constexpr size_t PATTERN_COMPONENT_BLOCK_SIZE = 1024;

//---------------------------------------------------------------------------

class PatternComponent;
class PatternQuickReject;

//---------------------------------------------------------------------------

struct PatternContext {
  const char *start;
  const char **captureList;
};

struct PatternRangeContext {
  List<PatternComponent *> visited;
  bool hasAnchor = false;
  size_t minimumLength = 0;
  size_t maximumLength = 0;

  bool HasMaximumLength() const { return maximumLength != (size_t)-1; }
  void SetUnlimitedMaximumLength() { maximumLength = (size_t)-1; }

  void AddLength(size_t i) {
    minimumLength += i;
    if (HasMaximumLength()) {
      maximumLength += i;
    }
  }
};

//---------------------------------------------------------------------------

class PatternComponent
    : public PoolAllocate<PatternComponent, PATTERN_COMPONENT_BLOCK_SIZE> {
public:
  virtual bool Match(const char *p, PatternContext &context) const = 0;
  virtual bool IsEpsilon() const { return false; }

  virtual void RemoveEpsilon();
  virtual void UpdateQuickReject(PatternQuickReject &quickReject) const;
  virtual void UpdateRanges(PatternRangeContext &context);

  static void *operator new(size_t size);
  static void operator delete(void *p) {}

protected:
  bool CallNext(const char *p, PatternContext &context) const;
  const PatternComponent *GetNext() const { return next; }

  void UpdateRangesForSingleByte(PatternRangeContext &context);

private:
  PatternComponent *next = nullptr;

  friend class Pattern;
  friend class BranchPatternComponent;
  friend class AlternatePatternComponent;
};

class EpsilonPatternComponent : public PatternComponent {
public:
  virtual bool Match(const char *p, PatternContext &context) const;
  virtual bool IsEpsilon() const { return true; }
};

class AnyPatternComponent : public PatternComponent {
public:
  virtual bool Match(const char *p, PatternContext &context) const;

  virtual void UpdateRanges(PatternRangeContext &context);
};

class AnyStarPatternComponent : public PatternComponent {
public:
  virtual bool Match(const char *p, PatternContext &context) const;

  virtual void UpdateRanges(PatternRangeContext &context);

private:
  size_t minimumMatchLength = 0;
  size_t maximumMatchLength = 0;
};

class BackReferencePatternComponent : public PatternComponent {
public:
  BackReferencePatternComponent(int index) : index(index) {}

  virtual bool Match(const char *p, PatternContext &context) const;

  virtual void UpdateRanges(PatternRangeContext &context);

private:
  int index;
};

class CharacterSetComponent : public PatternComponent {
public:
  virtual bool Match(const char *p, PatternContext &context) const;

  virtual void UpdateRanges(PatternRangeContext &context);

private:
  uint8_t mask[16] = {};

  void SetBit(int index) {
    assert(index < 128);
    int offset = index / 8;
    int bit = 1 << (index & 7);
    mask[offset] |= bit;
  }

  friend class Pattern;
};

class BranchPatternComponent : public PatternComponent {
public:
  BranchPatternComponent(PatternComponent *branch) : branch(branch) {}
  virtual bool Match(const char *p, PatternContext &context) const;
  virtual void RemoveEpsilon() final;
  virtual void UpdateRanges(PatternRangeContext &context);

private:
  PatternComponent *branch;
  bool processed = false;
};

class StartOfLinePatternComponent : public PatternComponent {
public:
  virtual bool Match(const char *p, PatternContext &context) const;
};

class EndOfLinePatternComponent : public PatternComponent {
public:
  virtual bool Match(const char *p, PatternContext &context) const;

  virtual void UpdateRanges(PatternRangeContext &context);
};

class CapturePatternComponent : public PatternComponent {
public:
  CapturePatternComponent(size_t index) : index(index) {}

  virtual bool Match(const char *p, PatternContext &context) const final;

private:
  size_t index;
};

class BytePatternComponent : public PatternComponent {
public:
  BytePatternComponent(uint8_t byte) : byte(byte) {}

  virtual bool Match(const char *p, PatternContext &context) const final;

  virtual void UpdateQuickReject(PatternQuickReject &quickReject) const;
  virtual void UpdateRanges(PatternRangeContext &context);

private:
  uint8_t byte;
};

class LiteralPatternComponent : public PatternComponent {
public:
  LiteralPatternComponent(const char *text, size_t length) {
    char *mutableText = (char *)this->text;
    mutableText[length] = '\0';
    memcpy(mutableText, text, length);
  }

  virtual bool Match(const char *p, PatternContext &context) const final;

  virtual void UpdateQuickReject(PatternQuickReject &quickReject) const;
  virtual void UpdateRanges(PatternRangeContext &context);

  static void *operator new(size_t size, size_t textLength) {
    return PatternComponent::operator new((size + textLength + sizeof(size_t)) &
                                          -sizeof(size_t));
  }

private:
  const char text[];
};

class ContainerPatternComponent : public PatternComponent {
public:
  ContainerPatternComponent() : componentCount(0), components(nullptr) {}
  ContainerPatternComponent(PatternComponent *initialComponent);

  void Add(PatternComponent *component);
  virtual void RemoveEpsilon() final;

protected:
  size_t componentCount;
  PatternComponent **components;
};

class AlternatePatternComponent : public ContainerPatternComponent {
public:
  AlternatePatternComponent(PatternComponent *initialComponent)
      : ContainerPatternComponent(initialComponent) {}

  virtual bool Match(const char *p, PatternContext &context) const final;

  virtual void UpdateRanges(PatternRangeContext &context);
};

//---------------------------------------------------------------------------
