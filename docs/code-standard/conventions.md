# Code Standards, Conventions & Embedded Patterns

## Fundamental Rule: 380KB RAM Constraint

ESP32-C3 has ~380KB usable DRAM and **no PSRAM**. Memory fragmentation or unexpected heap consumption will panic the device.

### 1. Memory Safety & Heap Protocol
- **`new` is not nothrow on ESP32**: Under `-fno-exceptions`, a failing `new` triggers `abort()`. Always use `makeUniqueNoThrow<T>()` from `lib/Memory/Memory.h` or `new (std::nothrow)`.
- **Heap Buffer Allocation**: Prefer `auto buf = makeUniqueNoThrow<uint8_t[]>(size);` over raw `malloc`. It guarantees automatic cleanup on early exits.
- **Stack Budget**: Function locals must stay under **256 bytes**. Stack frames are small; large buffers must live on the heap or static pools.
- **Pre-allocation (`std::vector`)**: Call `.reserve(N)` before executing loops with `push_back()` to avoid heap reallocation and memory fragmentation.
- **Strings**: Prohibit `std::string` and Arduino `String` in hot rendering paths. Use `std::string_view` for read-only access (caveat: check null termination before passing to C APIs) and fixed `char[]` buffers with `snprintf`.

### 2. Architecture & Hardware Rules
- **SdFat & Thread Safety**: Always use `HalStorage` (`Storage` macro) and `HalFile`. Never access `SdFat` or `SdSpiCard` directly; concurrent access without `storageMutex` trips FreeRTOS priority disinherit asserts.
- **`DESTRUCTOR_CLOSES_FILE=1`**: Local `HalFile` variables automatically close when going out of scope. Do not call `file.close()` on local variables unless reopening or deleting on the same path. Member file handles must be closed explicitly in `onExit()`.
- **RISC-V Alignment**: ESP32-C3 faults on unaligned multi-byte loads. Never cast a raw buffer to a pointer of a wider integer type. Always use `memcpy` for multi-byte deserialization.
- **Flash vs DRAM (`constexpr` First)**: Large lookup tables and static data must be `constexpr` or `static const` to reside on the flash bus, sparing DRAM. Any code executing during flash cache suspension (ISRs) must be annotated with `IRAM_ATTR`.

### 3. UI & Localization Conventions
- **Orientation Awareness**: Never hardcode dimensions (800 or 480). Query `renderer.getScreenWidth()`, `renderer.getScreenHeight()`, or `renderer.getOrientedViewableTRBL()`.
- **UI Theme Macro**: Direct all application drawing through `GUI` (`UITheme::getInstance()`).
- **Logical Buttons**: Use `MappedInputManager::Button::*` (e.g. `Button::Confirm`, `Button::Back`), never raw GPIO pin numbers.
- **I18n Strings**: All user-facing strings must use the `tr(STR_*)` macro with definitions in `lib/I18n/translations/english.yaml`. After editing YAML files, run:
  ```bash
  python3 scripts/gen_i18n.py lib/I18n/translations lib/I18n/
  ```

### 4. Code Formatting & Build Verification
- **Code Formatter**: Run the repository clang-format wrapper:
  ```bash
  ./bin/clang-format-fix -g
  ```
  Never invoke `clang-format` directly.
- **Compilation Check**:
  ```bash
  pio run
  ```

### 5. Release Protocol & Artifact Standards

Every release follows the standard upstream repository pattern (e.g. `v1.5.0`):

1. **Version Bump**: Update `version = X.Y.Z` in `platformio.ini` under `[crosspoint]`.
2. **Tag & Release Title**: `vX.Y.Z` (e.g. `v1.0.3`).
3. **Artifact Bundle**: Every GitHub release includes the production firmware binary for the target hardware:
   - `firmware.bin` (Production firmware for ESP32-C3 / Xteink X3/X4)
   - Multi-board assets: `firmware-sticky.bin`, `firmware-x4pro.bin`, `firmware-papermono.bin` (when compiled)
   - Flash components: `bootloader.bin`, `partitions.bin`
4. **Automated CI/CD Release**:
   Pushing any tag matching `v*` automatically triggers `.github/workflows/release.yml`, which compiles the matrix environments in parallel, creates/updates the GitHub Release with auto-generated changelog notes (`## What's Changed`), and attaches all binary artifacts.
5. **One-Command CLI Release**:
   Run `./bin/release vX.Y.Z` (or `python3 scripts/release.py vX.Y.Z`) to validate the git tree, build production binaries, push the tag, and publish the release with full changelog and attached binaries.
