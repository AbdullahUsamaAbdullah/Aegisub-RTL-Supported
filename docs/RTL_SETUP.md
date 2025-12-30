# Enabling right-to-left editing and building with bidi-enabled wxWidgets/Scintilla

This guide explains why the **Use right-to-left interface layout** option can be disabled and how to build Aegisub with a wxWidgets/Scintilla stack that keeps the checkbox active and renders RTL text correctly.

## Why the checkbox is greyed out
- The preferences checkbox at **View → Options → Interface → Edit Box → Use right-to-left interface layout** is disabled when the subtitle edit control reports that Scintilla lacks bidirectional APIs (`wxSTC_BIDIRECTIONAL_*` or `SCI_SETBIDIRECTIONAL`). The tooltip warns that full RTL rendering requires a wxStyledTextCtrl built with bidi support.
- `SubsTextEditCtrl::SupportsBidirectionalRendering()` returns `false` if those Scintilla flags are missing, and the preferences page disables the option and appends the warning. A bidi-capable Scintilla build is therefore required for the toggle to be usable.

## Quick toggle reminder (once bidi support is present)
1. Open **View → Options…** (on macOS it may appear under the application menu as **Preferences**).
2. Go to **Interface → Edit Box**.
3. Check **Use right-to-left interface layout (recommended for RTL languages)**.
4. If you pick an RTL UI language and leave **RTL Layout Follows UI Language** enabled, Aegisub will also prompt to turn this on automatically.

## Building with bidirectional Scintilla support
You need a wxWidgets build that exposes Scintilla’s bidirectional APIs. The rest of Aegisub’s dependencies follow the platform-specific instructions already in `README.md`; the additional work is ensuring wxWidgets (and its bundled or external Scintilla) is bidi-enabled.

### Windows (DirectWrite)
1. Install the Windows build prerequisites from `README.md` (Visual Studio, Python 3, Meson, CMake, and optional tools if you need installers).
2. Build wxWidgets 3.2+ with a Scintilla that exposes `wxSTC_BIDIRECTIONAL_*`/`SCI_SETBIDIRECTIONAL`. The easiest path is using the bundled Scintilla with DirectWrite enabled (e.g., `nmake /f makefile.vc BUILD=release RUNTIME_LIBS=static USE_OPENGL=0`).
3. Point Meson to that wxWidgets installation and build Aegisub, for example:
   ```cmd
   meson setup build -Ddefault_library=static --buildtype=release
   ninja -C build
   ```
4. Launch Aegisub from `build\aegisub.exe` and confirm the RTL checkbox is enabled.

### Linux (GTK/Harfbuzz)
1. Install the dependencies listed in `README.md` for Debian-based systems (`meson`, `ninja-build`, `libwxgtk3.2-dev`, etc.). Prefer a wxGTK package that ships a modern Scintilla with bidi support; if your distro package lacks it, build wxWidgets 3.2 yourself with the bundled Scintilla.
2. Configure and build Aegisub against that wxWidgets:
   ```bash
   meson setup build --prefix=/usr/local --buildtype=release --strip -Dsystem_luajit=false -Ddefault_library=static
   meson compile -C build
   sudo meson install -C build --skip-subprojects luajit
   ```
3. Start Aegisub and verify the **Use right-to-left interface layout** option is now clickable.

### macOS
1. Install the macOS prerequisites from `README.md` (e.g., `brew install meson cmake ninja pkg-config libass boost zlib ffms2 fftw hunspell uchardet`).
2. Build or install wxWidgets 3.2 that includes Scintilla bidi support. If Homebrew’s wxWidgets lacks bidi-enabled Scintilla, build wxWidgets from source with the bundled Scintilla before building Aegisub.
3. Build Aegisub with Meson:
   ```bash
   meson setup build && meson compile -C build
   ```
4. Open **Preferences → Interface → Edit Box** to confirm the RTL checkbox is available.

## Troubleshooting checklist
- If the checkbox remains disabled, double-check that your wxWidgets headers expose `wxSTC_BIDIRECTIONAL_R2L` (or `SCI_SETBIDIRECTIONAL` for older APIs). If they are missing, rebuild wxWidgets/Scintilla with bidi support.
- On Windows, ensure you picked the DirectWrite-enabled Scintilla technology; on GTK, confirm the build uses Harfbuzz shaping.
- After rebuilding wxWidgets, clean or recreate the Aegisub build directory so Meson picks up the new headers and libraries.
