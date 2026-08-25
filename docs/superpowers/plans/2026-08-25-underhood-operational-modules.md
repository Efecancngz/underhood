# underhood — Açık Uçlu Operasyon Modülleri (Data Structures) Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Split `ISimulationModule` into a step-scenario interface (unchanged behavior for the 3 existing modules) and a new open-ended-operation interface, add shared `Animator`/`AnimatedList<T>` animation infrastructure, wire the launcher to render either kind, and ship 4 new modules (Stack, Queue, Singly Linked List, Circular Linked List) under a new "Data Structures" category.

**Architecture:** `ISimulationModule` becomes a thin common base (`name()`, `codeSnippet()`, `render()`, `kind()`); `IStepSimulationModule` and `IOperationalModule` each add their own contract. `ModuleRegistry::create()` keeps returning the common base; the launcher branches on `kind()` and `static_cast`s to the right derived interface — no RTTI. All 4 new modules share one `AnimatedList<int>` (in `core`) for insert/remove animation bookkeeping instead of reimplementing it 4 times.

**Tech Stack:** C++17, CMake (FetchContent), raylib, Dear ImGui (docking), rlImGui, Catch2 — unchanged from the existing project.

**Spec:** `docs/superpowers/specs/2026-08-25-underhood-operational-modules-design.md`

## Global Constraints

- C++ standard: C++17, CMake minimum 3.20 (unchanged project-wide settings).
- Modules stay `add_library(... OBJECT ...)`, never `STATIC` (self-registration globals must survive linking).
- Each module gated by its own `BUILD_MODULE_<NAME>` CMake option, default `ON`.
- Commit messages: Conventional Commits, English, imperative mood, no AI co-author trailer.
- The 3 existing modules' behavior and tests must not change — Task 1 only renames their base class.
- All 4 new modules: max 8 elements (`kMaxSize = 8`), `codeSnippet()` returns `""`, category `"Data Structures"`, registered via `ModuleRegistry::registerModule(name, factory, "Data Structures")`.
- `AnimatedList<T>::removeAt` policy: if a previous `removingEntry_` is still animating when `removeAt` is called again, it is dropped immediately (no queueing) and replaced by the new one.
- `IOperationalModule::history()` returns oldest-first (append-only); the launcher reverses it for display, `history()` itself is never reversed.

---

### Task 1: Split `ISimulationModule` into `IStepSimulationModule` / `IOperationalModule`

**Files:**
- Modify: `core/include/underhood/simulation_module.hpp`
- Modify: `modules/unique_ptr/unique_ptr_module.hpp`
- Modify: `modules/shared_ptr/shared_ptr_module.hpp`
- Modify: `modules/move_semantics/move_semantics_module.hpp`
- Modify: `modules/_template/template_module.hpp`
- Modify: `tests/test_module_registry.cpp`

**Interfaces:**
- Produces: `underhood::ModuleKind` (`Step`/`Operational`), `underhood::ISimulationModule` (now just `name()`, `codeSnippet()`, `render()`, `kind()`), `underhood::IStepSimulationModule` (adds `parameters()`, `reset()`, `step()`, `currentHighlightedLine()`), `underhood::IOperationalModule` (adds `Operation{label, takesValue}`, `operations()`, `canPerform()`, `performOperation()`, `history()`, `update()`, `clear()`).
- Consumes: nothing new (pure refactor of an existing header).

- [ ] **Step 1: Rewrite `core/include/underhood/simulation_module.hpp`**

```cpp
#pragma once

#include <string>
#include <vector>

#include "underhood/parameter.hpp"

namespace underhood {

class Canvas;  // defined in core/include/underhood/canvas.hpp

enum class ModuleKind { Step, Operational };

// Common ground between the two module kinds below. ModuleRegistry::create()
// returns this type; the launcher checks kind() to decide which derived
// interface to cast to and which panel set to draw.
class ISimulationModule {
public:
    virtual ~ISimulationModule() = default;

    virtual std::string name() const = 0;
    // Fixed code snippet shown in the launcher's Code panel. Operational
    // modules return "" -- the launcher hides the Code panel's content
    // entirely when this is empty (their state speaks for itself; see
    // docs/superpowers/specs/2026-08-25-underhood-operational-modules-design.md).
    virtual std::string codeSnippet() const = 0;
    virtual void render(Canvas& canvas) const = 0;
    virtual ModuleKind kind() const = 0;
};

// A module that walks through one fixed, pre-scripted scenario one step()
// at a time (e.g. unique_ptr, shared_ptr, move_semantics). Behavior is
// unchanged from before this split -- existing modules only need their base
// class renamed from ISimulationModule to this.
class IStepSimulationModule : public ISimulationModule {
public:
    ModuleKind kind() const override { return ModuleKind::Step; }

    virtual std::vector<Parameter> parameters() const = 0;
    virtual void reset(const std::vector<Parameter>& params) = 0;
    virtual bool step() = 0;
    virtual int currentHighlightedLine() const = 0;
};

// A module the user drives with open-ended operations (Push/Pop,
// Enqueue/Dequeue, Insert/Delete) rather than a fixed scenario -- e.g. the
// Data Structures modules. See the design spec referenced above for the
// full rationale and the animation contract (update()).
class IOperationalModule : public ISimulationModule {
public:
    ModuleKind kind() const override { return ModuleKind::Operational; }

    struct Operation {
        std::string label;  // exact button text, e.g. "Push"
        bool takesValue;    // true: launcher shows a value input next to it
    };

    virtual std::vector<Operation> operations() const = 0;
    // Whether operationLabel can currently run (empty/full guards). The
    // launcher disables the corresponding button when this is false.
    virtual bool canPerform(const std::string& operationLabel) const = 0;
    virtual void performOperation(const std::string& operationLabel, int value) = 0;
    // Append-only log, oldest first. The launcher displays it reversed
    // (newest on top) and does not reverse this vector itself.
    virtual std::vector<std::string> history() const = 0;
    // Advances any in-flight insert/remove animations. Called every frame
    // by the launcher while this module is active, regardless of user input.
    virtual void update(float deltaTime) = 0;
    // Resets to empty instantly (no animation) and clears history().
    virtual void clear() = 0;
};

}  // namespace underhood
```

- [ ] **Step 2: Change the 3 existing modules' base class**

In `modules/unique_ptr/unique_ptr_module.hpp`, `modules/shared_ptr/shared_ptr_module.hpp`, and `modules/move_semantics/move_semantics_module.hpp`, change the class declaration line from:

```cpp
class UniquePtrModule : public underhood::ISimulationModule {
```

to:

```cpp
class UniquePtrModule : public underhood::IStepSimulationModule {
```

(same substitution for `SharedPtrModule` and `MoveSemanticsModule` — only the base class name changes, nothing else in these 3 files). Do not touch the `.cpp` files; their method bodies compile unchanged since `IStepSimulationModule` requires exactly the methods they already implement.

- [ ] **Step 3: Change the template module's base class the same way**

In `modules/_template/template_module.hpp`, change:

```cpp
class TemplateModule : public underhood::ISimulationModule {
```

to:

```cpp
class TemplateModule : public underhood::IStepSimulationModule {
```

- [ ] **Step 4: Update `tests/test_module_registry.cpp`'s `FakeModule`**

Change:

```cpp
class FakeModule : public underhood::ISimulationModule {
```

to:

```cpp
class FakeModule : public underhood::IStepSimulationModule {
```

