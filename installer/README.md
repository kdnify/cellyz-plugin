# Cellyz Phone FX - Windows Installer

This directory contains the Inno Setup script for building the professional Windows installer.

## What it creates:
- **Cellyz_Phone_FX_v1.0_Setup.exe** - Professional Windows installer
- Automatically installs VST3 to correct system directory
- Includes uninstaller
- Modern installer UI with welcome screen

## Features:
- ✅ Installs to `C:\Program Files\Common Files\VST3\`
- ✅ Works with all VST3-compatible DAWs
- ✅ Proper Windows registry integration
- ✅ Professional uninstaller
- ✅ Modern installer interface

## Build Process:
The installer is automatically built by GitHub Actions when you push changes. You'll get both:
- `Cellyz.vst3` (plugin file for manual installation)
- `Cellyz_Phone_FX_v1.0_Setup.exe` (professional installer)

## Manual Build:
To build locally (requires Inno Setup):
```bash
iscc cellyz-installer.iss
```

## Plugin Features Highlighted in Installer:
- Nokia Classic Ring Tone Effect
- iPhone Voice Processing  
- Sony Ericsson Vintage Phone Sound 