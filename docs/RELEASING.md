# Publishing a Release on GitHub

Follow these steps to create a new release and make the Windows build available for download.

## Step 1: Create the GitHub Repository

```bash
# From your local machine with Git installed
cd snappy
git init
git add .
git commit -m "Initial release — Snappy v1.0.0"
git branch -M main
git remote add origin https://github.com/YOUR_USERNAME/snappy.git
git push -u origin main
```

## Step 2: Create a Release

1. Go to your repo on GitHub
2. Click **Releases** (right sidebar) → **Create a new release**
3. Click **Choose a tag** → type `v1.0.0` → click **Create new tag**
4. Set the title to `Snappy v1.0.0`
5. In the description, paste:

```
## Snappy v1.0.0

A lightweight screenshot tool for Windows.

### Features
- Snip, Full Screen, and Scrolling capture modes
- Annotation tools: Arrow, Text, Numbered, Rectangle, Ellipse, Line, Pen, Blur, Measure
- Real-time size and color editing
- Pixel measurement tool
- Color picker with hex codes
- Magnifier with color picking
- Undo/Redo everything (including crops and deletes)
- Global hotkeys: Ctrl+Shift+1/2/3
- System tray integration
- ~12MB installed, ~10-15MB RAM usage

### Installation
1. Download `Snappy-v1.0.0-windows.zip` below
2. Extract anywhere on your PC
3. Run `Snappy.exe`
4. (Optional) Run `flush_icon_cache.bat` as admin to refresh the icon in Explorer

### Default Shortcuts
| Action | Shortcut |
|--------|----------|
| Snip Screenshot | Ctrl+Shift+1 |
| Scrolling Screenshot | Ctrl+Shift+2 |
| Full Screen Screenshot | Ctrl+Shift+3 |
```

6. Under **Attach binaries**, drag and drop the `snappy-windows.zip` file
7. Click **Publish release**

## Step 3: Share with Team

Share this link with your team:
```
https://github.com/YOUR_USERNAME/snappy/releases/latest
```

They can download the zip, extract it, and run `Snappy.exe` — no installation needed.

## Updating

For future releases:

```bash
# Make changes, then:
git add .
git commit -m "Description of changes"
git push

# Then create a new release on GitHub with the next version tag (v1.1.0, etc.)
```
