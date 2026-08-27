# 📘 Guide Complet - FL Studio Live Collaboration Plugin & Relay Server

Bienvenue dans le guide officiel de l'écosystème de collaboration temps réel pour **FL Studio** (et autres DAW compatibles). Ce document explique l'architecture, l'installation sur clé USB, l'utilisation quotidienne, ainsi que la procédure de compilation sur Windows et macOS.

---

## 🎵 1. Concept & Architecture

Contrairement au partage d'écran ou à la synchronisation automatique en continu, ce plugin repose sur le modèle **"Brouillon Privé + Validation Explicite"** (similaire au workflow de commits de Git) :

- **Étape 1** : Chaque musicien compose en privé dans son propre projet FL Studio sans perturber le collaborateur.
- **Étape 2** : Cliquez sur le bouton **`VALIDER PATTERN (PUSH LIVE)`** dans l'interface du plugin.
- **Étape 3** : Le pattern MIDI ou l'échantillon audio est transmis via le serveur relais WebSocket.
- **Étape 4** : Le collaborateur distant reçoit la prise validée et l'entend jouer instantanément dans son projet.

### 🏗️ Composants du Projet

```
.
├── FL_Studio_Collab_Portable/       # Pack portable prêt-à-l'emploi pour clé USB
├── FL_Studio_Collab_v1.0.0_Portable.zip # Archive ZIP portable prête pour téléchargement/USB
├── server/                          # Code source du Serveur Relais Node.js WebSocket
│   ├── src/server.js                # Gestionnaire de salons et routage du MIDI/Audio
│   └── package.json
├── dist/                            # Exécutable autonome du serveur relais (Node SEA)
│   └── FLStudioCollabServer.exe     # Ne nécessite aucune installation de Node.js !
└── plugin/                          # Code source du Plugin C++ (JUCE 7 + IXWebSocket)
    ├── CMakeLists.txt               # Fichier de configuration multiplateforme (Windows/macOS)
    └── Source/                      # Code source C++ (Processor, Editor, WebSocketClient)
```

---

## 💾 2. Guide Clé USB (Pack Portable Windows)

Vous pouvez transporter et exécuter l'ensemble du système sur n'importe quel ordinateur Windows depuis une simple clé USB, sans privilèges administrateur ni installation préalable.

### 📂 Structure du dossier sur la Clé USB (`FL_Studio_Collab_Portable`) :

1. **`📁 VST3/`** :
   - Contient le plugin `FL Studio Collab Plugin.vst3`.
   - **Utilisation** : Copiez ce fichier/dossier dans `C:\Program Files\Common Files\VST3\`, puis effectuez un *Scan plugins* dans FL Studio.
2. **`📁 Standalone/`** :
   - Contient l'application autonome `FL Studio Collab Plugin.exe`.
   - **Utilisation** : Utile pour tester l'interface ou collaborer sans même ouvrir un DAW.
3. **`📁 Server/` & `🚀 Lancer-Serveur.bat`** :
   - Contient `FLStudioCollabServer.exe` (Serveur autonome 92 Mo).
   - **Utilisation** : Double-cliquez sur `Lancer-Serveur.bat` pour démarrer le serveur relais local sur le port `8080`.

---

## 🎧 3. Guide d'Utilisation du Plugin

### 🔑 Connexion à un Salon
1. Lancez le serveur relais (ou connectez-vous à un serveur distant).
2. Ouvrez le plugin dans FL Studio.
3. **Créer une session** : Cliquez sur **`Créer une session`**. Le plugin générera un code unique (ex: `XK4R-92`).
4. **Rejoindre une session** : Entrez le code partagé par votre collaborateur dans la case prévue et cliquez sur **`Rejoindre`**.

### 🎛️ Modes de Transmission
- **🎹 Mode MIDI (Par défaut)** : Transmet la structure des notes (hauteur, vélocité, durée). Idéal pour partager des patterns que chaque collaborateur peut jouer avec ses propres instruments virtuels.
- **🔊 Mode Rendu Audio** : Capture et transmet le signal audio réel (format WAV PCM). Idéal pour envoyer des boucles de guitare, de chant ou des sons de synthétiseurs uniques.

### 📜 Historique & Rejeu
- Chaque prise reçue d'un collaborateur est automatiquement enregistrée dans l'**Historique des validations reçues**.
- Sélectionnez une prise dans le menu déroulant et cliquez sur **`Rejouer`** pour la réécouter à tout moment.

---

## 🛠️ 4. Guide de Compilation sous Windows

### 📋 Prérequis
- [CMake](https://cmake.org/download/) 3.22 ou supérieur.
- Un compilateur C++17 : **Visual Studio 2022** (avec le composant *Développement Desktop C++*) ou **MinGW-w64** (ex: `w64devkit`).

### ⚙️ Commandes de compilation (avec MinGW)
```powershell
# 1. Ajouter le compilateur au PATH
$env:PATH = "c:\Users\vhoeb\Desktop\CAMILLE CDL FL STUDIO\tools\w64devkit\bin;" + $env:PATH

# 2. Se placer dans le dossier du plugin
cd "c:\Users\vhoeb\Desktop\CAMILLE CDL FL STUDIO\plugin"

# 3. Configurer CMake (avec gestion des préprocesseurs pour chemins d'accès avec espaces)
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release "-DCMAKE_RC_FLAGS=--preprocessor=gcc --preprocessor-arg=-E --preprocessor-arg=-xc-header --preprocessor-arg=-DRC_INVOKED"

# 4. Lancer la compilation Release avec 4 threads
cmake --build build --config Release -j4
```

Les artéfacts générés seront situés dans :
- `plugin/build/FLStudioCollabPlugin_artefacts/Release/VST3/`
- `plugin/build/FLStudioCollabPlugin_artefacts/Release/Standalone/`

---

## 🍏 5. Guide de Compilation sous macOS

Le projet est 100% compatible macOS et génère les formats **VST3**, **AU (Audio Unit pour Logic Pro)** et **Standalone**.

### 📋 Prérequis sur Mac
- **Xcode** (disponible gratuitement sur le Mac App Store) et ses outils en ligne de commande (`xcode-select --install`).
- **CMake** (`brew install cmake`).

### ⚙️ Commandes de compilation sur Mac
```bash
cd plugin

# Générer les projets Xcode et compiler
cmake -B build -G Xcode
cmake --build build --config Release
```

Les fichiers générés sous macOS :
- `FL Studio Collab Plugin.vst3` -> `~/Library/Audio/Plug-Ins/VST3/`
- `FL Studio Collab Plugin.component` (Format AU) -> `~/Library/Audio/Plug-Ins/Components/`

---

## 🌐 6. GitHub & Liens de Téléchargement

Le code source et les guides sont synchronisés sur le dépôt officiel :
👉 **[Dépôt GitHub - FL Studio Collaboration Live](https://github.com/Nitrovinzleo/-FL-Studio--Plugin-2-collaboration-live-)**

### 📦 Télécharger le Package Portable USB depuis GitHub
1. Allez sur la page des [Releases du Dépôt GitHub](https://github.com/Nitrovinzleo/-FL-Studio--Plugin-2-collaboration-live-/releases).
2. Téléchargez l'archive **`FL_Studio_Collab_v1.0.0_Portable.zip`**.
3. Décompressez-la directement sur votre clé USB !
