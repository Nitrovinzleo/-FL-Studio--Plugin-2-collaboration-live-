# FL Studio Live Collaboration Plugin & Relay Server

Real-time MIDI & Audio collaboration system for **FL Studio** (and other DAWs) built with **JUCE (C++)** and a **Node.js WebSocket relay server**.

---

## 🎵 Concept

This plugin uses a **Private Draft + Explicit Validation** model (similar to Git commits):
1. **Private Editing**: Each musician composes privately in their own DAW project.
2. **Push Live**: Clicking **VALIDER PATTERN** captures the MIDI notes or audio render and transmits it to the relay server.
3. **Instant Playback**: The remote collaborator receives the validated pattern on their plugin track and hears it play back instantly.

---

## 📖 Tutoriel & Guide d'Installation (Windows & macOS)

<details open>
<summary><b>🪟 Windows (Tutoriel d'installation & compilation)</b></summary>

<br/>

### 1. Utilisation du Pack Portable USB (Sans installation)
- Téléchargez l'archive **`FL_Studio_Collab_v1.0.0_Portable.zip`** depuis les [Releases GitHub](https://github.com/Nitrovinzleo/-FL-Studio--Plugin-2-collaboration-live-/releases).
- Décompressez-la sur votre ordinateur ou votre clé USB.
- **Plugin VST3** : Copiez le dossier `VST3/FL Studio Collab Plugin.vst3` dans :
  `C:\Program Files\Common Files\VST3\`
  *(Puis faites "Scan plugins" dans FL Studio).*
- **Application Standalone** : Lancez directement `Standalone/FL Studio Collab Plugin.exe` sans ouvrir de DAW.
- **Serveur Relais** : Double-cliquez sur `Lancer-Serveur-Internet.bat` pour démarrer le serveur relais et générer l'adresse d'accès à distance.

### 2. Compilation depuis les sources sous Windows
**Prérequis** : CMake 3.22+ et MinGW-w64 (ex: `w64devkit`) ou Visual Studio 2022.

```powershell
# 1. Ajouter le compilateur au PATH (si vous utilisez w64devkit)
$env:PATH = "c:\Users\vhoeb\Desktop\CAMILLE CDL FL STUDIO\tools\w64devkit\bin;" + $env:PATH

# 2. Aller dans le dossier plugin
cd plugin

# 3. Configurer CMake avec les flags preprocessor windres
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release "-DCMAKE_RC_FLAGS=--preprocessor=gcc --preprocessor-arg=-E --preprocessor-arg=-xc-header --preprocessor-arg=-DRC_INVOKED"

# 4. Lancer la compilation (VST3 + Standalone .exe)
cmake --build build --config Release -j4
```
Les artéfacts générés se situent dans `plugin/build/FLStudioCollabPlugin_artefacts/Release/`.

</details>

<br/>

<details>
<summary><b>🍎 macOS (Tutoriel d'installation & compilation)</b></summary>

<br/>

### 1. Formats générés sous macOS
Sous macOS, la compilation génère automatiquement :
- **VST3** (FL Studio macOS, Ableton Live, Cubase, Bitwig) -> `~/Library/Audio/Plug-Ins/VST3/`
- **AU / Audio Unit** (Logic Pro, GarageBand) -> `~/Library/Audio/Plug-Ins/Components/`
- **Standalone** (Application autonome macOS `.app`)

### 2. Compilation depuis les sources sous macOS
**Prérequis** : Xcode (`xcode-select --install`) et CMake (`brew install cmake`).

```bash
# 1. Ouvrir le terminal et aller dans le dossier plugin
cd plugin

# 2. Générer le projet Xcode et compiler
cmake -B build -G Xcode
cmake --build build --config Release
```

### 3. Lancer le serveur relais sous macOS
```bash
cd server
npm install
npm start
```

</details>

---

## 🌐 Collaboration à distance (Chacun chez soi)

<details>
<summary><b>💬 Comment se connecter à distance entre 2 maisons</b></summary>

<br/>

1. **Option 1 (Instantanée)** :
   L'un d'entre vous lance `Lancer-Serveur-Internet.bat`. Le script démarre le serveur local et génère une adresse publique (ex: `https://xxxx.loca.lt`). Transmettez cette adresse et votre code de salon (ex: `XK4R-92`) à votre ami.

2. **Option 2 (Hébergement 24h/24 gratuit sur Render.com)** :
   Importez votre dépôt GitHub sur [Render.com](https://render.com) en créant un *Web Service* (Language: `Node`, Root Directory: `server`, Build: `npm install`, Start: `npm start`). Render utilise le fichier `render.yaml` pour maintenir votre serveur en ligne 24h/24.

</details>

---

## 📁 Repository Structure

```
.
├── GUIDE_UTILISATION_ET_COMPILATION.md  # Detailed technical documentation
├── cahier-des-charges-flstudio-collab.md  # Specification document
├── render.yaml                            # 1-Click Render.com Cloud deployment config
├── Lancer-Serveur-Internet.bat           # Automated server + public tunnel script
├── server/                                # WebSocket relay server (Node.js)
│   ├── src/server.js                      # Room management & payload forwarding
│   ├── test/test-client.js                # Integration tests
│   └── package.json
└── plugin/                                # JUCE C++ Plugin (Windows & macOS)
    ├── CMakeLists.txt                     # CMake build configuration
    └── Source/                            # C++ source code
```

---

## 📦 Package Portable (Clé USB)

Le dossier `FL_Studio_Collab_Portable` contient les exécutables précompilés sans installation pour Windows :

- **`VST3/`** : `FL Studio Collab Plugin.vst3` (à copier dans `C:\Program Files\Common Files\VST3\`)
- **`Standalone/`** : `FL Studio Collab Plugin.exe`
- **`Server/`** : `FLStudioCollabServer.exe` (exécutable Node SEA autonome)
- **`Lancer-Serveur-Internet.bat`** : script de lancement du serveur relais et du tunnel public

L'archive **`FL_Studio_Collab_v1.0.0_Portable.zip`** est téléchargeable dans la section [Releases](https://github.com/Nitrovinzleo/-FL-Studio--Plugin-2-collaboration-live-/releases).
