# Contributing to underhood

## Building and testing

```bash
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Run the app itself with `./build/launcher/underhood` (or `underhood.exe` on Windows).

## Workflow

- Every module ships with a unit test for its `step()`/parameter logic — no "add tests later" PRs.
- TDD order: write the failing test, watch it fail, write the minimal implementation, watch it pass, then commit.
- Branch naming: `feat/<short-description>`, `fix/<short-description>`.
- Commit messages: Conventional Commits (`feat:`, `fix:`, `docs:`, `test:`, `refactor:`, `chore:`), English, imperative mood.
- CI must be green (build + test on Windows/Linux/macOS) before merge.

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
