# FL Studio Live Collaboration Plugin & Relay Server

Real-time MIDI collaboration plugin for FL Studio built with JUCE (C++) and a Node.js WebSocket relay server.

## Concept

This plugin uses a **Private Draft + Explicit Validation** model (similar to Git commits):
- Each musician composes privately in their own FL Studio project.
- Clicking **Valider** captures the track's MIDI/audio pattern and pushes it to the relay server.
- The remote collaborator receives the validated pattern on their dedicated plugin track.

---

## Repository Structure

```
.
├── cahier-des-charges-flstudio-collab.md  # Specification document
├── server/                                # WebSocket relay server (Node.js)
│   ├── src/
│   │   └── server.js                      # Room management & payload forwarding
│   ├── test/
│   │   └── test-client.js                # Integration tests
│   └── package.json
└── plugin/                                # JUCE C++ Plugin
    ├── CMakeLists.txt                     # CMake build configuration
    └── Source/                            # C++ source code
```

---

## Quick Start

### 1. Relay Server (`server/`)

Requires Node.js v18+:

```bash
cd server
npm install
npm run dev
```

To run integration tests:
```bash
npm test
```

### 2. JUCE Plugin (`plugin/`)

Requires CMake 3.22+ and a C++17 compiler (Visual Studio 2022 / GCC / Clang):

```bash
cd plugin
cmake -B build
cmake --build build --config Release
```

The output binaries (`.vst3`, `.exe` / `.component`) are generated in `plugin/build/FLStudioCollabPlugin_artefacts/Release/`.

---

## Package Portable (Clé USB)

Le dossier `FL_Studio_Collab_Portable` contient les exécutables précompilés sans installation pour Windows :

- **`VST3/`** : `FL Studio Collab Plugin.vst3` (à copier dans `C:\Program Files\Common Files\VST3\`)
- **`Standalone/`** : `FL Studio Collab Plugin.exe`
- **`Server/`** : `FLStudioCollabServer.exe` (exécutable Node SEA autonome)
- **`Lancer-Serveur.bat`** : script de lancement du serveur

L'archive **`FL_Studio_Collab_v1.0.0_Portable.zip`** est téléchargeable dans la section [Releases](https://github.com/Nitrovinzleo/-FL-Studio--Plugin-2-collaboration-live-/releases).
