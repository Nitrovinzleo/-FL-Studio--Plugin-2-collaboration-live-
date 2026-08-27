# FL Studio Live Collaboration Plugin & Relay Server

A real-time MIDI collaboration plugin for **FL Studio** built with **JUCE (C++)** and a lightweight **Node.js WebSocket relay server**.

## 🎵 Concept

Unlike screen mirroring or direct project syncing, this plugin uses a **"Private Draft + Explicit Validation"** model (similar to Git commits):
- Each musician composes privately in their own FL Studio project.
- Clicking **"Valider"** inside the plugin captures the track's MIDI pattern and pushes it to the relay server.
- The remote collaborator receives the validated MIDI pattern on their dedicated plugin track and hears it play back instantly.

---

## 📁 Repository Structure

```
.
├── cahier-des-charges-flstudio-collab.md  # Detailed specification document
├── server/                                # WebSocket relay server (Node.js)
│   ├── src/
│   │   └── server.js                      # Room management & MIDI payload forwarding
│   ├── test/
│   │   └── test-client.js                # Integration test simulating 2 clients
│   └── package.json
└── plugin/                                # JUCE VST3 C++ Plugin
    ├── CMakeLists.txt                     # CMake build configuration
    └── Source/
        ├── PluginProcessor.h / .cpp       # Audio & MIDI processing + lock-free queues
        └── PluginEditor.h / .cpp          # Modern JUCE UI with Room Join & Validate controls
```

---

## 🚀 Quick Start

### 1. Relay Server (`server/`)

Requires [Node.js](https://nodejs.org/) v18+:

```bash
cd server
npm install
npm run dev
```

To run integration tests:
```bash
npm test
```

### 2. JUCE VST3 Plugin (`plugin/`)

Requires CMake 3.22+ and a C++17 compiler (Visual Studio 2022 / GCC / Clang):

```bash
cd plugin
cmake -B build
cmake --build build --config Release
```

The resulting `.vst3` binary will be generated in `plugin/build/FLStudioCollabPlugin_artefacts/Release/VST3/`.

---

## 💾 Package Portable pour Clé USB

Un package complet et prêt-à-l'emploi sans installation est disponible pour être transporté sur clé USB :

### 📂 Contenu du dossier portable (`FL_Studio_Collab_Portable`) :
- **`VST3/`** : `FL Studio Collab Plugin.vst3` (Plugin à copier dans `C:\Program Files\Common Files\VST3\`)
- **`Standalone/`** : `FL Studio Collab Plugin.exe` (Application autonome Windows)
- **`Server/`** : `FLStudioCollabServer.exe` (Serveur Relais autonome Node SEA ne nécessitant aucune installation de Node.js)
- **`Lancer-Serveur.bat`** : Script de lancement rapide du serveur relais depuis la racine de la clé USB.

### 📥 Téléchargement depuis GitHub :
Vous pouvez télécharger directement l'archive ZIP prêt-à-l'emploi **`FL_Studio_Collab_v1.0.0_Portable.zip`** depuis la section [Releases du dépôt GitHub](https://github.com/Nitrovinzleo/-FL-Studio--Plugin-2-collaboration-live-/releases).

