# Snappy - Screenshot Tool

A lightweight, fast screenshot tool for Windows. Capture, annotate, measure, and share — all in under 15MB of RAM.

![Snappy](resources/icons/snappy.png)

## Features

**Capture**
- Snip screenshot (Ctrl+Shift+1) — select a region
- Scrolling capture (Ctrl+Shift+2) — auto-scroll and stitch
- Full screen (Ctrl+Shift+3)

**Annotate**
- Arrow, Rectangle, Ellipse, Line, Freehand pen
- Text with real-time size/color adjustment
- Numbered pins (resizable)
- Blur regions
- Measure/ruler tool with persistent measurements

**Edit**
- Crop (includes annotations in the crop)
- Undo/Redo (supports annotation, delete, and crop undo)
- Color picker — click to pick colors from the image
- Magnifier with live color info
- Middle-click or Ctrl+click to pan
- Ctrl+Scroll to zoom

**Preferences**
- Configurable global hotkeys
- Default color and per-tool width presets
- Auto-save to folder
- Start with Windows
- Start minimized to tray

## Download

Grab the latest release from the [Releases](https://github.com/federico187/snappy/releases) page. Extract the zip and run `Snappy.exe` — no installation needed.

## Build from Source

### Requirements
- Qt 5.15+ (Core, Gui, Widgets, Svg)
- C++17 compiler
- CMake 3.16+ or qmake

### Linux (native build for testing)
```bash
qmake snappy.pro
make -j$(nproc)
```

### Windows (cross-compile from Linux)
```bash
mkdir build-win && cd build-win
cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain-mingw64.cmake \
  -DCMAKE_PREFIX_PATH=/path/to/qt/5.15.2/mingw81_64 \
  -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

### Windows (native with MSVC or MinGW)
```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=C:/Qt/5.15.2/mingw81_64 -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

## Tech Stack

- C++17
- Qt 5.15 (Widgets, no QML)
- ~12MB installed, ~10-15MB RAM usage
- Single executable + Qt DLLs

## License

MIT License — see [LICENSE](LICENSE) for details.
