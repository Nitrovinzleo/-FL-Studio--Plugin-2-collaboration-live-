# FL Studio Collaboration Plugin (JUCE VST3)

C++ VST3 audio plugin for FL Studio handling MIDI capture, room pairing UI, and MIDI pattern playback.

## Building with CMake

### Prerequisites
- [CMake](https://cmake.org/) v3.22+
- C++17 compatible compiler:
  - **Windows**: Visual Studio 2022 / Build Tools or MinGW-w64 / Clang
  - **macOS**: Xcode / Clang
  - **Linux**: GCC 9+ / Clang

### Build Steps

```bash
# Navigate to plugin directory
cd plugin

# Generate build files (JUCE 7 will be downloaded automatically via CMake FetchContent)
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build VST3 plugin target
cmake --build build --config Release
```

### Output Location
Upon compilation, the `.vst3` plugin bundle will be located at:
`build/FLStudioCollabPlugin_artefacts/Release/VST3/FL Studio Collab Plugin.vst3`

Copy this folder to your VST3 plugins directory (e.g. `C:\Program Files\Common Files\VST3\`) for FL Studio to detect it automatically.