(the rest of `FakeModule`'s method bodies are unchanged — it already implements everything `IStepSimulationModule` requires).

- [ ] **Step 5: Build and run the full existing test suite**

Run:
```bash
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```
Expected: build succeeds, all 7 existing tests still pass (registry + 3 module tests + smoke test) — this task changes zero runtime behavior, only which base class each module declares.

- [ ] **Step 6: Commit**

```bash
git add core/include/underhood/simulation_module.hpp modules/unique_ptr/unique_ptr_module.hpp modules/shared_ptr/shared_ptr_module.hpp modules/move_semantics/move_semantics_module.hpp modules/_template/template_module.hpp tests/test_module_registry.cpp
git commit -m "refactor: split ISimulationModule into step and operational interfaces"
```

---

### Task 2: `Animator` time-based progress utility

**Files:**
- Create: `core/include/underhood/animation.hpp`
- Create: `core/src/animation.cpp`
- Modify: `core/CMakeLists.txt`
- Create: `tests/test_animation.cpp`
- Modify: `tests/CMakeLists.txt`

**Interfaces:**
- Consumes: nothing (standalone utility).
- Produces: `underhood::Animator` — `start()`, `update(float deltaTime)`, `progress() const -> float` (0..1, ease-out), `isAnimating() const -> bool`. Consumed by Task 3's `AnimatedList<T>`.

- [ ] **Step 1: Write the failing test — `tests/test_animation.cpp`**

```cpp
#include <catch2/catch_test_macros.hpp>

#include "underhood/animation.hpp"

TEST_CASE("Animator reports finished progress before start() is called") {
    underhood::Animator animator;
    REQUIRE(animator.progress() == 1.0f);
    REQUIRE_FALSE(animator.isAnimating());
}

TEST_CASE("Animator progresses from 0 toward 1 after start()") {
    underhood::Animator animator;
    animator.start();
    REQUIRE(animator.isAnimating());
    REQUIRE(animator.progress() == 0.0f);

    animator.update(0.15f);  // halfway through the 0.30s duration
    REQUIRE(animator.isAnimating());
    REQUIRE(animator.progress() > 0.0f);
    REQUIRE(animator.progress() < 1.0f);

    animator.update(0.20f);  // pushes elapsed past 0.30s total
    REQUIRE(animator.progress() == 1.0f);
    REQUIRE_FALSE(animator.isAnimating());
}

TEST_CASE("Animator start() restarts an already-finished animation") {
    underhood::Animator animator;
    animator.start();
    animator.update(1.0f);  // finishes it
    REQUIRE_FALSE(animator.isAnimating());

    animator.start();
    REQUIRE(animator.isAnimating());
    REQUIRE(animator.progress() == 0.0f);
}
```

Add `test_animation.cpp` to `tests/CMakeLists.txt`'s `add_executable(underhood_tests ...)` source list (append after `test_move_semantics_module.cpp`).

- [ ] **Step 2: Run it to verify it fails**

Run: `cmake --build build --parallel`
Expected: FAIL — `underhood/animation.hpp` does not exist yet.

- [ ] **Step 3: Write `core/include/underhood/animation.hpp`**

```cpp
#pragma once

namespace underhood {

// Simple ease-out time-based progress tracker used by IOperationalModule
// implementations to animate insert/remove transitions. Owns no rendering
// state itself -- callers read progress() each frame and use it to
// interpolate their own box positions/opacity.
class Animator {
public:
    void start();
    void update(float deltaTime);
    float progress() const;
    bool isAnimating() const;

private:
    static constexpr float kDurationSeconds = 0.30f;
    float elapsed_ = kDurationSeconds;  // starts "finished" until start() is called
    bool active_ = false;
};

}  // namespace underhood
```

- [ ] **Step 4: Write `core/src/animation.cpp`**

```cpp
#include "underhood/animation.hpp"

#include <algorithm>

namespace underhood {

void Animator::start() {
    elapsed_ = 0.0f;
    active_ = true;
}

void Animator::update(float deltaTime) {
    if (!active_) return;
    elapsed_ += deltaTime;
    if (elapsed_ >= kDurationSeconds) {
        elapsed_ = kDurationSeconds;
        active_ = false;
    }
}

float Animator::progress() const {
    float linear = elapsed_ / kDurationSeconds;
    linear = std::min(1.0f, std::max(0.0f, linear));
    // Ease-out (quadratic): fast start, settles smoothly toward 1.
    return 1.0f - (1.0f - linear) * (1.0f - linear);
}

bool Animator::isAnimating() const {
    return active_;
}

}  // namespace underhood
```

- [ ] **Step 5: Add `animation.cpp` to `core/CMakeLists.txt`**

In `core/CMakeLists.txt`, change:

```cmake
add_library(underhood_core STATIC
  src/module_registry.cpp
  src/canvas.cpp
  src/theme.cpp
)
```

to:

```cmake
add_library(underhood_core STATIC
  src/module_registry.cpp
  src/canvas.cpp
  src/theme.cpp
  src/animation.cpp
)
```

- [ ] **Step 6: Run tests to verify they pass**

Run:
```bash
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```
Expected: all tests pass, including the 3 new `Animator` tests.

- [ ] **Step 7: Commit**

```bash
git add core/include/underhood/animation.hpp core/src/animation.cpp core/CMakeLists.txt tests/test_animation.cpp tests/CMakeLists.txt
git commit -m "feat: add Animator time-based progress utility"
```

---

### Task 3: `AnimatedList<T>` shared entry-list template

**Files:**
- Create: `core/include/underhood/animated_list.hpp`
- Create: `tests/test_animated_list.cpp`
- Modify: `tests/CMakeLists.txt`

**Interfaces:**
- Consumes: `underhood::Animator` (Task 2).
- Produces: `underhood::AnimatedList<T>` — `insertAt(index, value)`, `removeAt(index)`, `update(deltaTime)`, `entries() const -> const std::vector<Entry>&`, `removingEntry() const -> const Entry*`, `size() const`, `empty() const`, `full(maxSize) const`. `Entry{T value; Animator insertAnim;}` (reused for both the insert-in animation on live entries and the fade-out animation on `removingEntry_` — same field, different meaning depending on which list it's read from). Consumed by Tasks 4/6/7/8 (Stack/Queue/both linked lists).

- [ ] **Step 1: Write the failing test — `tests/test_animated_list.cpp`**

```cpp
#include <catch2/catch_test_macros.hpp>

#include "underhood/animated_list.hpp"

TEST_CASE("AnimatedList insertAt appends a new animating entry") {
    underhood::AnimatedList<int> list;
    REQUIRE(list.empty());

    list.insertAt(0, 42);
    REQUIRE(list.size() == 1);
    REQUIRE(list.entries()[0].value == 42);
    REQUIRE(list.entries()[0].insertAnim.isAnimating());
}

TEST_CASE("AnimatedList insertAt respects index for front/back insertion") {
    underhood::AnimatedList<int> list;
    list.insertAt(0, 1);
    list.insertAt(1, 2);  // back
    list.insertAt(0, 0);  // front

    REQUIRE(list.size() == 3);
    REQUIRE(list.entries()[0].value == 0);
    REQUIRE(list.entries()[1].value == 1);
    REQUIRE(list.entries()[2].value == 2);
}

TEST_CASE("AnimatedList removeAt moves the entry into removingEntry until its animation finishes") {
    underhood::AnimatedList<int> list;
    list.insertAt(0, 7);
    REQUIRE(list.removingEntry() == nullptr);

    list.removeAt(0);
    REQUIRE(list.empty());
    REQUIRE(list.removingEntry() != nullptr);
    REQUIRE(list.removingEntry()->value == 7);

    list.update(1.0f);  // well past the 0.30s animation duration
    REQUIRE(list.removingEntry() == nullptr);
}

TEST_CASE("AnimatedList full() reports capacity correctly") {
    underhood::AnimatedList<int> list;
    REQUIRE_FALSE(list.full(2));
    list.insertAt(0, 1);
    REQUIRE_FALSE(list.full(2));
    list.insertAt(1, 2);
    REQUIRE(list.full(2));
}

TEST_CASE("AnimatedList removeAt while a previous removal is still animating replaces it immediately") {
    underhood::AnimatedList<int> list;
    list.insertAt(0, 1);
    list.insertAt(1, 2);

    list.removeAt(1);  // removingEntry = 2, mid-animation
    REQUIRE(list.removingEntry()->value == 2);

    list.removeAt(0);  // removingEntry_ overwritten per spec policy
    REQUIRE(list.removingEntry()->value == 1);
}
```

Add `test_animated_list.cpp` to `tests/CMakeLists.txt`'s source list (append after `test_animation.cpp`).

- [ ] **Step 2: Run it to verify it fails**

Run: `cmake --build build --parallel`
Expected: FAIL — `underhood/animated_list.hpp` does not exist yet.

- [ ] **Step 3: Write `core/include/underhood/animated_list.hpp`**

```cpp
#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include "underhood/animation.hpp"

namespace underhood {

// Generic ordered collection with per-entry insert/remove animation state,
// shared by all IOperationalModule data-structure modules (Stack, Queue,
// linked lists) so each doesn't reimplement the same bookkeeping. Insertion
// starts that entry's Animator; removal moves the entry into a single
// "removingEntry" slot with its own fade-out Animator instead of deleting
// it immediately, so render() can still draw it mid-animation.
template <typename T>
class AnimatedList {
public:
    struct Entry {
        T value;
        Animator insertAnim;
    };

    void insertAt(std::size_t index, T value) {
        Entry entry{std::move(value), Animator{}};
        entry.insertAnim.start();
        entries_.insert(entries_.begin() + static_cast<std::ptrdiff_t>(index), std::move(entry));
    }

    void removeAt(std::size_t index) {
        Entry removed = std::move(entries_[index]);
        entries_.erase(entries_.begin() + static_cast<std::ptrdiff_t>(index));
        removingEntry_ = std::move(removed);
        removingEntry_->insertAnim.start();
    }

    void update(float deltaTime) {
        for (auto& entry : entries_) {
            entry.insertAnim.update(deltaTime);
        }
        if (removingEntry_) {
            removingEntry_->insertAnim.update(deltaTime);
            if (!removingEntry_->insertAnim.isAnimating()) {
                removingEntry_.reset();
            }
        }
    }

    const std::vector<Entry>& entries() const { return entries_; }
    const Entry* removingEntry() const { return removingEntry_ ? &*removingEntry_ : nullptr; }
    std::size_t size() const { return entries_.size(); }
    bool empty() const { return entries_.empty(); }
    bool full(std::size_t maxSize) const { return entries_.size() >= maxSize; }

private:
    std::vector<Entry> entries_;
    std::optional<Entry> removingEntry_;
};

}  // namespace underhood
```

- [ ] **Step 4: Run tests to verify they pass**

Run:
```bash
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```
Expected: all tests pass, including the 5 new `AnimatedList` tests.

- [ ] **Step 5: Commit**

```bash
git add core/include/underhood/animated_list.hpp tests/test_animated_list.cpp tests/CMakeLists.txt
git commit -m "feat: add AnimatedList shared entry-list template"
```

---

### Task 4: `Stack` module

**Files:**
- Create: `modules/stack/stack_module.hpp`
- Create: `modules/stack/stack_module.cpp`
- Create: `modules/stack/CMakeLists.txt`
- Modify: `CMakeLists.txt` (root — add `BUILD_MODULE_STACK` option)
- Modify: `modules/CMakeLists.txt` (add guarded `add_subdirectory(stack)`)
- Create: `tests/test_stack_module.cpp`
- Modify: `tests/CMakeLists.txt`

**Interfaces:**
- Consumes: `underhood::IOperationalModule`, `underhood::AnimatedList<int>` (Task 3), `underhood::Canvas`, `underhood::ModuleRegistry`.
- Produces: `underhood::modules::StackModule` (registers as `"stack"`, category `"Data Structures"`), test accessors `size() const -> std::size_t`, `topValue() const -> int` (caller must check `size() > 0` first).

- [ ] **Step 1: Write the failing test — `tests/test_stack_module.cpp`**

```cpp
#include <catch2/catch_test_macros.hpp>

#include "stack_module.hpp"

TEST_CASE("StackModule starts empty") {
    underhood::modules::StackModule module;
    REQUIRE(module.size() == 0);
    REQUIRE_FALSE(module.canPerform("Pop"));
    REQUIRE(module.canPerform("Push"));
}

TEST_CASE("StackModule Push adds to the top and Pop removes it in LIFO order") {
    underhood::modules::StackModule module;
    module.performOperation("Push", 1);
    module.performOperation("Push", 2);
    module.performOperation("Push", 3);

    REQUIRE(module.size() == 3);
    REQUIRE(module.topValue() == 3);

    module.performOperation("Pop", 0);
    REQUIRE(module.size() == 2);
    REQUIRE(module.topValue() == 2);
}

TEST_CASE("StackModule records operation history") {
    underhood::modules::StackModule module;
    module.performOperation("Push", 5);
    module.performOperation("Pop", 0);

    auto history = module.history();
    REQUIRE(history.size() == 2);
    REQUIRE(history[0] == "Push 5");
    REQUIRE(history[1] == "Pop -> 5");
}

TEST_CASE("StackModule Pop on an empty stack is a no-op") {
    underhood::modules::StackModule module;
    module.performOperation("Pop", 0);
    REQUIRE(module.size() == 0);
    REQUIRE(module.history().empty());
}

TEST_CASE("StackModule refuses Push once at capacity") {
    underhood::modules::StackModule module;
    for (int i = 0; i < 8; ++i) {
        module.performOperation("Push", i);
    }
    REQUIRE(module.size() == 8);
    REQUIRE_FALSE(module.canPerform("Push"));

    module.performOperation("Push", 99);  // no-op, already full
    REQUIRE(module.size() == 8);
    REQUIRE(module.topValue() == 7);
}

TEST_CASE("StackModule clear empties the stack and history") {
    underhood::modules::StackModule module;
    module.performOperation("Push", 1);
    module.performOperation("Push", 2);
    module.clear();
    REQUIRE(module.size() == 0);
    REQUIRE(module.history().empty());
}

TEST_CASE("StackModule caps history at 10 entries, dropping the oldest") {
    underhood::modules::StackModule module;
    // 6 push+pop cycles = 12 history entries, stack size never exceeds 1
    // (capacity is a separate concern from history capping).
    for (int i = 0; i < 6; ++i) {
        module.performOperation("Push", i);
        module.performOperation("Pop", 0);
    }
    auto history = module.history();
    REQUIRE(history.size() == 10);
    REQUIRE(history.front() == "Push 1");  // "Push 0" and "Pop -> 0" fell off
    REQUIRE(history.back() == "Pop -> 5");
}
```

- [ ] **Step 2: Run it to verify it fails**

Run: `cmake --build build --parallel`
Expected: FAIL — `stack_module.hpp` does not exist yet.

- [ ] **Step 3: Write `modules/stack/stack_module.hpp`**

```cpp
#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "underhood/animated_list.hpp"
#include "underhood/canvas.hpp"
#include "underhood/simulation_module.hpp"

namespace underhood::modules {

class StackModule : public underhood::IOperationalModule {
public:
    std::string name() const override;
    std::string codeSnippet() const override;
    void render(underhood::Canvas& canvas) const override;

    std::vector<Operation> operations() const override;
    bool canPerform(const std::string& operationLabel) const override;
    void performOperation(const std::string& operationLabel, int value) override;
    std::vector<std::string> history() const override;
    void update(float deltaTime) override;
    void clear() override;

    // Test-facing accessors (not part of IOperationalModule).
    std::size_t size() const;
    int topValue() const;  // caller must check size() > 0 first

private:
    static constexpr std::size_t kMaxSize = 8;
    underhood::AnimatedList<int> entries_;
    std::vector<std::string> history_;
};

}  // namespace underhood::modules
```

- [ ] **Step 4: Write `modules/stack/stack_module.cpp`**

```cpp
#include "stack_module.hpp"

#include <memory>
#include <string>

#include "underhood/module_registry.hpp"

namespace underhood::modules {

std::string StackModule::name() const {
    return "stack";
}

std::string StackModule::codeSnippet() const {
    return "";
}

std::vector<underhood::IOperationalModule::Operation> StackModule::operations() const {
    return {{"Push", true}, {"Pop", false}};
}

bool StackModule::canPerform(const std::string& operationLabel) const {
    if (operationLabel == "Push") {
        return !entries_.full(kMaxSize);
    }
    if (operationLabel == "Pop") {
        return !entries_.empty();
    }
    return false;
}

namespace {
constexpr std::size_t kMaxHistoryEntries = 10;

void appendHistory(std::vector<std::string>& history, std::string entry) {
    history.push_back(std::move(entry));
    if (history.size() > kMaxHistoryEntries) {
        history.erase(history.begin());
    }
}
}  // namespace

void StackModule::performOperation(const std::string& operationLabel, int value) {
    if (operationLabel == "Push" && !entries_.full(kMaxSize)) {
        entries_.insertAt(entries_.size(), value);
        appendHistory(history_, "Push " + std::to_string(value));
    } else if (operationLabel == "Pop" && !entries_.empty()) {
        int poppedValue = entries_.entries().back().value;
        entries_.removeAt(entries_.size() - 1);
        appendHistory(history_, "Pop -> " + std::to_string(poppedValue));
    }
}

std::vector<std::string> StackModule::history() const {
    return history_;
}

void StackModule::update(float deltaTime) {
    entries_.update(deltaTime);
}

void StackModule::clear() {
    entries_ = underhood::AnimatedList<int>();
    history_.clear();
}

std::size_t StackModule::size() const {
    return entries_.size();
}

int StackModule::topValue() const {
    return entries_.entries().back().value;
}

void StackModule::render(underhood::Canvas& canvas) const {
    constexpr int kBoxWidth = 100;
    constexpr int kBoxHeight = 36;
    constexpr int kBoxSpacing = 6;
    constexpr int kBaseX = 170;
    constexpr int kBaseY = 230;  // bottom of the stack; grows upward

    auto drawEntry = [&](const underhood::AnimatedList<int>::Entry& entry, std::size_t stackIndex,
                          bool isTop) {
        float progress = entry.insertAnim.progress();
        int y = kBaseY - static_cast<int>(stackIndex) * (kBoxHeight + kBoxSpacing);
        // Slide in from above while inserting/removing.
        int slideOffset = static_cast<int>((1.0f - progress) * 40.0f);
        underhood::BoxState state = entry.insertAnim.isAnimating() ? underhood::BoxState::JustChanged
                                     : isTop                        ? underhood::BoxState::Owned
                                                                     : underhood::BoxState::Empty;
        canvas.drawBox(kBaseX, y - slideOffset, kBoxWidth, kBoxHeight, std::to_string(entry.value),
                        state);
    };

    const auto& entries = entries_.entries();
    for (std::size_t i = 0; i < entries.size(); ++i) {
        drawEntry(entries[i], i, i + 1 == entries.size());
    }
    if (const auto* removing = entries_.removingEntry()) {
        drawEntry(*removing, entries.size(), false);
    }
}

}  // namespace underhood::modules

namespace {
struct StackRegistrar {
    StackRegistrar() {
        underhood::ModuleRegistry::instance().registerModule(
            "stack", []() { return std::make_unique<underhood::modules::StackModule>(); },
            "Data Structures");
    }
};
const StackRegistrar stackRegistrar;
}  // namespace
```

- [ ] **Step 5: Write `modules/stack/CMakeLists.txt`**

```cmake
add_library(underhood_module_stack OBJECT stack_module.cpp)
target_include_directories(underhood_module_stack PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(underhood_module_stack PUBLIC underhood_core)
```

- [ ] **Step 6: Add the `BUILD_MODULE_STACK` option to root `CMakeLists.txt`**

Add this line after the existing `option(BUILD_MODULE_MOVE_SEMANTICS ...)` line:

```cmake
option(BUILD_MODULE_STACK "Build the stack simulation module" ON)
```

- [ ] **Step 7: Guard the module in `modules/CMakeLists.txt`**

Append below the existing `move_semantics` block:

```cmake
if(BUILD_MODULE_STACK)
  add_subdirectory(stack)
endif()
```

- [ ] **Step 8: Add the test to `tests/CMakeLists.txt`**

Add `test_stack_module.cpp` to the `add_executable(underhood_tests ...)` source list and `underhood_module_stack` to its `target_link_libraries(underhood_tests PRIVATE ...)` list.

- [ ] **Step 9: Run tests to verify they pass**

Run:
```bash
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```
Expected: all tests pass, including the 7 new `StackModule` tests.

- [ ] **Step 10: Commit**

```bash
git add modules/stack CMakeLists.txt modules/CMakeLists.txt tests/test_stack_module.cpp tests/CMakeLists.txt
git commit -m "feat: add stack simulation module"
```

---

### Task 5: Launcher — wire the `Operational` panel set

**Files:**
- Modify: `launcher/main.cpp`
- Modify: `launcher/CMakeLists.txt`

**Interfaces:**
- Consumes: `underhood::ModuleKind`, `underhood::IStepSimulationModule`, `underhood::IOperationalModule` (Task 1), `underhood::modules::StackModule` (Task 4, as the manual-verification subject — no other module needed for this task).
- Produces: nothing new for later tasks to consume — Tasks 6/7/8 add modules through the same registry/CMake pattern without touching `main.cpp` again.

This task has no automated GUI test (same as the original launcher task) — verified by building, running the existing test suite (must stay green), and a smoke-run confirming the process stays alive. A full manual click-through happens in Task 9 once all 4 modules exist.

- [ ] **Step 1: Link the Stack module into the launcher**

In `launcher/CMakeLists.txt`, add this block after the existing `BUILD_MODULE_MOVE_SEMANTICS` guard:

```cmake
if(BUILD_MODULE_STACK)
  target_link_libraries(underhood PRIVATE underhood_module_stack)
endif()
```

- [ ] **Step 2: Replace `DrawCodePanel` to branch on `kind()`**

In `launcher/main.cpp`, replace the existing `DrawCodePanel` function:

```cpp
void DrawCodePanel(const underhood::ISimulationModule* module) {
    ImGui::Begin(kCodePanel);
    if (module == nullptr) {
        ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_TextDisabled],
                            "Select a simulation from the left to see its code.");
    } else {
        auto lines = splitLines(module->codeSnippet());
        for (std::size_t i = 0; i < lines.size(); ++i) {
            int lineNumber = static_cast<int>(i) + 1;
            if (lineNumber == module->currentHighlightedLine()) {
                // Amber, matching Canvas's JustChanged box color -- the same
                // color means "this is what just happened" in both panels.
                ImGui::TextColored(ImVec4(0.90f, 0.62f, 0.0f, 1.0f), "%s", lines[i].c_str());
            } else {
                ImGui::Text("%s", lines[i].c_str());
            }
        }
    }
    ImGui::End();
}
```

with:

```cpp
void DrawCodePanel(const underhood::ISimulationModule* module) {
    ImGui::Begin(kCodePanel);
    if (module == nullptr) {
        ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_TextDisabled],
                            "Select a simulation from the left to see its code.");
    } else if (module->kind() == underhood::ModuleKind::Step) {
        const auto* stepModule = static_cast<const underhood::IStepSimulationModule*>(module);
        auto lines = splitLines(stepModule->codeSnippet());
        for (std::size_t i = 0; i < lines.size(); ++i) {
            int lineNumber = static_cast<int>(i) + 1;
            if (lineNumber == stepModule->currentHighlightedLine()) {
                // Amber, matching Canvas's JustChanged box color -- the same
                // color means "this is what just happened" in both panels.
                ImGui::TextColored(ImVec4(0.90f, 0.62f, 0.0f, 1.0f), "%s", lines[i].c_str());
            } else {
                ImGui::Text("%s", lines[i].c_str());
            }
        }
    }
    // Operational modules (empty codeSnippet by contract) render nothing
    // here -- their Controls/Visualization panels carry the content.
    ImGui::End();
}
```

- [ ] **Step 3: Replace `DrawControlsPanel` with kind-specific helpers**

Replace the existing `DrawControlsPanel` function:

```cpp
void DrawControlsPanel(std::vector<underhood::Parameter>& params, bool hasActiveModule,
                        bool& resetRequested, bool& stepRequested) {
    ImGui::Begin(kControlsPanel);
    if (!hasActiveModule) {
        ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_TextDisabled], "No simulation selected.");
        ImGui::End();
        return;
    }

    for (auto& param : params) {
        ImGui::InputInt(param.name.c_str(), &param.value);
        if (param.value < param.minValue) param.value = param.minValue;
        if (param.value > param.maxValue) param.value = param.maxValue;
    }
    ImGui::Spacing();
    if (ImGui::Button("Reset", ImVec2(-1, 0))) {
        resetRequested = true;
    }
    if (ImGui::Button("Next Step", ImVec2(-1, 0))) {
        stepRequested = true;
    }
    ImGui::End();
}
```

with three functions:

```cpp
void DrawStepControls(std::vector<underhood::Parameter>& params, bool& resetRequested,
                       bool& stepRequested) {
    for (auto& param : params) {
        ImGui::InputInt(param.name.c_str(), &param.value);
        if (param.value < param.minValue) param.value = param.minValue;
        if (param.value > param.maxValue) param.value = param.maxValue;
    }
    ImGui::Spacing();
    if (ImGui::Button("Reset", ImVec2(-1, 0))) {
        resetRequested = true;
    }
    if (ImGui::Button("Next Step", ImVec2(-1, 0))) {
        stepRequested = true;
    }
}

// takesValue is currently informational only -- v1 shows one shared value
// input regardless of which operation the user is about to click, rather
// than hiding/graying it out per-button. Operations that ignore the value
// (e.g. "Pop") simply don't read it in performOperation().
void DrawOperationalControls(underhood::IOperationalModule* module, int& pendingValue) {
    ImGui::InputInt("Value", &pendingValue);
    ImGui::Spacing();

    for (const auto& operation : module->operations()) {
        bool enabled = module->canPerform(operation.label);
        ImGui::BeginDisabled(!enabled);
        if (ImGui::Button(operation.label.c_str(), ImVec2(-1, 0))) {
            module->performOperation(operation.label, pendingValue);
        }
        ImGui::EndDisabled();
    }

    ImGui::Spacing();
    if (ImGui::Button("Clear", ImVec2(-1, 0))) {
        module->clear();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_TextDisabled], "HISTORY");
    ImGui::BeginChild("##history", ImVec2(0, 160), true);
    auto history = module->history();
    for (auto it = history.rbegin(); it != history.rend(); ++it) {
        ImGui::TextUnformatted(it->c_str());
    }
    ImGui::EndChild();
}

void DrawControlsPanel(underhood::ISimulationModule* module, std::vector<underhood::Parameter>& stepParams,
                        int& pendingValue, bool& resetRequested, bool& stepRequested) {
    ImGui::Begin(kControlsPanel);
    if (module == nullptr) {
        ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_TextDisabled], "No simulation selected.");
    } else if (module->kind() == underhood::ModuleKind::Step) {
        DrawStepControls(stepParams, resetRequested, stepRequested);
    } else {
        DrawOperationalControls(static_cast<underhood::IOperationalModule*>(module), pendingValue);
    }
    ImGui::End();
}
```

- [ ] **Step 4: Update `main()`'s state and per-frame logic**

In `main()`, add a new local next to the existing `activeParams` declaration:

```cpp
    std::unique_ptr<underhood::ISimulationModule> activeModule;
    std::string activeModuleName;
    std::vector<underhood::Parameter> activeParams;
```

becomes:

```cpp
    std::unique_ptr<underhood::ISimulationModule> activeModule;
    std::string activeModuleName;
    std::vector<underhood::Parameter> activeParams;
    int pendingValue = 0;
```

Replace the module-switch block:

```cpp
        if (moduleClicked) {
            activeModule = underhood::ModuleRegistry::instance().create(clickedModuleName);
            activeModuleName = clickedModuleName;
            activeParams = activeModule->parameters();
            activeModule->reset(activeParams);
        }
```

with:

```cpp
        if (moduleClicked) {
            activeModule = underhood::ModuleRegistry::instance().create(clickedModuleName);
            activeModuleName = clickedModuleName;
            pendingValue = 0;
            if (activeModule->kind() == underhood::ModuleKind::Step) {
                auto* stepModule = static_cast<underhood::IStepSimulationModule*>(activeModule.get());
                activeParams = stepModule->parameters();
                stepModule->reset(activeParams);
            } else {
                activeParams.clear();
            }
        }
```

Replace the Controls-panel call and its follow-up Reset/Next Step handling:

```cpp
        DrawCodePanel(activeModule.get());

        bool resetRequested = false;
        bool stepRequested = false;
        DrawControlsPanel(activeParams, activeModule != nullptr, resetRequested, stepRequested);
        if (resetRequested && activeModule) {
            activeModule->reset(activeParams);
        }
        if (stepRequested && activeModule) {
            activeModule->step();
        }

        DrawVisualizationPanel(activeModule.get(), canvas, palette, canvasTexture);
```

with:

```cpp
        DrawCodePanel(activeModule.get());

        bool resetRequested = false;
        bool stepRequested = false;
        DrawControlsPanel(activeModule.get(), activeParams, pendingValue, resetRequested, stepRequested);
        if (activeModule && activeModule->kind() == underhood::ModuleKind::Step) {
            auto* stepModule = static_cast<underhood::IStepSimulationModule*>(activeModule.get());
            if (resetRequested) {
                stepModule->reset(activeParams);
            }
            if (stepRequested) {
                stepModule->step();
            }
        }
        if (activeModule && activeModule->kind() == underhood::ModuleKind::Operational) {
            static_cast<underhood::IOperationalModule*>(activeModule.get())->update(GetFrameTime());
        }

        DrawVisualizationPanel(activeModule.get(), canvas, palette, canvasTexture);
```

- [ ] **Step 5: Build**

Run:
```bash
cmake -S . -B build
cmake --build build --parallel
```
Expected: build succeeds, producing `build/launcher/Debug/underhood.exe` (or platform equivalent) linked against `underhood_module_stack`.

- [ ] **Step 6: Run the existing test suite**

Run: `ctest --test-dir build --output-on-failure`
Expected: all existing tests still pass (this task touches no module logic, only the launcher).

- [ ] **Step 7: Smoke-test the launcher**

Launch the built executable, wait ~5 seconds, confirm the process is still running (no crash), then terminate it. Also confirm in the report that a "Data Structures" section with a "stack" entry appears in the sidebar (from `StackModule`'s category) — this can be confirmed by reading the built executable ran without error; the actual visual click-through happens in Task 9 with the human present.

- [ ] **Step 8: Commit**

```bash
git add launcher/main.cpp launcher/CMakeLists.txt
git commit -m "feat: wire launcher Controls/Code panels for operational modules"
```

---

### Task 6: `Queue` module

**Files:**
- Create: `modules/queue/queue_module.hpp`
- Create: `modules/queue/queue_module.cpp`
- Create: `modules/queue/CMakeLists.txt`
- Modify: `CMakeLists.txt` (root — add `BUILD_MODULE_QUEUE` option)
- Modify: `modules/CMakeLists.txt` (guarded `add_subdirectory(queue)`)
- Modify: `launcher/CMakeLists.txt` (guarded link)
- Create: `tests/test_queue_module.cpp`
- Modify: `tests/CMakeLists.txt`

**Interfaces:**
- Consumes: `underhood::IOperationalModule`, `underhood::AnimatedList<int>` (Task 3).
- Produces: `underhood::modules::QueueModule` (registers as `"queue"`, category `"Data Structures"`), test accessors `size() const -> std::size_t`, `frontValue() const -> int` (caller must check `size() > 0` first).

- [ ] **Step 1: Write the failing test — `tests/test_queue_module.cpp`**

```cpp
#include <catch2/catch_test_macros.hpp>

#include "queue_module.hpp"

TEST_CASE("QueueModule starts empty") {
    underhood::modules::QueueModule module;
    REQUIRE(module.size() == 0);
    REQUIRE_FALSE(module.canPerform("Dequeue"));
    REQUIRE(module.canPerform("Enqueue"));
}

TEST_CASE("QueueModule Enqueue adds to the back and Dequeue removes from the front (FIFO)") {
    underhood::modules::QueueModule module;
    module.performOperation("Enqueue", 1);
    module.performOperation("Enqueue", 2);
    module.performOperation("Enqueue", 3);

    REQUIRE(module.size() == 3);
    REQUIRE(module.frontValue() == 1);

    module.performOperation("Dequeue", 0);
    REQUIRE(module.size() == 2);
    REQUIRE(module.frontValue() == 2);
}

TEST_CASE("QueueModule records operation history") {
    underhood::modules::QueueModule module;
    module.performOperation("Enqueue", 5);
    module.performOperation("Dequeue", 0);

    auto history = module.history();
    REQUIRE(history.size() == 2);
    REQUIRE(history[0] == "Enqueue 5");
    REQUIRE(history[1] == "Dequeue -> 5");
}

TEST_CASE("QueueModule Dequeue on an empty queue is a no-op") {
    underhood::modules::QueueModule module;
    module.performOperation("Dequeue", 0);
    REQUIRE(module.size() == 0);
    REQUIRE(module.history().empty());
}

TEST_CASE("QueueModule refuses Enqueue once at capacity") {
    underhood::modules::QueueModule module;
    for (int i = 0; i < 8; ++i) {
        module.performOperation("Enqueue", i);
    }
    REQUIRE(module.size() == 8);
    REQUIRE_FALSE(module.canPerform("Enqueue"));

    module.performOperation("Enqueue", 99);  // no-op, already full
    REQUIRE(module.size() == 8);
    REQUIRE(module.frontValue() == 0);
}

TEST_CASE("QueueModule clear empties the queue and history") {
    underhood::modules::QueueModule module;
    module.performOperation("Enqueue", 1);
    module.performOperation("Enqueue", 2);
    module.clear();
    REQUIRE(module.size() == 0);
    REQUIRE(module.history().empty());
}

TEST_CASE("QueueModule caps history at 10 entries, dropping the oldest") {
    underhood::modules::QueueModule module;
    // 6 enqueue+dequeue cycles = 12 history entries, size never exceeds 1.
    for (int i = 0; i < 6; ++i) {
        module.performOperation("Enqueue", i);
        module.performOperation("Dequeue", 0);
    }
    auto history = module.history();
    REQUIRE(history.size() == 10);
    REQUIRE(history.front() == "Enqueue 1");  // "Enqueue 0" and "Dequeue -> 0" fell off
    REQUIRE(history.back() == "Dequeue -> 5");
}
```

- [ ] **Step 2: Run it to verify it fails**

Run: `cmake --build build --parallel`
Expected: FAIL — `queue_module.hpp` does not exist yet.

- [ ] **Step 3: Write `modules/queue/queue_module.hpp`**

```cpp
#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "underhood/animated_list.hpp"
#include "underhood/canvas.hpp"
#include "underhood/simulation_module.hpp"

namespace underhood::modules {

class QueueModule : public underhood::IOperationalModule {
public:
    std::string name() const override;
    std::string codeSnippet() const override;
    void render(underhood::Canvas& canvas) const override;

    std::vector<Operation> operations() const override;
    bool canPerform(const std::string& operationLabel) const override;
    void performOperation(const std::string& operationLabel, int value) override;
    std::vector<std::string> history() const override;
    void update(float deltaTime) override;
    void clear() override;

    // Test-facing accessors (not part of IOperationalModule).
    std::size_t size() const;
    int frontValue() const;  // caller must check size() > 0 first

private:
    static constexpr std::size_t kMaxSize = 8;
    underhood::AnimatedList<int> entries_;
    std::vector<std::string> history_;
};

}  // namespace underhood::modules
```

- [ ] **Step 4: Write `modules/queue/queue_module.cpp`**

```cpp
#include "queue_module.hpp"

#include <memory>
#include <string>

#include "underhood/module_registry.hpp"

namespace underhood::modules {

std::string QueueModule::name() const {
    return "queue";
}

std::string QueueModule::codeSnippet() const {
    return "";
}

std::vector<underhood::IOperationalModule::Operation> QueueModule::operations() const {
    return {{"Enqueue", true}, {"Dequeue", false}};
}

bool QueueModule::canPerform(const std::string& operationLabel) const {
    if (operationLabel == "Enqueue") {
        return !entries_.full(kMaxSize);
    }
    if (operationLabel == "Dequeue") {
        return !entries_.empty();
    }
    return false;
}

namespace {
constexpr std::size_t kMaxHistoryEntries = 10;

void appendHistory(std::vector<std::string>& history, std::string entry) {
    history.push_back(std::move(entry));
    if (history.size() > kMaxHistoryEntries) {
        history.erase(history.begin());
    }
}
}  // namespace

void QueueModule::performOperation(const std::string& operationLabel, int value) {
    if (operationLabel == "Enqueue" && !entries_.full(kMaxSize)) {
        entries_.insertAt(entries_.size(), value);
        appendHistory(history_, "Enqueue " + std::to_string(value));
    } else if (operationLabel == "Dequeue" && !entries_.empty()) {
        int frontVal = entries_.entries().front().value;
        entries_.removeAt(0);
        appendHistory(history_, "Dequeue -> " + std::to_string(frontVal));
    }
}

std::vector<std::string> QueueModule::history() const {
    return history_;
}

void QueueModule::update(float deltaTime) {
    entries_.update(deltaTime);
}

void QueueModule::clear() {
    entries_ = underhood::AnimatedList<int>();
    history_.clear();
}

std::size_t QueueModule::size() const {
    return entries_.size();
}

int QueueModule::frontValue() const {
    return entries_.entries().front().value;
}

void QueueModule::render(underhood::Canvas& canvas) const {
    constexpr int kAvailableWidth = 400;
    constexpr int kBaseX = 20;
    constexpr int kBoxHeight = 60;
    constexpr int kBaseY = 100;

    const auto& entries = entries_.entries();
    std::size_t slots = entries.size() + 1;  // +1 keeps room even when nearly full
    int cellWidth = kAvailableWidth / static_cast<int>(slots);
    int boxWidth = cellWidth > 20 ? cellWidth - 10 : cellWidth;

    auto drawEntry = [&](const underhood::AnimatedList<int>::Entry& entry, std::size_t visualIndex,
                          bool isFront) {
        float progress = entry.insertAnim.progress();
        int x = kBaseX + static_cast<int>(visualIndex) * cellWidth;
        int dropOffset = static_cast<int>((1.0f - progress) * 30.0f);
        underhood::BoxState state = entry.insertAnim.isAnimating() ? underhood::BoxState::JustChanged
                                     : isFront                     ? underhood::BoxState::Owned
                                                                     : underhood::BoxState::Empty;
        canvas.drawBox(x, kBaseY - dropOffset, boxWidth, kBoxHeight, std::to_string(entry.value), state);
    };

    for (std::size_t i = 0; i < entries.size(); ++i) {
        drawEntry(entries[i], i, i == 0);
    }
    if (const auto* removing = entries_.removingEntry()) {
        drawEntry(*removing, 0, true);  // dequeue always removes index 0
    }

    canvas.drawText("front", kBaseX, kBaseY + kBoxHeight + 10);
    if (!entries.empty()) {
        int backX = kBaseX + static_cast<int>(entries.size() - 1) * cellWidth;
        canvas.drawText("back", backX, kBaseY + kBoxHeight + 10);
    }
}

}  // namespace underhood::modules

namespace {
struct QueueRegistrar {
    QueueRegistrar() {
        underhood::ModuleRegistry::instance().registerModule(
            "queue", []() { return std::make_unique<underhood::modules::QueueModule>(); },
            "Data Structures");
    }
};
const QueueRegistrar queueRegistrar;
}  // namespace
```

- [ ] **Step 5: Write `modules/queue/CMakeLists.txt`**

```cmake
add_library(underhood_module_queue OBJECT queue_module.cpp)
target_include_directories(underhood_module_queue PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(underhood_module_queue PUBLIC underhood_core)
```

- [ ] **Step 6: Add the `BUILD_MODULE_QUEUE` option to root `CMakeLists.txt`**

Add after the `BUILD_MODULE_STACK` line:

```cmake
option(BUILD_MODULE_QUEUE "Build the queue simulation module" ON)
```

- [ ] **Step 7: Guard the module in `modules/CMakeLists.txt`**

Append below the `stack` block:

```cmake
if(BUILD_MODULE_QUEUE)
  add_subdirectory(queue)
endif()
```

- [ ] **Step 8: Link it into the launcher**

In `launcher/CMakeLists.txt`, append below the `BUILD_MODULE_STACK` guard added in Task 5:

```cmake
if(BUILD_MODULE_QUEUE)
  target_link_libraries(underhood PRIVATE underhood_module_queue)
endif()
```

- [ ] **Step 9: Add the test to `tests/CMakeLists.txt`**

Add `test_queue_module.cpp` to the source list and `underhood_module_queue` to the link list.

- [ ] **Step 10: Run tests to verify they pass**

Run:
```bash
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```
Expected: all tests pass, including the 7 new `QueueModule` tests.

- [ ] **Step 11: Commit**

```bash
git add modules/queue CMakeLists.txt modules/CMakeLists.txt launcher/CMakeLists.txt tests/test_queue_module.cpp tests/CMakeLists.txt
git commit -m "feat: add queue simulation module"
```

---

### Task 7: `Singly Linked List` module

**Files:**
- Create: `modules/singly_linked_list/singly_linked_list_module.hpp`
- Create: `modules/singly_linked_list/singly_linked_list_module.cpp`
- Create: `modules/singly_linked_list/CMakeLists.txt`
- Modify: `CMakeLists.txt` (root — add `BUILD_MODULE_SINGLY_LINKED_LIST` option)
- Modify: `modules/CMakeLists.txt` (guarded `add_subdirectory(singly_linked_list)`)
- Modify: `launcher/CMakeLists.txt` (guarded link)
- Create: `tests/test_singly_linked_list_module.cpp`
- Modify: `tests/CMakeLists.txt`

**Interfaces:**
- Consumes: `underhood::IOperationalModule`, `underhood::AnimatedList<int>` (Task 3), `Canvas::drawArrow` (existing API, unchanged).
- Produces: `underhood::modules::SinglyLinkedListModule` (registers as `"linked_list_singly"`, category `"Data Structures"`), test accessors `size() const -> std::size_t`, `frontValue() const -> int` (caller must check `size() > 0` first).

- [ ] **Step 1: Write the failing test — `tests/test_singly_linked_list_module.cpp`**

```cpp
#include <catch2/catch_test_macros.hpp>

#include "singly_linked_list_module.hpp"

TEST_CASE("SinglyLinkedListModule starts empty") {
    underhood::modules::SinglyLinkedListModule module;
    REQUIRE(module.size() == 0);
    REQUIRE_FALSE(module.canPerform("Delete Front"));
    REQUIRE(module.canPerform("Insert Front"));
    REQUIRE(module.canPerform("Insert Back"));
}

TEST_CASE("SinglyLinkedListModule Insert Front adds to the head") {
    underhood::modules::SinglyLinkedListModule module;
    module.performOperation("Insert Front", 1);
    module.performOperation("Insert Front", 2);

    REQUIRE(module.size() == 2);
    REQUIRE(module.frontValue() == 2);
}

TEST_CASE("SinglyLinkedListModule Insert Back adds to the tail") {
    underhood::modules::SinglyLinkedListModule module;
    module.performOperation("Insert Back", 1);
    module.performOperation("Insert Back", 2);

    REQUIRE(module.size() == 2);
    REQUIRE(module.frontValue() == 1);
}

TEST_CASE("SinglyLinkedListModule Delete Front removes the head") {
    underhood::modules::SinglyLinkedListModule module;
    module.performOperation("Insert Back", 1);
    module.performOperation("Insert Back", 2);
    module.performOperation("Delete Front", 0);

    REQUIRE(module.size() == 1);
    REQUIRE(module.frontValue() == 2);
}

TEST_CASE("SinglyLinkedListModule records operation history") {
    underhood::modules::SinglyLinkedListModule module;
    module.performOperation("Insert Front", 5);
    module.performOperation("Delete Front", 0);

    auto history = module.history();
    REQUIRE(history.size() == 2);
    REQUIRE(history[0] == "Insert Front 5");
    REQUIRE(history[1] == "Delete Front -> 5");
}

TEST_CASE("SinglyLinkedListModule Delete Front on an empty list is a no-op") {
    underhood::modules::SinglyLinkedListModule module;
    module.performOperation("Delete Front", 0);
    REQUIRE(module.size() == 0);
    REQUIRE(module.history().empty());
}

TEST_CASE("SinglyLinkedListModule refuses inserts once at capacity") {
    underhood::modules::SinglyLinkedListModule module;
    for (int i = 0; i < 8; ++i) {
        module.performOperation("Insert Back", i);
    }
    REQUIRE(module.size() == 8);
    REQUIRE_FALSE(module.canPerform("Insert Front"));
    REQUIRE_FALSE(module.canPerform("Insert Back"));
}

TEST_CASE("SinglyLinkedListModule clear empties the list and history") {
    underhood::modules::SinglyLinkedListModule module;
    module.performOperation("Insert Back", 1);
    module.clear();
    REQUIRE(module.size() == 0);
    REQUIRE(module.history().empty());
}

TEST_CASE("SinglyLinkedListModule caps history at 10 entries, dropping the oldest") {
    underhood::modules::SinglyLinkedListModule module;
    // 6 insert+delete cycles = 12 history entries, size never exceeds 1.
    for (int i = 0; i < 6; ++i) {
        module.performOperation("Insert Front", i);
        module.performOperation("Delete Front", 0);
    }
    auto history = module.history();
    REQUIRE(history.size() == 10);
    REQUIRE(history.front() == "Insert Front 1");  // first cycle's 2 entries fell off
    REQUIRE(history.back() == "Delete Front -> 5");
}
```

- [ ] **Step 2: Run it to verify it fails**

Run: `cmake --build build --parallel`
Expected: FAIL — `singly_linked_list_module.hpp` does not exist yet.

- [ ] **Step 3: Write `modules/singly_linked_list/singly_linked_list_module.hpp`**

```cpp
#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "underhood/animated_list.hpp"
#include "underhood/canvas.hpp"
#include "underhood/simulation_module.hpp"

namespace underhood::modules {

class SinglyLinkedListModule : public underhood::IOperationalModule {
public:
    std::string name() const override;
    std::string codeSnippet() const override;
    void render(underhood::Canvas& canvas) const override;

    std::vector<Operation> operations() const override;
    bool canPerform(const std::string& operationLabel) const override;
    void performOperation(const std::string& operationLabel, int value) override;
    std::vector<std::string> history() const override;
    void update(float deltaTime) override;
    void clear() override;

    // Test-facing accessors (not part of IOperationalModule).
    std::size_t size() const;
    int frontValue() const;  // caller must check size() > 0 first

private:
    static constexpr std::size_t kMaxSize = 8;
    underhood::AnimatedList<int> entries_;
    std::vector<std::string> history_;
};

}  // namespace underhood::modules
```

- [ ] **Step 4: Write `modules/singly_linked_list/singly_linked_list_module.cpp`**

```cpp
#include "singly_linked_list_module.hpp"

#include <memory>
#include <string>

#include "underhood/module_registry.hpp"

namespace underhood::modules {

std::string SinglyLinkedListModule::name() const {
    return "linked_list_singly";
}

std::string SinglyLinkedListModule::codeSnippet() const {
    return "";
}

std::vector<underhood::IOperationalModule::Operation> SinglyLinkedListModule::operations() const {
    return {{"Insert Front", true}, {"Insert Back", true}, {"Delete Front", false}};
}

bool SinglyLinkedListModule::canPerform(const std::string& operationLabel) const {
    if (operationLabel == "Insert Front" || operationLabel == "Insert Back") {
        return !entries_.full(kMaxSize);
    }
    if (operationLabel == "Delete Front") {
        return !entries_.empty();
    }
    return false;
}

namespace {
constexpr std::size_t kMaxHistoryEntries = 10;

void appendHistory(std::vector<std::string>& history, std::string entry) {
    history.push_back(std::move(entry));
    if (history.size() > kMaxHistoryEntries) {
        history.erase(history.begin());
    }
}
}  // namespace

void SinglyLinkedListModule::performOperation(const std::string& operationLabel, int value) {
    if (operationLabel == "Insert Front" && !entries_.full(kMaxSize)) {
        entries_.insertAt(0, value);
        appendHistory(history_, "Insert Front " + std::to_string(value));
    } else if (operationLabel == "Insert Back" && !entries_.full(kMaxSize)) {
        entries_.insertAt(entries_.size(), value);
        appendHistory(history_, "Insert Back " + std::to_string(value));
    } else if (operationLabel == "Delete Front" && !entries_.empty()) {
        int frontVal = entries_.entries().front().value;
        entries_.removeAt(0);
        appendHistory(history_, "Delete Front -> " + std::to_string(frontVal));
    }
}

std::vector<std::string> SinglyLinkedListModule::history() const {
    return history_;
}

void SinglyLinkedListModule::update(float deltaTime) {
    entries_.update(deltaTime);
}

void SinglyLinkedListModule::clear() {
    entries_ = underhood::AnimatedList<int>();
    history_.clear();
}

std::size_t SinglyLinkedListModule::size() const {
    return entries_.size();
}

int SinglyLinkedListModule::frontValue() const {
    return entries_.entries().front().value;
}

void SinglyLinkedListModule::render(underhood::Canvas& canvas) const {
    constexpr int kAvailableWidth = 400;
    constexpr int kBaseX = 10;
    constexpr int kBoxHeight = 50;
    constexpr int kBaseY = 100;

    const auto& entries = entries_.entries();
    std::size_t slots = entries.size() + 2;  // +1 null terminator, +1 breathing room
    int cellWidth = kAvailableWidth / static_cast<int>(slots);
    int boxWidth = cellWidth > 20 ? cellWidth - 15 : cellWidth;  // leaves room for the arrow

    auto drawEntry = [&](const underhood::AnimatedList<int>::Entry& entry, std::size_t visualIndex) {
        float progress = entry.insertAnim.progress();
        int x = kBaseX + static_cast<int>(visualIndex) * cellWidth;
        int dropOffset = static_cast<int>((1.0f - progress) * 30.0f);
        underhood::BoxState state =
            entry.insertAnim.isAnimating() ? underhood::BoxState::JustChanged : underhood::BoxState::Owned;
        canvas.drawBox(x, kBaseY - dropOffset, boxWidth, kBoxHeight, std::to_string(entry.value), state);
    };

    for (std::size_t i = 0; i < entries.size(); ++i) {
        drawEntry(entries[i], i);
        int arrowStartX = kBaseX + static_cast<int>(i) * cellWidth + boxWidth;
        int arrowY = kBaseY + kBoxHeight / 2;
        canvas.drawArrow(arrowStartX, arrowY, kBaseX + static_cast<int>(i + 1) * cellWidth, arrowY,
                          underhood::BoxState::Owned);
    }
    if (const auto* removing = entries_.removingEntry()) {
        drawEntry(*removing, 0);
    }

    // "null" terminator after the last node.
    int nullX = kBaseX + static_cast<int>(entries.size()) * cellWidth;
    canvas.drawBox(nullX, kBaseY, boxWidth, kBoxHeight, "null", underhood::BoxState::Empty);
}

}  // namespace underhood::modules

namespace {
struct SinglyLinkedListRegistrar {
    SinglyLinkedListRegistrar() {
        underhood::ModuleRegistry::instance().registerModule(
            "linked_list_singly",
            []() { return std::make_unique<underhood::modules::SinglyLinkedListModule>(); },
            "Data Structures");
    }
};
const SinglyLinkedListRegistrar singlyLinkedListRegistrar;
}  // namespace
```

- [ ] **Step 5: Write `modules/singly_linked_list/CMakeLists.txt`**

```cmake
add_library(underhood_module_singly_linked_list OBJECT singly_linked_list_module.cpp)
target_include_directories(underhood_module_singly_linked_list PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(underhood_module_singly_linked_list PUBLIC underhood_core)
```

- [ ] **Step 6: Add the `BUILD_MODULE_SINGLY_LINKED_LIST` option to root `CMakeLists.txt`**

Add after the `BUILD_MODULE_QUEUE` line:

```cmake
option(BUILD_MODULE_SINGLY_LINKED_LIST "Build the singly linked list simulation module" ON)
```

- [ ] **Step 7: Guard the module in `modules/CMakeLists.txt`**

Append below the `queue` block:

```cmake
if(BUILD_MODULE_SINGLY_LINKED_LIST)
  add_subdirectory(singly_linked_list)
endif()
```

- [ ] **Step 8: Link it into the launcher**

In `launcher/CMakeLists.txt`, append below the `BUILD_MODULE_QUEUE` guard:

```cmake
if(BUILD_MODULE_SINGLY_LINKED_LIST)
  target_link_libraries(underhood PRIVATE underhood_module_singly_linked_list)
endif()
```

- [ ] **Step 9: Add the test to `tests/CMakeLists.txt`**

Add `test_singly_linked_list_module.cpp` to the source list and `underhood_module_singly_linked_list` to the link list.

- [ ] **Step 10: Run tests to verify they pass**

Run:
```bash
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```
Expected: all tests pass, including the 8 new `SinglyLinkedListModule` tests.

- [ ] **Step 11: Commit**

```bash
git add modules/singly_linked_list CMakeLists.txt modules/CMakeLists.txt launcher/CMakeLists.txt tests/test_singly_linked_list_module.cpp tests/CMakeLists.txt
git commit -m "feat: add singly linked list simulation module"
```

---

### Task 8: `Circular Linked List` module

**Files:**
- Create: `modules/circular_linked_list/circular_linked_list_module.hpp`
- Create: `modules/circular_linked_list/circular_linked_list_module.cpp`
- Create: `modules/circular_linked_list/CMakeLists.txt`
- Modify: `CMakeLists.txt` (root — add `BUILD_MODULE_CIRCULAR_LINKED_LIST` option)
- Modify: `modules/CMakeLists.txt` (guarded `add_subdirectory(circular_linked_list)`)
- Modify: `launcher/CMakeLists.txt` (guarded link)
- Create: `tests/test_circular_linked_list_module.cpp`
- Modify: `tests/CMakeLists.txt`

**Interfaces:**
- Consumes: `underhood::IOperationalModule`, `underhood::AnimatedList<int>` (Task 3), `Canvas::drawArrow`/`drawText` (existing API).
- Produces: `underhood::modules::CircularLinkedListModule` (registers as `"linked_list_circular"`, category `"Data Structures"`), test accessors `size() const -> std::size_t`, `frontValue() const -> int` (caller must check `size() > 0` first).

- [ ] **Step 1: Write the failing test — `tests/test_circular_linked_list_module.cpp`**

```cpp
#include <catch2/catch_test_macros.hpp>

#include "circular_linked_list_module.hpp"

TEST_CASE("CircularLinkedListModule starts empty") {
    underhood::modules::CircularLinkedListModule module;
    REQUIRE(module.size() == 0);
    REQUIRE_FALSE(module.canPerform("Delete Front"));
    REQUIRE(module.canPerform("Insert"));
}

TEST_CASE("CircularLinkedListModule Insert always appends to the back") {
    underhood::modules::CircularLinkedListModule module;
    module.performOperation("Insert", 1);
    module.performOperation("Insert", 2);
    module.performOperation("Insert", 3);

    REQUIRE(module.size() == 3);
    REQUIRE(module.frontValue() == 1);
}

TEST_CASE("CircularLinkedListModule Delete Front removes the head") {
    underhood::modules::CircularLinkedListModule module;
    module.performOperation("Insert", 1);
    module.performOperation("Insert", 2);
    module.performOperation("Delete Front", 0);

    REQUIRE(module.size() == 1);
    REQUIRE(module.frontValue() == 2);
}

TEST_CASE("CircularLinkedListModule records operation history") {
    underhood::modules::CircularLinkedListModule module;
    module.performOperation("Insert", 5);
    module.performOperation("Delete Front", 0);

    auto history = module.history();
    REQUIRE(history.size() == 2);
    REQUIRE(history[0] == "Insert 5");
    REQUIRE(history[1] == "Delete Front -> 5");
}

TEST_CASE("CircularLinkedListModule Delete Front on an empty list is a no-op") {
    underhood::modules::CircularLinkedListModule module;
    module.performOperation("Delete Front", 0);
    REQUIRE(module.size() == 0);
    REQUIRE(module.history().empty());
}

TEST_CASE("CircularLinkedListModule refuses Insert once at capacity") {
    underhood::modules::CircularLinkedListModule module;
    for (int i = 0; i < 8; ++i) {
        module.performOperation("Insert", i);
    }
    REQUIRE(module.size() == 8);
    REQUIRE_FALSE(module.canPerform("Insert"));
}

TEST_CASE("CircularLinkedListModule clear empties the list and history") {
    underhood::modules::CircularLinkedListModule module;
    module.performOperation("Insert", 1);
    module.clear();
    REQUIRE(module.size() == 0);
    REQUIRE(module.history().empty());
}

TEST_CASE("CircularLinkedListModule caps history at 10 entries, dropping the oldest") {
    underhood::modules::CircularLinkedListModule module;
    // 6 insert+delete cycles = 12 history entries, size never exceeds 1.
    for (int i = 0; i < 6; ++i) {
        module.performOperation("Insert", i);
        module.performOperation("Delete Front", 0);
    }
    auto history = module.history();
    REQUIRE(history.size() == 10);
    REQUIRE(history.front() == "Insert 1");  // first cycle's 2 entries fell off
    REQUIRE(history.back() == "Delete Front -> 5");
}
```

- [ ] **Step 2: Run it to verify it fails**

Run: `cmake --build build --parallel`
Expected: FAIL — `circular_linked_list_module.hpp` does not exist yet.

- [ ] **Step 3: Write `modules/circular_linked_list/circular_linked_list_module.hpp`**

```cpp
#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "underhood/animated_list.hpp"
#include "underhood/canvas.hpp"
#include "underhood/simulation_module.hpp"

namespace underhood::modules {

class CircularLinkedListModule : public underhood::IOperationalModule {
public:
    std::string name() const override;
    std::string codeSnippet() const override;
    void render(underhood::Canvas& canvas) const override;

    std::vector<Operation> operations() const override;
    bool canPerform(const std::string& operationLabel) const override;
    void performOperation(const std::string& operationLabel, int value) override;
    std::vector<std::string> history() const override;
    void update(float deltaTime) override;
    void clear() override;

    // Test-facing accessors (not part of IOperationalModule).
    std::size_t size() const;
    int frontValue() const;  // caller must check size() > 0 first

private:
    static constexpr std::size_t kMaxSize = 8;
    underhood::AnimatedList<int> entries_;
    std::vector<std::string> history_;
};

}  // namespace underhood::modules
```

- [ ] **Step 4: Write `modules/circular_linked_list/circular_linked_list_module.cpp`**

```cpp
#include "circular_linked_list_module.hpp"

#include <algorithm>
#include <memory>
#include <string>

#include "underhood/module_registry.hpp"

namespace underhood::modules {

std::string CircularLinkedListModule::name() const {
    return "linked_list_circular";
}

std::string CircularLinkedListModule::codeSnippet() const {
    return "";
}

std::vector<underhood::IOperationalModule::Operation> CircularLinkedListModule::operations() const {
    return {{"Insert", true}, {"Delete Front", false}};
}

bool CircularLinkedListModule::canPerform(const std::string& operationLabel) const {
    if (operationLabel == "Insert") {
        return !entries_.full(kMaxSize);
    }
    if (operationLabel == "Delete Front") {
        return !entries_.empty();
    }
    return false;
}

namespace {
constexpr std::size_t kMaxHistoryEntries = 10;

void appendHistory(std::vector<std::string>& history, std::string entry) {
    history.push_back(std::move(entry));
    if (history.size() > kMaxHistoryEntries) {
        history.erase(history.begin());
    }
}
}  // namespace

void CircularLinkedListModule::performOperation(const std::string& operationLabel, int value) {
    if (operationLabel == "Insert" && !entries_.full(kMaxSize)) {
        entries_.insertAt(entries_.size(), value);  // always appends to the back
        appendHistory(history_, "Insert " + std::to_string(value));
    } else if (operationLabel == "Delete Front" && !entries_.empty()) {
        int frontVal = entries_.entries().front().value;
        entries_.removeAt(0);
        appendHistory(history_, "Delete Front -> " + std::to_string(frontVal));
    }
}

std::vector<std::string> CircularLinkedListModule::history() const {
    return history_;
}

void CircularLinkedListModule::update(float deltaTime) {
    entries_.update(deltaTime);
}

void CircularLinkedListModule::clear() {
    entries_ = underhood::AnimatedList<int>();
    history_.clear();
}

std::size_t CircularLinkedListModule::size() const {
    return entries_.size();
}

int CircularLinkedListModule::frontValue() const {
    return entries_.entries().front().value;
}

void CircularLinkedListModule::render(underhood::Canvas& canvas) const {
    constexpr int kAvailableWidth = 400;
    constexpr int kBaseX = 10;
    constexpr int kBoxHeight = 50;
    constexpr int kBaseY = 100;

    const auto& entries = entries_.entries();
    std::size_t slots = entries.size() + 1;
    int cellWidth = kAvailableWidth / static_cast<int>(std::max<std::size_t>(slots, 1));
    int boxWidth = cellWidth > 20 ? cellWidth - 15 : cellWidth;

    auto drawEntry = [&](const underhood::AnimatedList<int>::Entry& entry, std::size_t visualIndex) {
        float progress = entry.insertAnim.progress();
        int x = kBaseX + static_cast<int>(visualIndex) * cellWidth;
        int dropOffset = static_cast<int>((1.0f - progress) * 30.0f);
        underhood::BoxState state =
            entry.insertAnim.isAnimating() ? underhood::BoxState::JustChanged : underhood::BoxState::Owned;
        canvas.drawBox(x, kBaseY - dropOffset, boxWidth, kBoxHeight, std::to_string(entry.value), state);
    };

    for (std::size_t i = 0; i < entries.size(); ++i) {
        drawEntry(entries[i], i);
        if (i + 1 < entries.size()) {
            int arrowStartX = kBaseX + static_cast<int>(i) * cellWidth + boxWidth;
            int arrowY = kBaseY + kBoxHeight / 2;
            canvas.drawArrow(arrowStartX, arrowY, kBaseX + static_cast<int>(i + 1) * cellWidth, arrowY,
                              underhood::BoxState::Owned);
        }
    }
    if (const auto* removing = entries_.removingEntry()) {
        drawEntry(*removing, 0);
    }

    // v1 simplification: the wrap-around is a straight line to the first
    // box plus a text label, not a true curved arrow (Canvas::drawArrow only
    // draws straight lines; see the design spec's "Bilinen sadeleştirmeler").
    if (entries.size() >= 2) {
        int lastX = kBaseX + static_cast<int>(entries.size() - 1) * cellWidth + boxWidth / 2;
        int firstX = kBaseX + boxWidth / 2;
        int topY = kBaseY - 20;
        canvas.drawArrow(lastX, kBaseY, firstX, topY, underhood::BoxState::JustChanged);
        canvas.drawText("wraps to front", kBaseX, kBaseY + kBoxHeight + 10);
    } else if (entries.size() == 1) {
        canvas.drawText("wraps to front (self)", kBaseX, kBaseY + kBoxHeight + 10);
    }
}

}  // namespace underhood::modules

namespace {
struct CircularLinkedListRegistrar {
    CircularLinkedListRegistrar() {
        underhood::ModuleRegistry::instance().registerModule(
            "linked_list_circular",
            []() { return std::make_unique<underhood::modules::CircularLinkedListModule>(); },
            "Data Structures");
    }
};
const CircularLinkedListRegistrar circularLinkedListRegistrar;
}  // namespace
```

- [ ] **Step 5: Write `modules/circular_linked_list/CMakeLists.txt`**

```cmake
add_library(underhood_module_circular_linked_list OBJECT circular_linked_list_module.cpp)
target_include_directories(underhood_module_circular_linked_list PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(underhood_module_circular_linked_list PUBLIC underhood_core)
```

- [ ] **Step 6: Add the `BUILD_MODULE_CIRCULAR_LINKED_LIST` option to root `CMakeLists.txt`**

Add after the `BUILD_MODULE_SINGLY_LINKED_LIST` line:

```cmake
option(BUILD_MODULE_CIRCULAR_LINKED_LIST "Build the circular linked list simulation module" ON)
```

- [ ] **Step 7: Guard the module in `modules/CMakeLists.txt`**

Append below the `singly_linked_list` block:

```cmake
if(BUILD_MODULE_CIRCULAR_LINKED_LIST)
  add_subdirectory(circular_linked_list)
endif()
```

- [ ] **Step 8: Link it into the launcher**

In `launcher/CMakeLists.txt`, append below the `BUILD_MODULE_SINGLY_LINKED_LIST` guard:

```cmake
if(BUILD_MODULE_CIRCULAR_LINKED_LIST)
  target_link_libraries(underhood PRIVATE underhood_module_circular_linked_list)
endif()
```

- [ ] **Step 9: Add the test to `tests/CMakeLists.txt`**

Add `test_circular_linked_list_module.cpp` to the source list and `underhood_module_circular_linked_list` to the link list.

- [ ] **Step 10: Run tests to verify they pass**

Run:
```bash
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```
Expected: all tests pass, including the 8 new `CircularLinkedListModule` tests.

- [ ] **Step 11: Commit**

```bash
git add modules/circular_linked_list CMakeLists.txt modules/CMakeLists.txt launcher/CMakeLists.txt tests/test_circular_linked_list_module.cpp tests/CMakeLists.txt
git commit -m "feat: add circular linked list simulation module"
```

---

### Task 9: Docs + manual end-to-end verification

**Files:**
- Modify: `CLAUDE.md`
- Modify: `CONTRIBUTING.md`
- Modify: `HANDOFF.md`

**Interfaces:** None — documentation and verification only, per the project's Definition of Done (real click-through, not just green tests).

- [ ] **Step 1: Update `CLAUDE.md`'s "Visual design" section**

Append this paragraph to the end of the existing "Visual design" section (after the `UNDERHOOD_ASSETS_DIR` sentence added by the earlier launcher redesign):

```md

Two module kinds share the launcher: `IStepSimulationModule` (fixed scripted scenario, `unique_ptr`/`shared_ptr`/`move_semantics`) and `IOperationalModule` (open-ended Push/Pop-style operations, the `Data Structures` category — `stack`/`queue`/`linked_list_singly`/`linked_list_circular`). `ISimulationModule::kind()` tells the launcher which panel set to draw; a module never needs both. Operational modules share one `AnimatedList<int>` (`core/include/underhood/animated_list.hpp`) for their insert/remove animation bookkeeping instead of reimplementing it per module — each entry's `Animator` (`core/include/underhood/animation.hpp`) is a simple 0.30s ease-out progress tracker `render()` reads to interpolate a box's slide-in offset or fade. Operational modules keep an append-only `history()` log the Controls panel shows reversed (newest first); `Canvas::drawArrow` (previously only used by `unique_ptr`/`move_semantics`'s ownership arrows) is now also used by the linked-list modules to connect nodes, with a documented v1 simplification for the circular list's wrap-around (a straight line + label, not a true curve).
```

- [ ] **Step 2: Update `CONTRIBUTING.md`'s "Adding a new simulation module" section**

Replace:

```md
## Adding a new simulation module

1. Copy `modules/_template/` to `modules/<your_topic>/` and rename `template_module.hpp`/`.cpp` and the `TemplateModule` class to match your topic.
2. Implement each `ISimulationModule` method: `codeSnippet()` returns the fixed code your module walks through, `parameters()` lists the values a viewer can tweak, `reset()`/`step()` drive your topic's state machine, `render()` draws the current state with `Canvas::drawBox`/`drawArrow`/`drawText`.
3. Uncomment and adapt the registrar block at the bottom of your module's `.cpp` file so it registers itself with `ModuleRegistry` under a unique name.
4. Add a `BUILD_MODULE_<YOUR_TOPIC>` option to the root `CMakeLists.txt` (copy the existing module options), add your folder to `modules/CMakeLists.txt` behind that option, and link your module's target into `launcher/CMakeLists.txt`.
5. Write a unit test in `tests/` exercising your module's `reset()`/`step()` sequence directly (see `tests/test_unique_ptr_module.cpp` for the pattern) and add it to `tests/CMakeLists.txt`.
6. Run the full build+test loop above, then manually run the launcher and confirm your module appears in the menu and steps through correctly.
```

with:

```md
## Adding a new simulation module

First decide which kind your topic is:
- **`IStepSimulationModule`** — a single fixed, pre-scripted scenario the viewer steps through one line at a time (the code panel matters). Pattern: `unique_ptr`, `shared_ptr`, `move_semantics`.
- **`IOperationalModule`** — an open-ended structure the viewer drives with Push/Pop-style buttons and a value input, no fixed code snippet (`codeSnippet()` returns `""`). Pattern: `stack`, `queue`, `linked_list_singly`, `linked_list_circular`. If your topic has real insert/remove animation, use `underhood::AnimatedList<int>` (`core/include/underhood/animated_list.hpp`) for the bookkeeping instead of writing your own — see any of the 4 Data Structures modules for the pattern.

Steps (both kinds):

1. Copy `modules/_template/` to `modules/<your_topic>/` and rename `template_module.hpp`/`.cpp` and the `TemplateModule` class to match your topic. `_template` implements `IStepSimulationModule`; if you're building an `IOperationalModule`, use one of the 4 Data Structures modules as your starting point instead (e.g. `modules/stack/`).
2. Implement every method your chosen interface requires. For `IStepSimulationModule`: `codeSnippet()`, `parameters()`, `reset()`, `step()`, `currentHighlightedLine()`. For `IOperationalModule`: `operations()`, `canPerform()`, `performOperation()`, `history()`, `update()`, `clear()`. Both kinds implement `name()` and `render()` (draw with `Canvas::drawBox`/`drawArrow`/`drawText`).
3. Uncomment and adapt the registrar block at the bottom of your module's `.cpp` file so it registers itself with `ModuleRegistry` under a unique name and a category (e.g. `"Smart Pointers"`, `"Data Structures"`, or a new one — the launcher's sidebar groups by whatever string you pass, no launcher changes needed for a new category).
4. Add a `BUILD_MODULE_<YOUR_TOPIC>` option to the root `CMakeLists.txt` (copy the existing module options), add your folder to `modules/CMakeLists.txt` behind that option, and link your module's target into `launcher/CMakeLists.txt` behind the same option.
5. Write a unit test in `tests/` exercising your module's state logic directly (see `tests/test_unique_ptr_module.cpp` for the `IStepSimulationModule` pattern, `tests/test_stack_module.cpp` for the `IOperationalModule` pattern) and add it to `tests/CMakeLists.txt`.
6. Run the full build+test loop above, then manually run the launcher and confirm your module appears under the right sidebar category and behaves correctly end to end.
```

- [ ] **Step 3: Run the full local verification loop**

Run:
```bash
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```
Expected: build succeeds, all tests pass (7 original + 3 Animator + 5 AnimatedList + 7 Stack + 7 Queue + 8 SinglyLinkedList + 8 CircularLinkedList = 45 tests).

- [ ] **Step 4: Manual end-to-end verification (requires a human with a display)**

Run the built launcher (`build/launcher/Debug/underhood.exe` or platform equivalent) and confirm:
- The sidebar shows two sections: "SMART POINTERS" (unique_ptr, shared_ptr, move_semantics — unchanged from before) and "DATA STRUCTURES" (stack, queue, linked_list_singly, linked_list_circular).
- Clicking `stack`: a "Value" input and Push/Pop buttons appear in Controls; Pop is disabled while empty. Typing a value and clicking Push animates a new box sliding in at the top of a vertical stack in Visualization, and a "Push N" line appears in the History list. Clicking Pop animates the top box fading/sliding out and adds a "Pop -> N" history line.
- Clicking `queue`: same but Enqueue/Dequeue, horizontal layout, front/back labels.
- Clicking `linked_list_singly`: Insert Front/Insert Back/Delete Front all work, boxes are connected by arrows, a "null" box trails the last node.
- Clicking `linked_list_circular`: Insert/Delete Front work, and once there are 2+ nodes a "wraps to front" arrow/label appears near the last node.
- Pushing/inserting past 8 elements disables the relevant button instead of crashing or silently doing nothing without feedback.
- The Code panel is empty (no placeholder text) while any Data Structures module is active, and still works as before for the 3 Smart Pointers modules.
- Clicking `unique_ptr`/`shared_ptr`/`move_semantics` still behaves exactly as before this plan (Reset/Next Step, no Controls-panel regressions).

If any of these don't match, note the specific mismatch — this step's outcome is a human judgment call, not an automated pass/fail, and any fix goes through the normal task-review loop before this plan is considered done.

- [ ] **Step 5: Update `HANDOFF.md`**

Replace `HANDOFF.md`'s contents to reflect the finished state (adapt the date/commit list to whatever is current when this step actually runs):

```md
# Handoff — underhood

## Şu an ne yapılıyor
Core framework + 3 "Smart Pointers" modülü (unique_ptr, shared_ptr, move_semantics) + 4 "Data Structures" modülü (stack, queue, linked_list_singly, linked_list_circular) + launcher tamamlandı. İki modül türü var: IStepSimulationModule (sabit senaryo) ve IOperationalModule (açık uçlu Push/Pop tarzı operasyonlar + animasyon + geçmiş). CI yeşil (Task 12'nin push+doğrulama adımı hâlâ bekliyor, ayrı not).

## Sıradaki somut adım
Yeni bir modül eklemek istenirse CONTRIBUTING.md'deki akışı takip et (önce IStepSimulationModule mi IOperationalModule mü karar ver). Framework tarafında planlanmış bir sonraki iş yok.

## Bilinmesi gerekenler
- Circular linked list'in "başa dönüş" oku gerçek bir kavis değil, düz çizgi + etiket (bkz. docs/superpowers/specs/2026-08-25-underhood-operational-modules-design.md "Bilinen sadeleştirmeler").
- Tüm Data Structures modülleri max 8 eleman, kullanıcı tarafından ayarlanamaz.
- Task 12'nin push+CI-yeşil doğrulaması hâlâ yapılmadı — repoda henüz git remote yok.

## İlgili dosyalar
- docs/superpowers/specs/2026-08-24-underhood-core-design.md — çekirdek framework tasarımı
- docs/superpowers/specs/2026-08-25-underhood-operational-modules-design.md — bu planın tasarımı
- docs/superpowers/plans/2026-08-25-underhood-operational-modules.md — bu implementasyon planı (tamamlandı)
- CONTRIBUTING.md — yeni modül ekleme rehberi (her iki modül türü için)

## Son 3 commit
(implementasyon sırasında oluşan son 3 commit'i `git log --oneline -3` ile buraya kopyala)
```

- [ ] **Step 6: Commit**

```bash
git add CLAUDE.md CONTRIBUTING.md HANDOFF.md
git commit -m "docs: document operational modules and finalize handoff"
```
