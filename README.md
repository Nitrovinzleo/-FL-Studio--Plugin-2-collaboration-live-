# FL Studio Live Collaboration Plugin & Relay Server

Real-time MIDI & Audio collaboration system for **FL Studio** (and other DAWs) built with **JUCE (C++)** and a **Node.js WebSocket relay server**.

---

## 🎵 Concept

This plugin uses a **Private Draft + Explicit Validation** model (similar to Git commits):
1. **Private Editing**: Each musician composes privately in their own DAW project.
2. **Push Live**: Clicking **VALIDER PATTERN** captures the MIDI notes or audio render and transmits it to the relay server.
3. **Instant Playback**: The remote collaborator receives the validated pattern on their plugin track and hears it play back instantly.

---

## 🚀 Expérience Utilisateur (100% Automatique & Sans Compte)

Pour vos utilisateurs finaux (les musiciens), **tout est 100% automatique** :

1. **Aucun compte à créer** et aucune configuration de serveur requise.
2. Le plugin se connecte **automatiquement** au serveur relais en ligne au démarrage.
3. **Créer une session** : L'utilisateur 1 clique sur `Créer une session` et obtient son code unique (ex: `XK4R-92`).
4. **Rejoindre** : L'utilisateur 2 entre `XK4R-92` et clique sur `Rejoindre`.
5. **C'est tout !** Ils collaborent en direct depuis leurs maisons respectives.

---

## 📖 Tutoriel & Guide de Compilation (Windows & macOS)

<details open>
<summary><b>🪟 Windows (Tutoriel d'installation & compilation)</b></summary>

<br/>

### 1. Pack Portable USB (Prêt à l'emploi)
- Téléchargez **`FL_Studio_Collab_v1.0.0_Portable.zip`** depuis les [Releases GitHub](https://github.com/Nitrovinzleo/-FL-Studio--Plugin-2-collaboration-live-/releases).
- Décompressez l'archive.
- Copiez `VST3/FL Studio Collab Plugin.vst3` dans `C:\Program Files\Common Files\VST3\`.
- Lancez `Standalone/FL Studio Collab Plugin.exe` si vous souhaitez l'utiliser sans DAW.

### 2. Compilation depuis les sources sous Windows
**Prérequis** : CMake 3.22+ et MinGW-w64 (ex: `w64devkit`) ou Visual Studio 2022.

```powershell
$env:PATH = "c:\Users\vhoeb\Desktop\CAMILLE CDL FL STUDIO\tools\w64devkit\bin;" + $env:PATH
cd plugin
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release "-DCMAKE_RC_FLAGS=--preprocessor=gcc --preprocessor-arg=-E --preprocessor-arg=-xc-header --preprocessor-arg=-DRC_INVOKED"
cmake --build build --config Release -j4
```

</details>

<br/>

<details>
<summary><b>🍎 macOS (Tutoriel d'installation & compilation)</b></summary>

<br/>

### 1. Formats générés sous macOS
- **VST3** (FL Studio macOS, Ableton Live, Cubase, Bitwig) -> `~/Library/Audio/Plug-Ins/VST3/`
- **Audio Unit (AU)** (Logic Pro, GarageBand) -> `~/Library/Audio/Plug-Ins/Components/`
- **Standalone** (Application autonome macOS `.app`)

### 2. Compilation sous macOS
```bash
cd plugin
cmake -B build -G Xcode
cmake --build build --config Release
```

</details>

---

## 🛠️ Déploiement du Serveur Relais Cloud (Développeur)

Le serveur relais en ligne permet d'interconnecter tous les utilisateurs du monde entier.

- **Render.com (Hébergement 24h/24 Gratuit)** :
  1. Créez un Web Service sur Render.com lié au dépôt GitHub.
  2. Spécifiez : Language `Node`, Root Directory `server`, Build `npm install`, Start `npm start`.
  3. Render utilise le fichier `render.yaml` pour maintenir le serveur en ligne 24h/24.

---

## 📦 Package Portable (Clé USB)

Le dossier `FL_Studio_Collab_Portable` contient les exécutables précompilés sans installation pour Windows :

- **`VST3/`** : `FL Studio Collab Plugin.vst3`
- **`Standalone/`** : `FL Studio Collab Plugin.exe`
- **`Server/`** : `FLStudioCollabServer.exe`
- **`Lancer-Serveur-Internet.bat`** : script alternatif pour serveur local + tunnel public

Archive **`FL_Studio_Collab_v1.0.0_Portable.zip`** disponible dans la section [Releases](https://github.com/Nitrovinzleo/-FL-Studio--Plugin-2-collaboration-live-/releases).
