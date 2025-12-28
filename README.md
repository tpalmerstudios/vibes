# DeerBourne

DeerBourne is a modular C99 project that splits responsibilities between two components:

- **Bourne (back-end):** A background service responsible for persistent, disk-based storage of small JSON structures in a non-binary format. A static library contains the service primitives, and a daemon-style executable links against it.
- **Deer (front-end):** A TCP-based interface that exposes localized socket access for the data managed by Bourne. It currently provides a GTK4 GUI and is designed to expand to multiple sensor sources.

## Service Overview
Bourne now exposes a modular, multi-thread-ready scaffold. The service initializes defaults, prepares the listening socket, wires a worker pool, and logs verbosely if socket setup or connection acceptance fails. The daemon is designed for multiple front-end instances, but the dispatch loop is intentionally stubbed while the storage and protocol design are finalized.

## Repository Layout
- `bourne/`: Back-end static library (`src/`), headers (`include/`), and service entry point (`service/`). Modules are split across config, logging, network, threading, storage, and lifecycle orchestration.
- `deer/`: Front-end modules (`src/`), headers (`include/`), and GUI entry point (`app/`). Core (config/client) logic is GUI-agnostic; GTK layout lives in a separate UI library.
- `tests/`: Lightweight unit-test executables for configuration scaffolding; enabled via CMake option.
- `CMakeLists.txt`: Project build configuration targeting C99 with library, service, docs, and test targets.
- `Doxyfile`: Doxygen configuration for generating project documentation.
- `.clang-format`: Style configuration using the GNU base style.
- `.gitignore`: Ignored files for the repository.

## Build
This project uses CMake. A typical build flow is:

```bash
cmake -S . -B build
cmake --build build
```

Targets of interest include:
- `bourne` (static library)
- `bourne_service` (back-end service executable)
- `bourne_service_debug` (debug-oriented Bourne daemon with symbols)
- `deer_core` (front-end client/config library without GTK)
- `deer_ui` (GTK4 layout helpers, built when GTK4 is available)
- `deer_app` (GTK4 GUI executable; skipped when GTK4 is missing)
- `deer_app_debug` (debug-oriented GUI build with symbols)
- `docs` (if Doxygen is available and the docs option is enabled)
- `tests` (when tests are added and the testing option is enabled)

The GUI target depends on GTK4 (no GTK3 fallback) and uses `pkg-config` for discovery. When GTK4 is not present, GUI targets are skipped automatically. On Debian/Ubuntu, install with:

```bash
sudo apt-get install -y libgtk-4-dev pkg-config
```

### GUI configuration
`deer_app` accepts an optional configuration path as its first argument. When present, the file is parsed using an ini-style format:

```
[Theme]
Name=DeerBourne Custom

[Colors]
Background=#11202D
Foreground=#EDEFF2
Accent=#6BD9FF
```

If no configuration file is provided or parsing fails, the GUI falls back to the built-in "DeerBourne Modern" theme. The window exposes a color section where you can interactively adjust background, foreground, and accent colors at runtime.

### GUI layout
The GTK4 interface is organized for clarity and future growth:

- **Main search area:** Enter queries that will be sent to the Bourne service; results are appended to history.
- **History view:** A scrollable log of prior queries.
- **Saved items view:** A scrollable list of saved entries or responses.
- **Saved item entry:** A dedicated input and control to submit data for persistence.
- **Theme colors:** Controls to adjust the active palette live.

## Testing
With tests enabled (default), lightweight unit tests cover the configuration scaffolding for both Bourne and Deer:

```bash
cmake -S . -B build -DDEERBOURNE_ENABLE_GUI=OFF  # optional: skip GUI dependencies for headless CI
cmake --build build
ctest --test-dir build
```

## Documentation
Generate documentation with Doxygen:

```bash
doxygen Doxyfile
```
