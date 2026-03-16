<p align="center">
  <img src="resources/icons/snappy.png" width="120" alt="Snappy Logo"/>
</p>

<h1 align="center">Snappy — Screenshot Tool</h1>

<p align="center">
  A lightweight, fast screenshot tool for designers & developers on Windows.<br/>
  Built with C++17 and Qt5. ~12MB installed, ~10–15MB RAM.
</p>

<p align="center">
  <img src="https://img.shields.io/badge/platform-Windows-blue" alt="Windows"/>
  <img src="https://img.shields.io/badge/language-C%2B%2B17-orange" alt="C++17"/>
  <img src="https://img.shields.io/badge/framework-Qt5-green" alt="Qt5"/>
  <img src="https://img.shields.io/badge/version-1.0.0-purple" alt="v1.0.0"/>
</p>

---

## Quick Start

1. Download the latest release from [Releases](../../releases)
2. Extract the `Snappy/` folder anywhere on your PC
3. Run `Snappy.exe`
4. The app lives in your system tray (bottom-right corner)

**Default shortcuts:**

| Action | Shortcut |
|--------|----------|
| Snip Screenshot | `Ctrl+Shift+1` |
| Scrolling Screenshot | `Ctrl+Shift+2` |
| Full Screen Screenshot | `Ctrl+Shift+3` |

> These shortcuts work globally — you can press them from any app.

---

## Features

### Capture Modes

- **Snip Screenshot** — Select any region of your screen. The screen freezes instantly, then you drag to select the area you want.
- **Full Screen** — Captures your entire screen in one click.
- **Scrolling Screenshot** — Select a region, then Snappy auto-scrolls and stitches the frames together. Press `Escape` to stop early. If nothing scrolls, it falls back to a full-screen capture.

### Editor Tools

Once you capture a screenshot, the editor opens with a horizontal toolbar at the top:

| Tool | Shortcut | Description |
|------|----------|-------------|
| Select | `V` | Click items to select them. Drag to select multiple. Click empty areas to pick colors. |
| Crop | `C` | Draw a rectangle to crop. Includes all annotations in the crop. |
| Copy | `Ctrl+C` | Copy the screenshot (with annotations) to clipboard. |
| Save | `Ctrl+S` | Save as PNG. |
| Undo / Redo | `Ctrl+Z` / `Ctrl+Y` | Undo/redo any action, including crops and deletes. |
| Arrow | `A` | Draw arrows with adjustable thickness. |
| Text | `T` | Click to create text. Click existing text to edit it. |
| Numbered | `N` | Drop numbered pins (1, 2, 3...) for step-by-step annotations. |
| Rectangle | `R` | Draw rectangles. |
| Ellipse | `E` | Draw ellipses/circles. |
| Line | `L` | Draw straight lines. |
| Pen | `P` | Freehand drawing. |
| Measure | `H` | Drag to measure pixel distances. Measurements stay on the image. |
| Blur | `B` | Pixelate sensitive areas. |
| Magnifier | — | Zoom lens that follows your cursor. Click to pick colors. |
| Delete | `Del` | Deletes selected items. If nothing is selected, deletes all annotations. |

### Real-Time Editing

- **Size slider** — Appears below the toolbar for tools that support it (Arrow, Text, Rect, Ellipse, Line, Pen, Numbered). Drag it to change the size of the item you just created — updates instantly.
- **Color picker** — Click the color swatch to open the color dialog. Changes apply to selected items and future drawings. The hex code label updates as you hover in Select mode.
- **Pick from image** — Click the 💧 button next to the color swatch, then click anywhere on the screenshot to pick that color.

### Navigation

- **Zoom** — `Ctrl+Scroll` to zoom in/out.
- **Pan** — Hold `Middle Mouse Button` or `Ctrl+Left Click` and drag to pan around.

### System Tray

Snappy runs in the system tray. Right-click for options:

- Snip Screenshot
- Full Screenshot
- Scrolling Capture
- Preferences
- About
- Quit

Double-click the tray icon to reopen the last screenshot with all your annotations.

---

## Preferences

Right-click the tray icon → **Preferences**:

### General Tab
- **Save folder** — Where screenshots are saved.
- **Auto-save** — Automatically save to the folder on capture.
- **Start minimized to tray** — Start the app without showing a window.
- **Start with Windows** — Launch Snappy automatically on login.

### Shortcuts Tab
- Customize the three global hotkeys (Snip, Scrolling, Full Screen).
- Click **Reset Defaults** to restore `Ctrl+Shift+1/2/3`.

---

## Building from Source

### Requirements

- **Qt 5.15+** (Core, Gui, Widgets, Svg)
- **C++17 compiler** (GCC 8+, MSVC 2017+, MinGW 8+)
- **CMake 3.16+** or **qmake**

### Build with CMake

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

### Build with qmake

```bash
qmake pawxel-clone.pro
make -j$(nproc)
```

### Cross-compile for Windows (from Linux)

```bash
# Install MinGW and Qt for Windows
mkdir build-win && cd build-win
cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain-mingw64.cmake \
  -DCMAKE_PREFIX_PATH=/path/to/qt/5.15.2/mingw81_64 \
  -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

---

## Project Structure

```
snappy/
├── src/
│   ├── main.cpp              # Entry point
│   ├── application.h/cpp     # Tray icon, hotkeys, app lifecycle
│   ├── screenshotmanager.h/cpp # Capture logic (snip, full, scrolling)
│   ├── snipoverlay.h/cpp     # Fullscreen selection overlay
│   ├── editorwindow.h/cpp    # Main editor window
│   ├── canvasview.h/cpp      # QGraphicsView canvas with zoom, tools, undo
│   ├── toolbar.h/cpp         # Horizontal toolbar with icon buttons
│   ├── annotationitems.h/cpp # All annotation types (Rect, Arrow, Text, etc.)
│   ├── preferencesdialog.h/cpp # Settings UI
│   └── globalhotkey.h/cpp    # Windows RegisterHotKey wrapper
├── resources/
│   ├── icons/
│   │   ├── snappy.png        # App icon (PNG)
│   │   └── icon.ico          # App icon (Windows ICO, multi-size)
│   ├── resources.qrc         # Qt resource file
│   └── snappy.rc             # Windows version info + icon embedding
├── CMakeLists.txt            # CMake build
├── pawxel-clone.pro          # qmake build
├── toolchain-mingw64.cmake   # Cross-compilation toolchain
└── README.md
```

---

## Tips for the Team

1. **Fastest workflow**: Press `Ctrl+Shift+1`, drag to select, annotate, press `Ctrl+C` to copy, paste into Slack/Docs.

2. **Measuring UI elements**: Select the Measure tool (`H`), drag between two points. The measurement stays on the screenshot and gets included when you save or copy.

3. **Step-by-step guides**: Use the Numbered tool (`N`) to drop pins, then add Text (`T`) labels next to each one.

4. **Redacting sensitive info**: Use Blur (`B`) to pixelate passwords, emails, or personal data before sharing.

5. **Color inspection**: In Select mode, hover over any pixel to see its hex code in the sidebar. Click empty areas to pick colors. Use the Magnifier for precision.

6. **Recovering from mistakes**: Undo (`Ctrl+Z`) works for everything — annotations, deletes, and even crops.

---

## License

MIT License. See [LICENSE](LICENSE) for details.

---

<p align="center">
  <sub>Built with ❤️ by Federico Martinez</sub>
</p>
