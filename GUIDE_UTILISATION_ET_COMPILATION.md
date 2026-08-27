# Guide Technique - FL Studio Collaboration Live

Plugin VST3/AU et serveur relais WebSocket pour la collaboration temps réel sur FL Studio.

---

## 1. Fonctionnement

Chaque utilisateur travaille localement sur son projet FL Studio. Lorsqu'un pattern MIDI ou une prise audio est prête :
1. Cliquez sur **VALIDER PATTERN**.
2. Les données sont envoyées au serveur relais via WebSocket.
3. Le collaborateur distant reçoit la prise et l'écoute en direct sur sa piste dédiée.

---

## 2. Version Portable (Clé USB)

Le dossier `FL_Studio_Collab_Portable` (et l'archive `.zip`) permet de lancer le système sur n'importe quel PC Windows sans installation :

- `VST3/` : Copier `FL Studio Collab Plugin.vst3` dans `C:\Program Files\Common Files\VST3\`
- `Standalone/` : Application autonome `FL Studio Collab Plugin.exe`
- `Server/` : Serveur relais autonome `FLStudioCollabServer.exe`
- `Lancer-Serveur.bat` : Lancer le serveur sur le port 8080.

---

## 3. Utilisation du Plugin

1. Démarrer le serveur relais (`Lancer-Serveur.bat` ou `node server/src/server.js`).
2. Charger le plugin dans FL Studio.
3. Créer un salon ou rejoindre avec un code de session (ex: `XK4R-92`).
4. Sélectionner le mode d'échange :
   - **MIDI** : transmission des notes, vélocités et durées.
   - **Rendu Audio** : transmission du signal audio WAV PCM.
5. Valider les patterns pour les envoyer au collaborateur.
6. Utiliser la liste déroulante pour rejouer les prises reçues dans l'historique.

---

## 4. Compilation sous Windows

### Prérequis
- CMake 3.22+
- MinGW-w64 ou Visual Studio 2022

### Commandes
```powershell
$env:PATH = "c:\Users\vhoeb\Desktop\CAMILLE CDL FL STUDIO\tools\w64devkit\bin;" + $env:PATH
cd plugin
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release "-DCMAKE_RC_FLAGS=--preprocessor=gcc --preprocessor-arg=-E --preprocessor-arg=-xc-header --preprocessor-arg=-DRC_INVOKED"
cmake --build build --config Release -j4
```

Fichiers générés dans `plugin/build/FLStudioCollabPlugin_artefacts/Release/`.

---

## 5. Compilation sous macOS

### Prérequis
- Xcode (`xcode-select --install`)
- CMake (`brew install cmake`)

### Commandes
```bash
cd plugin
cmake -B build -G Xcode
cmake --build build --config Release
```

Génère les formats VST3, Audio Unit (AU pour Logic Pro) et Standalone.
