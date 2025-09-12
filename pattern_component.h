//---------------------------------------------------------------------------

#pragma once
#include "pool_allocate.h"
#include <assert.h>
#include <string.h>

//---------------------------------------------------------------------------

constexpr size_t PATTERN_COMPONENT_BLOCK_SIZE = 1024;

//---------------------------------------------------------------------------

class PatternQuickReject;

//---------------------------------------------------------------------------

struct PatternContext {
  const char *start;
  const char **captures;
};

//---------------------------------------------------------------------------

#if JAVELIN_USE_PATTERN_JIT
class PatternComponent;
class PatternJitContext {
public:
  PatternJitContext();
  ~PatternJitContext();

  bool StartComponent(const PatternComponent *component);
  void AddCode(const void *data, size_t length);
  void PatchBranch(size_t offset);
  void PatchImm16(size_t offset, uint32_t value);
  void Branch(size_t target);
  void BneFail();

  bool (*Build())(const char *, const char **, const char *);
  size_t GetOffset() const { return count; }

private:
  uint8_t *buffer;
  size_t count;
  size_t capacity;
};

#define JIT_COMPONENT_METHOD                                                   \
  void Compile(PatternJitContext &context) const final;
#else
#define JIT_COMPONENT_METHOD
#endif

//---------------------------------------------------------------------------

struct PatternRecurseContext {
  PatternRecurseContext() : reference(++currentReference) {}
  PatternRecurseContext(size_t reference) : reference(reference) {}

  size_t reference;

  void Reset() { reference = ++currentReference; }

  static size_t currentReference;

  bool operator==(const PatternRecurseContext &other) const {
    return reference == other.reference;
  }
};

//---------------------------------------------------------------------------

class PatternComponent
    : public PoolAllocate<PatternComponent, PATTERN_COMPONENT_BLOCK_SIZE> {
#if JAVELIN_USE_PATTERN_JIT
private:
  mutable size_t jitOffset = 0;
#endif

public:
  constexpr PatternComponent();

  virtual bool Match(const char *p, PatternContext &context) const = 0;
  virtual bool IsEpsilon() const { return false; }

  virtual void GenerateMetrics(const PatternRecurseContext &context);

  virtual void RemoveEpsilon();
  virtual void UpdateQuickReject(PatternQuickReject &quickReject) const;

  virtual bool HasEndAnchor(const PatternRecurseContext &context) const;
  virtual size_t GetMinimumLength(const PatternRecurseContext &context) const;
  virtual size_t GetMaximumLength(const PatternRecurseContext &context) const;

#if JAVELIN_USE_PATTERN_JIT
  virtual void Compile(PatternJitContext &context) const = 0;
#endif

  static void *operator new(size_t size);
  static void operator delete(void *p) {}

  static const size_t INFINITE_LENGTH = size_t(-1);

protected:
  bool CallNext(const char *p, PatternContext &context) const;
  const PatternComponent *GetNext() const { return next; }

private:
  PatternComponent *next; // Initialized to SuccessComponent::instance

  friend class Pattern;
  friend class BranchPatternComponent;
  friend class PatternJitContext;
  friend class AlternatePatternComponent;
};

class SingleBytePatternComponent : public PatternComponent {
private:
  using super = PatternComponent;

public:
  virtual size_t GetMinimumLength(const PatternRecurseContext &context) const;
  virtual size_t GetMaximumLength(const PatternRecurseContext &context) const;
};

class SuccessPatternComponent final : public PatternComponent {
public:
  virtual bool Match(const char *p, PatternContext &context) const {
    return true;
  }
  virtual void RemoveEpsilon() {}
  virtual void UpdateQuickReject(PatternQuickReject &quickReject) const {}

  void GenerateMetrics(const PatternRecurseContext &context) {}

  bool HasEndAnchor(const PatternRecurseContext &context) const {
    return false;
  }
  size_t GetMinimumLength(const PatternRecurseContext &context) const {
    return 0;
  }
  size_t GetMaximumLength(const PatternRecurseContext &context) const {
    return 0;
  }

  JIT_COMPONENT_METHOD

  static SuccessPatternComponent instance;
};

inline constexpr PatternComponent::PatternComponent()
    : next(&SuccessPatternComponent::instance) {}

class EpsilonPatternComponent : public PatternComponent {
private:
  using super = PatternComponent;

public:
  virtual bool Match(const char *p, PatternContext &context) const;
  virtual bool IsEpsilon() const { return true; }

  virtual void GenerateMetrics(const PatternRecurseContext &context);

  virtual bool HasEndAnchor(const PatternRecurseContext &context) const;
  virtual size_t GetMinimumLength(const PatternRecurseContext &context) const;
  virtual size_t GetMaximumLength(const PatternRecurseContext &context) const;

  JIT_COMPONENT_METHOD

private:
  union RecurseData {
    bool hasEndAnchor;
    size_t length;
  };

  mutable PatternRecurseContext generateMetricsContext = 0;
  mutable PatternRecurseContext recurseContext = 0;
  mutable RecurseData recurseData;
};

class AnyPatternComponent : public SingleBytePatternComponent {
public:
  virtual bool Match(const char *p, PatternContext &context) const;

