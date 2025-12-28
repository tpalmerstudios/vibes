# DeerBourne

DeerBourne is a modular C99 project that splits responsibilities between two components:

- **Bourne (back-end):** A background service responsible for persistent, disk-based storage of small JSON structures in a non-binary format. A static library contains the service primitives, and a daemon-style executable links against it.
- **Deer (front-end):** A TCP-based interface that exposes localized socket access for the data managed by Bourne. It currently provides a GTK4 GUI and is designed to expand to multiple sensor sources.

## Service Overview
Bourne now exposes a modular, multi-thread-ready scaffold. The service initializes defaults, prepares the listening socket, wires a worker pool, and logs verbosely if socket setup or connection acceptance fails. The daemon runs continuously in its accept loop, only exiting when a `shutdown` command is received over the socket or when a `SIGINT`/`SIGTERM` arrives. The loop remains safe for multiple front-end instances and leaves room for richer protocol handling while storage and query design stabilize.

### Wire protocol
- `PING` → `PONG` for liveness checks.
- `SEARCH <query>` → `RESULT <count>` plus newline-delimited "title — summary" rows for each match drawn from disk-backed JSON (falling back to built-in fixtures when the file is unavailable).
- `STORE <json>` → `STORED <message>` when a payload is indexed into the in-memory catalog, or `ERROR <reason>` when storage is
  full or validation fails.
- `SHUTDOWN` → `SHUTTING DOWN` followed by a graceful service stop.

The parser and validator live in `bourne/src/query.c`, while search execution is backed by `bourne/src/storage.c` to keep responsibilities separated and testable.

## Repository Layout
- `bourne/`: Back-end static library (`src/`), headers (`include/`), and service entry point (`service/`). Modules are split across config, logging, network, threading, storage, and lifecycle orchestration.
- `deer/`: Front-end modules (`src/`), headers (`include/`), and GUI entry point (`app/`). Core (config/client) logic is GUI-agnostic; GTK layout lives in a separate UI library. Sample data helpers live alongside the layout to feed the UI during development.
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
`deer_app` accepts an optional configuration path as its first argument. The file uses a simple `key=value` format and supports theming and layout toggles:

```
theme=DeerBourne Custom
color_background=#11202D
color_foreground=#EDEFF2
color_accent=#6BD9FF
show_history=false
show_saved_items=false
show_connection=true
show_sample_browser=true
```

If no configuration file is provided or parsing fails, the GUI falls back to the built-in "DeerBourne Modern" theme with all sections visible. The window exposes a color section where you can interactively adjust background, foreground, and accent colors at runtime. A top-level "Layout" menu offers checkboxes to toggle visibility for search, history, saved items, data entry, theme controls, connectivity probes, and the structured data browser; a reset action restores the configuration defaults.

### GUI layout
The GTK4 interface is organized for clarity and future growth:

- **Main search area:** A modern search entry for exploring data; queries are appended to the history list.
- **Structured data explorer:** A three-pane view showing list summaries, a relationships column, and detailed descriptors. It is populated from `data/sample_records.json` (falls back to built-in fixtures when JSON-GLib is absent).
- **History view:** A scrollable log of prior queries.
- **Saved items view:** A scrollable list of saved entries or responses.
- **Saved item entry:** A dedicated input and control to submit data for persistence.
- **Theme colors:** Controls to adjust background, foreground, and accent colors live.
- **Network connectivity:** A quick probe to verify the Deer client can reach the Bourne endpoint.
- **Protocol search panel:** The search button issues a `SEARCH` command to Bourne and appends the returned summaries into the saved items pane for quick inspection.
- **Layout controls:** Menu-driven toggles let you hide or show the core areas above; settings respect defaults from the config file and can be reset in-app.

### Sample data
The UI ships with a JSON file containing people, cars, and homes with descriptive attributes:

- Path: `data/sample_records.json`
- Structure: Array of objects with `title`, `summary`, `category` (person, car, home), `relationships` (strings linking records), and a `details` object (appearance, year, color, location, interests, possessions, etc.).
- Behavior: When present, the GUI loads the JSON and renders summaries, relationships, and detail panes. When the file is missing or JSON-GLib is unavailable, a built-in dataset is used so the layout always has content.

## Testing
With tests enabled (default), lightweight unit tests cover the configuration scaffolding for both Bourne and Deer, the protocol parser, storage search helpers, and a connectivity probe that exercises the `PING` path:

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
