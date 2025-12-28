# DeerBourne

DeerBourne is a modular C99 project that splits responsibilities between two components:

- **Bourne (back-end):** A background service responsible for persistent, disk-based storage of small JSON structures in a non-binary format. A static library contains the service primitives, and a daemon-style executable links against it.
- **Deer (front-end):** A TCP-based interface that exposes localized socket access for the data managed by Bourne. It currently provides a GTK4 GUI and is designed to expand to multiple sensor sources.

## Service Overview
Bourne runs a simple TCP control loop bound to `127.0.0.1:5555` by default. Connections are accepted sequentially, providing a clear data boundary between the socket listener and higher-level processing so the service can evolve without changing the front-end contract. The loop is started and stopped through the Bourne library, keeping daemon responsibilities modular and testable.

## Repository Layout
- `bourne/`: Back-end static library (`src/`), headers (`include/`), and service entry point (`service/`).
- `deer/`: Front-end source (`src/`), headers (`include/`), and GUI entry point (`app/`).
- `tests/`: Placeholder for future tests; enabled via CMake option.
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
- `deer` (front-end library)
- `deer_app` (GTK4 GUI executable)
- `deer_app_debug` (debug-oriented GUI build with symbols)
- `docs` (if Doxygen is available and the docs option is enabled)
- `tests` (when tests are added and the testing option is enabled)

The GUI target depends on GTK4 (no GTK3 fallback) and uses `pkg-config` for discovery. On Debian/Ubuntu, install with:

```bash
sudo apt-get install -y libgtk-4-dev pkg-config
```

### GUI configuration
`deer_app` accepts an optional configuration path as its first argument. When present, the file is parsed using an ini-style forma
t:

```
[Theme]
Name=DeerBourne Custom

[Colors]
Background=#11202D
Foreground=#EDEFF2
Accent=#6BD9FF
```

If no configuration file is provided or parsing fails, the GUI falls back to the built-in "DeerBourne Modern" theme. The window e
xposes a color section where you can interactively adjust background, foreground, and accent colors at runtime.

### GUI layout
The GTK4 interface is organized for clarity and future growth:

- **Main search area:** Enter queries that will be sent to the Bourne service; results are appended to history.
- **History view:** A scrollable log of prior queries.
- **Saved items view:** A scrollable list of saved entries or responses.
- **Saved item entry:** A dedicated input and control to submit data for persistence.
- **Theme colors:** Controls to adjust the active palette live.

## Documentation
Generate documentation with Doxygen:

```bash
doxygen Doxyfile
```
