//---------------------------------------------------------------------------

#pragma once
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

//---------------------------------------------------------------------------

struct PatternContext {
  const char *start;
  const char **captureList;
};

//---------------------------------------------------------------------------

class PatternComponent {
public:
  virtual bool Match(const char *p, PatternContext &context) = 0;
  virtual bool IsEpsilon() const { return false; }

  virtual void RemoveEpsilon();

protected:
  bool CallNext(const char *p, PatternContext &context);

private:
  PatternComponent *next = nullptr;

  friend class Pattern;
  friend class BranchPatternComponent;
  friend class AlternatePatternComponent;
};

class EpsilonPatternComponent : public PatternComponent {
public:
  virtual bool Match(const char *p, PatternContext &context);
  virtual bool IsEpsilon() const { return true; }
};

class AnyPatternComponent : public PatternComponent {
public:
  virtual bool Match(const char *p, PatternContext &context);
};

class AnyStarPatternComponent : public PatternComponent {
public:
  virtual bool Match(const char *p, PatternContext &context);
};

class CharacterSetComponent : public PatternComponent {
public:
  virtual bool Match(const char *p, PatternContext &context);

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
  virtual bool Match(const char *p, PatternContext &context);
  virtual void RemoveEpsilon() final;

private:
  PatternComponent *branch;
};

class StartOfLinePatternComponent : public PatternComponent {
public:
  virtual bool Match(const char *p, PatternContext &context);
};

class EndOfLinePatternComponent : public PatternComponent {
public:
  virtual bool Match(const char *p, PatternContext &context);
};

class CapturePatternComponent : public PatternComponent {
public:
  CapturePatternComponent(size_t index) : index(index) {}

  virtual bool Match(const char *p, PatternContext &context) final;

private:
  size_t index;
};

class LiteralPatternComponent : public PatternComponent {
public:
  LiteralPatternComponent(const char *text) : text(text) {}

  virtual bool Match(const char *p, PatternContext &context) final;

private:
  const char *text;
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

  virtual bool Match(const char *p, PatternContext &context) final;
};

//---------------------------------------------------------------------------
