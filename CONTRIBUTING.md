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

1. Copy `modules/_template/` to `modules/<your_topic>/` and rename `template_module.hpp`/`.cpp` and the `TemplateModule` class to match your topic.
2. Implement each `ISimulationModule` method: `codeSnippet()` returns the fixed code your module walks through, `parameters()` lists the values a viewer can tweak, `reset()`/`step()` drive your topic's state machine, `render()` draws the current state with `Canvas::drawBox`/`drawArrow`/`drawText`.
3. Uncomment and adapt the registrar block at the bottom of your module's `.cpp` file so it registers itself with `ModuleRegistry` under a unique name.
4. Add a `BUILD_MODULE_<YOUR_TOPIC>` option to the root `CMakeLists.txt` (copy the existing module options), add your folder to `modules/CMakeLists.txt` behind that option, and link your module's target into `launcher/CMakeLists.txt`.
5. Write a unit test in `tests/` exercising your module's `reset()`/`step()` sequence directly (see `tests/test_unique_ptr_module.cpp` for the pattern) and add it to `tests/CMakeLists.txt`.
6. Run the full build+test loop above, then manually run the launcher and confirm your module appears in the menu and steps through correctly.
