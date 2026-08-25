# underhood

Interactive C++ simulations that make niche language behavior (smart pointer ownership, move semantics, and more) visible instead of abstract.

## Why
Reasoning about ownership transfer, reference counting, or move semantics from text alone is hard to retain. underhood renders a fixed code snippet next to an animated view of memory/ownership state, stepped one line at a time, with editable starting parameters — so the mental model has something to point at.

## Stack
C++17 · CMake · raylib · Dear ImGui (via rlImGui) · Catch2

## Quick start
```bash
git clone <repo-url>
cd underhood
cmake -S . -B build
cmake --build build --parallel
./build/launcher/underhood
```

## Documentation
- [Architecture](docs/architecture.md)
- [Contributing a new module](CONTRIBUTING.md)

## License
MIT — see [LICENSE](LICENSE)

Bundles the [Inter](https://github.com/rsms/inter) typeface (SIL Open Font License 1.1 — see [assets/fonts/Inter-OFL.txt](assets/fonts/Inter-OFL.txt)).
