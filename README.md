# DeerBourne

DeerBourne is a modular C99 project that splits responsibilities between two components:

- **Bourne (back-end):** A background service responsible for persistent, disk-based storage of small JSON structures in a non-binary format. The shared functionality is provided as a static library, and the service executable links against it.
- **Deer (front-end):** A TCP-based interface that exposes localized socket access for the data managed by Bourne. It will accept input from Linux GUI elements (initially a single input field) and is designed to expand to multiple sensor sources.

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
- `deer` (front-end library)
- `deer_app` (GTK4 GUI executable)
- `docs` (if Doxygen is available and the docs option is enabled)
- `tests` (when tests are added and the testing option is enabled)

The GUI target depends on GTK4 and uses `pkg-config` for discovery. On Debian/Ubuntu, install with:

```bash
sudo apt-get install -y libgtk-4-dev pkg-config
```

## Documentation
Generate documentation with Doxygen:

```bash
doxygen Doxyfile
```
