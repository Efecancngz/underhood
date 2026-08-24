# underhood — CLAUDE.md

Interactive C++ simulations for niche language behavior (smart pointers, move semantics, more later). See [README.md](README.md) for the pitch and [docs/architecture.md](docs/architecture.md) for the full design rationale. Handoff state: [HANDOFF.md](HANDOFF.md).

## Running

```bash
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/launcher/underhood
```

## Architecture (why, not what — see docs/architecture.md for detail)

- Single launcher binary. Each topic is an independent CMake `OBJECT` library under `modules/<name>/` implementing `ISimulationModule` (`core/include/underhood/simulation_module.hpp`). `OBJECT` (not `STATIC`) is deliberate: a module's self-registration global only runs if its object file is actually linked in, and linkers can silently drop unreferenced members of a `STATIC` archive.
- `ModuleRegistry` (`core/include/underhood/module_registry.hpp`) is a Meyers' singleton (function-local `static`) specifically to avoid static-initialization-order-fiasco: each module's self-registering global calls `ModuleRegistry::instance()`, which is safe to call in any order because the singleton constructs itself lazily on first use.
- Code shown in a module is always fixed; only starting parameter *values* are editable from the UI. No live compilation.
- Each module is toggled by its own `BUILD_MODULE_<NAME>` CMake option, so a broken module fails CI in isolation and can be disabled without blocking the others.

## Adding a module

See [CONTRIBUTING.md](CONTRIBUTING.md).

## Known simplifications (v1)

- The full Idle/Configuring/Stepping/Finished state diagram in the design spec is collapsed in the launcher UI: parameters are editable at any time, and there's no distinct visual "Finished" state beyond `step()` returning `false`. Revisit if this causes confusion in practice.
- The spec calls for "animated" state updates; v1 renders each `step()` result as an instant snap (no interpolation/tweening between states). The state itself still changes per step and is visually clear, but there's no motion. If step-to-step snapping turns out to hurt retention, add tweening (e.g. lerp box positions/opacity over a few frames) inside `Canvas`/`render()` without touching the `ISimulationModule` interface.
