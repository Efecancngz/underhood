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
- The spec calls for "animated" state updates; this only applies to `IStepSimulationModule`s (`unique_ptr`/`shared_ptr`/`move_semantics`), which still render each `step()` result as an instant snap (no interpolation/tweening between states). The state itself still changes per step and is visually clear, but there's no motion. `IOperationalModule`s (the Data Structures category) DO animate their insert/remove transitions via `AnimatedList`/`Animator` (see "Visual design" below). If step-to-step snapping turns out to hurt retention for the Step modules too, add tweening (e.g. lerp box positions/opacity over a few frames) inside `Canvas`/`render()` without touching the `ISimulationModule` interface.

## Visual design

Box color is semantic, not decorative: gray = empty/nullptr, teal = holds a value, amber = changed on the last step (the highlighted code line uses the same amber, so "what just happened" reads the same way in both panels). Palette is deliberately flat and muted — no gradients, no glassmorphism, none of raylib's default neon colors — closer to how data-structure visualizers (e.g. visualgo.net) present state than to a typical UI mockup. Both a dark and a light variant of this palette exist; color is reinforcement, not the only signal — labels always spell out the state in words too.

`underhood::GetPalette`/`underhood::ApplyTheme` (`core/include/underhood/theme.hpp`) are the single source of truth for both the ImGui chrome and `Canvas`'s raylib-drawn boxes, so the two can't drift into two different "dark themes." The launcher uses a fixed docked layout (`ImGui::DockBuilder*`, built once per run — `io.IniFilename` is null so there's no stale `imgui.ini` to fight) instead of v1's free-floating ImGui windows: a persistent left sidebar lists modules (visualgo.net/dsa-visualizer-style nav, not a menu screen you leave), with Code/Controls/Visualization panels docked to the right. The simulation itself renders into a `RenderTexture2D` sized to match the Visualization panel's content region exactly (recreated only when that size actually changes) rather than drawing straight to the window, so the canvas is always 1:1 pixel-crisp — never blurred by `rlImGuiImageRenderTextureFit` upscaling a fixed-size texture — and can still live inside a resizable dock panel. UI text uses a bundled Inter font (OFL-licensed, `assets/fonts/`, loaded once for both ImGui and raylib's direct `Canvas` text so the two don't visually clash) instead of ImGui's default pixel font. The font path is baked in at build time via `UNDERHOOD_ASSETS_DIR` (absolute path to the source tree's `assets/` — fine for a dev/portfolio build launched from its own build directory, would need real asset packaging for a distributable install).

Two module kinds share the launcher: `IStepSimulationModule` (fixed scripted scenario, `unique_ptr`/`shared_ptr`/`move_semantics`) and `IOperationalModule` (open-ended Push/Pop-style operations, the `Data Structures` category — `stack`/`queue`/`linked_list_singly`/`linked_list_circular`). `ISimulationModule::kind()` tells the launcher which panel set to draw; a module never needs both. Operational modules share one `AnimatedList<int>` (`core/include/underhood/animated_list.hpp`) for their insert/remove animation bookkeeping instead of reimplementing it per module — each entry's `Animator` (`core/include/underhood/animation.hpp`) is a simple 0.30s ease-out progress tracker `render()` reads to interpolate a box's slide-in offset or fade. Operational modules keep an append-only `history()` log the Controls panel shows reversed (newest first); `Canvas::drawArrow` (previously only used by `unique_ptr`/`move_semantics`'s ownership arrows) is now also used by the linked-list modules to connect nodes, with a documented v1 simplification for the circular list's wrap-around (a straight line + label, not a true curve).
