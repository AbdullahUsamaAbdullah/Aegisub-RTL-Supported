# Enabling right-to-left editing and building with bidi-enabled wxWidgets/Scintilla

This guide explains why the **Use right-to-left interface layout** option can be disabled and how to build Aegisub with a wxWidgets/Scintilla stack that keeps the checkbox active and renders RTL text correctly.

## Why the checkbox is greyed out
- The preferences checkbox at **View → Options → Interface → Edit Box → Use right-to-left interface layout** is disabled when the subtitle edit control reports that Scintilla lacks bidirectional APIs (`wxSTC_BIDIRECTIONAL_*` or `SCI_SETBIDIRECTIONAL`). The tooltip warns that full RTL rendering requires a wxStyledTextCtrl built with bidi support.
- `SubsTextEditCtrl::SupportsBidirectionalRendering()` now reflects the Meson configure check: if the detected wxWidgets/Scintilla headers do not expose those APIs, configuration fails (unless you enable the bundled fallback below) instead of producing a build that silently disables the toggle.

## Quick toggle reminder (once bidi support is present)
1. Open **View → Options…** (on macOS it may appear under the application menu as **Preferences**).
2. Go to **Interface → Edit Box**.
3. Check **Use right-to-left interface layout (recommended for RTL languages)**.
4. If you pick an RTL UI language and leave **RTL Layout Follows UI Language** enabled, Aegisub will also prompt to turn this on automatically.

## Building with bidirectional Scintilla support
You need a wxWidgets build that exposes Scintilla’s bidirectional APIs. The rest of Aegisub’s dependencies follow the platform-specific instructions already in `README.md`; the additional work is ensuring wxWidgets (and its bundled or external Scintilla) is bidi-enabled.

Meson now enforces this requirement during configuration:

- If a system wxWidgets is found with bidi-capable Scintilla headers, it is used.
- If the headers are missing `wxSTC_BIDIRECTIONAL_*`/`SCI_SETBIDIRECTIONAL`, configuration fails unless the bundled fallback is enabled.
- The bundled fallback is on by default: `-Dwx_bundled_bidi=true` builds wxWidgets/Scintilla as a subproject with bidi flags enabled. On Windows this also turns on DirectWrite/Direct2D; on other toolkits it keeps Harfbuzz shaping enabled. Disable it (`-Dwx_bundled_bidi=false`) only if you want configuration to hard-fail instead of using the bundled copy.
- You can still force extra DirectWrite toggles for the bundled build with `-Dwx_direct2d=true`.

### Windows (DirectWrite)
1. Install the Windows build prerequisites from `README.md` (Visual Studio, Python 3, Meson, CMake, and optional tools if you need installers).
2. Build wxWidgets 3.2+ with a Scintilla that exposes `wxSTC_BIDIRECTIONAL_*`/`SCI_SETBIDIRECTIONAL`. The easiest path is letting Meson use the bundled fallback (`-Dwx_bundled_bidi=true`, the default), which enables DirectWrite/Direct2D and the Scintilla bidi flags automatically. If you prefer a system install, build wxWidgets with those flags first.
3. Point Meson to that wxWidgets installation and build Aegisub (or just let it fall back to the bundled copy), for example:
   ```cmd
   meson setup build -Ddefault_library=static --buildtype=release
   ninja -C build
   ```
4. Launch Aegisub from `build\aegisub.exe` and confirm the RTL checkbox is enabled.

### Linux (GTK/Harfbuzz)
1. Install the dependencies listed in `README.md` for Debian-based systems (`meson`, `ninja-build`, `libwxgtk3.2-dev`, etc.). Prefer a wxGTK package that ships a modern Scintilla with bidi support; if your distro package lacks it, leave `-Dwx_bundled_bidi` enabled so Meson builds wxWidgets with the bundled Scintilla and Harfbuzz shaping for you.
2. Configure and build Aegisub against that wxWidgets (or rely on the bundled fallback):
   ```bash
   meson setup build --prefix=/usr/local --buildtype=release --strip -Dsystem_luajit=false -Ddefault_library=static
   meson compile -C build
   sudo meson install -C build --skip-subprojects luajit
   ```
3. Start Aegisub and verify the **Use right-to-left interface layout** option is now clickable.

### macOS
1. Install the macOS prerequisites from `README.md` (e.g., `brew install meson cmake ninja pkg-config libass boost zlib ffms2 fftw hunspell uchardet`).
2. Build or install wxWidgets 3.2 that includes Scintilla bidi support. If Homebrew’s wxWidgets lacks bidi-enabled Scintilla, keep `-Dwx_bundled_bidi=true` so Meson builds wxWidgets with the bundled Scintilla before building Aegisub.
3. Build Aegisub with Meson:
   ```bash
   meson setup build && meson compile -C build
   ```
4. Open **Preferences → Interface → Edit Box** to confirm the RTL checkbox is available.

## Troubleshooting checklist
- If the checkbox remains disabled, double-check that your wxWidgets headers expose `wxSTC_BIDIRECTIONAL_R2L` (or `SCI_SETBIDIRECTIONAL` for older APIs). If they are missing, rebuild wxWidgets/Scintilla with bidi support.
- On Windows, ensure you picked the DirectWrite-enabled Scintilla technology; on GTK, confirm the build uses Harfbuzz shaping.
- After rebuilding wxWidgets, clean or recreate the Aegisub build directory so Meson picks up the new headers and libraries.