  JIT_COMPONENT_METHOD
};

class AnyStarPatternComponent : public PatternComponent {
private:
  using super = PatternComponent;

public:
  virtual bool Match(const char *p, PatternContext &context) const;

  virtual void GenerateMetrics(const PatternRecurseContext &context);
  virtual size_t GetMaximumLength(const PatternRecurseContext &context) const;

  JIT_COMPONENT_METHOD

private:
  bool hasProcessed = false;
  bool hasEndAnchor;
  size_t minimumLength;
  size_t maximumLength;
};

class BackReferencePatternComponent : public PatternComponent {
public:
  BackReferencePatternComponent(int index) : index(index) {}

  virtual bool Match(const char *p, PatternContext &context) const;

  JIT_COMPONENT_METHOD

private:
  int index;
};

class CharacterSetPatternComponent : public SingleBytePatternComponent {
private:
  using super = PatternComponent;

public:
  virtual bool Match(const char *p, PatternContext &context) const;

  JIT_COMPONENT_METHOD

private:
  uint8_t mask[16] = {};

  void SetBit(size_t index) {
    assert(index < 128);
    const size_t offset = index / 8;
    const size_t bit = 1 << (index & 7);
    mask[offset] |= bit;
  }

  bool IsBitSet(size_t index) const {
    const size_t offset = index / 8;
    const size_t bit = 1 << (index & 7);
    return (mask[offset] & bit) != 0;
  }

  size_t GetMinimumBitIndex() const {
    const uint8_t *p = mask;
    size_t index = 0;
    while (*p == 0) {
      ++p;
      index += 8;
    }
    return index + __builtin_ctz(*p);
  }

  // Returns the index above the top bit set.
  size_t GetMaximumBitIndex() const {
    const uint8_t *p = mask + 15;
    size_t index = 128 + 24; // 24 zero bits in a 32 bit word
    while (*p == 0) {
      --p;
      index -= 8;
    }
    return index - __builtin_clz(*p);
  }

  friend class Pattern;
};

enum class BranchType {
  BRANCH_BACK,
  NEXT_BACK,
  BRANCH_FORWARD,
  NEXT_FORWARD,
};

class BranchPatternComponent : public PatternComponent {
private:
  using super = PatternComponent;

public:
  BranchPatternComponent(PatternComponent *branch, BranchType type)
      : branch(branch), type(type) {}

  virtual bool Match(const char *p, PatternContext &context) const;
  virtual void RemoveEpsilon() final;

  virtual void GenerateMetrics(const PatternRecurseContext &context);

  virtual bool HasEndAnchor(const PatternRecurseContext &context) const;
  virtual size_t GetMinimumLength(const PatternRecurseContext &context) const;
  virtual size_t GetMaximumLength(const PatternRecurseContext &context) const;

  JIT_COMPONENT_METHOD

private:
  PatternComponent *branch;
  BranchType type;
  bool processed = false;
};

class StartOfLinePatternComponent : public PatternComponent {
public:
  virtual bool Match(const char *p, PatternContext &context) const;

  JIT_COMPONENT_METHOD
};

class EndOfLinePatternComponent : public PatternComponent {
public:
  virtual bool Match(const char *p, PatternContext &context) const;

  virtual bool HasEndAnchor(const PatternRecurseContext &context) const {
    return true;
  }

  JIT_COMPONENT_METHOD
};

class CapturePatternComponent : public PatternComponent {
public:
  CapturePatternComponent(size_t index) : index(index) {}

  virtual bool Match(const char *p, PatternContext &context) const final;

  JIT_COMPONENT_METHOD

private:
  size_t index;
};

class BytePatternComponent : public SingleBytePatternComponent {
public:
  BytePatternComponent(uint8_t byte) : byte(byte) {}

  virtual void UpdateQuickReject(PatternQuickReject &quickReject) const;

  virtual bool Match(const char *p, PatternContext &context) const final;

  JIT_COMPONENT_METHOD

#if JAVELIN_USE_PATTERN_JIT
  static void CompileByteCheck(PatternJitContext &context, uint8_t byte);
#endif

private:
  uint8_t byte;
};

class LiteralPatternComponent : public PatternComponent {
private:
  using super = PatternComponent;

public:
  LiteralPatternComponent(const char *text, size_t length) {
    char *mutableText = (char *)this->text;
    mutableText[length] = '\0';
    memcpy(mutableText, text, length);
  }

  virtual void UpdateQuickReject(PatternQuickReject &quickReject) const;

  virtual bool Match(const char *p, PatternContext &context) const final;

  virtual size_t GetMinimumLength(const PatternRecurseContext &context) const;
  virtual size_t GetMaximumLength(const PatternRecurseContext &context) const;

  JIT_COMPONENT_METHOD

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

  virtual void GenerateMetrics(const PatternRecurseContext &context);

  virtual bool HasEndAnchor(const PatternRecurseContext &context) const;
  virtual size_t GetMinimumLength(const PatternRecurseContext &context) const;
  virtual size_t GetMaximumLength(const PatternRecurseContext &context) const;

  JIT_COMPONENT_METHOD
};

//---------------------------------------------------------------------------
