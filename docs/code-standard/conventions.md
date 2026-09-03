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
