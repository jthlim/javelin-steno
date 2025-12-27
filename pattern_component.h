//---------------------------------------------------------------------------

#pragma once
#include "bit_field.h"
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

#define JIT_COMPONENT_METHOD void Compile(PatternJitContext &context) const;
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
  virtual bool IsCapture() const { return false; }

  virtual void GenerateMetrics(const PatternRecurseContext &context);

  virtual void RemoveEpsilon();
  virtual void UpdateQuickReject(PatternQuickReject &quickReject) const;

  virtual bool HasEndAnchor(const PatternRecurseContext &context) const;
  virtual size_t GetMinimumLength(const PatternRecurseContext &context) const;
  virtual size_t GetMaximumLength(const PatternRecurseContext &context) const;
  virtual void MarkRequiredCaptures(const void *loopbackObject);

#if JAVELIN_USE_PATTERN_JIT
  virtual void Compile(PatternJitContext &context) const = 0;
#endif

  static void *operator new(size_t size);
  static void operator delete(void *p) {}

  static const size_t INFINITE_LENGTH = size_t(-1);

protected:
  const PatternComponent *GetNext() const { return next; }
  bool CallNext(const char *p, PatternContext &context) const;
#if JAVELIN_USE_PATTERN_JIT
  void CompileNext(PatternJitContext &context) const { next->Compile(context); }
#endif

  static void *operator new(size_t, void *p) { return p; }
  static void operator delete(void *, void *p) {}

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
  void MarkRequiredCaptures(const void *loopbackObject) {}

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
  BitField<128> mask;

  void SetBit(size_t index) { mask.Set(index); }
  bool IsBitSet(size_t index) const { return mask.IsSet(index); }
  void FlipBits() {
    mask = ~mask;
    mask.Clear(0);
  }

  size_t GetMinimumBitIndex() const { return mask.GetFirstBitIndex(); }
  size_t GetMaximumBitIndex() const { return mask.GetLastBitIndex(); }

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
  virtual void MarkRequiredCaptures(const void *loopbackObject) final;

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
private:
  using super = PatternComponent;

public:
  CapturePatternComponent(size_t index) : index(index) {}

  virtual bool IsCapture() const { return true; }

  virtual bool Match(const char *p, PatternContext &context) const;

  JIT_COMPONENT_METHOD

protected:
  size_t index;

private:
  bool isRequired = false;
  bool hasLoopback = false;
  bool alwaysStoreCapture = false;
  void MarkRequiredCaptures(const void *loopbackObject) final;
};

class AlwaysCapturePatternComponent : public CapturePatternComponent {
private:
  using super = CapturePatternComponent;

public:
  AlwaysCapturePatternComponent(const CapturePatternComponent &other)
      : super(other) {}

  virtual bool Match(const char *p, PatternContext &context) const final;

  JIT_COMPONENT_METHOD
};
static_assert(sizeof(CapturePatternComponent) ==
              sizeof(AlwaysCapturePatternComponent));

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
  LiteralPatternComponent(const char *text, size_t length);

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
