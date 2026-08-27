# FL Studio Live Collaboration Plugin & Relay Server

Plugin de collaboration MIDI & Audio en temps réel pour **FL Studio** (et autres DAW).

---

## 🚀 Utilisation Rapide (Pour les Musiciens)

Aucun compte, aucune inscription et aucune configuration requise !

1. **Télécharger le plugin** : Récupérez l'archive **`FL_Studio_Collab_v1.0.0_Portable.zip`** depuis les [Releases GitHub](https://github.com/Nitrovinzleo/-FL-Studio--Plugin-2-collaboration-live-/releases).
2. **Installer** : Copiez `FL Studio Collab Plugin.vst3` dans votre dossier VST3 (`C:\Program Files\Common Files\VST3\`).
3. **Créer une session** : Ouvrez le plugin dans FL Studio et cliquez sur **`Créer une session`** pour obtenir votre code unique (ex: `XK4R-92`).
4. **Rejoindre** : Votre ami entre le code `XK4R-92` dans son plugin et clique sur **`Rejoindre`**.
5. **Composer** : Cliquez sur **`VALIDER PATTERN`** pour envoyer vos prises en direct !

---

## 🎵 Concept

Ce plugin utilise le principe du **"Brouillon Privé + Validation Explicite"** :
- **Brouillon Privé** : Vous composez tranquillement dans votre FL Studio sans perturber votre ami.
- **Validation Explicite** : Cliquez sur **VALIDER PATTERN** pour envoyer les notes MIDI ou le son réel au collaborateur.
- **Écoute instantanée** : Le collaborateur reçoit la prise et l'entend jouer directement dans son projet.

---

## 📦 Package Portable (Clé USB)

L'archive **`FL_Studio_Collab_v1.0.0_Portable.zip`** contient :
- **`VST3/`** : Plugin `FL Studio Collab Plugin.vst3` pour FL Studio.
- **`Standalone/`** : Application `FL Studio Collab Plugin.exe` (utilisable directement sans ouvrir de DAW).

---

<details>
<summary><b>🛠️ Section Développeur (Compilation & Code Source)</b></summary>

<br/>

### Compilation sous Windows
```powershell
$env:PATH = "c:\Users\vhoeb\Desktop\CAMILLE CDL FL STUDIO\tools\w64devkit\bin;" + $env:PATH
cd plugin
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release "-DCMAKE_RC_FLAGS=--preprocessor=gcc --preprocessor-arg=-E --preprocessor-arg=-xc-header --preprocessor-arg=-DRC_INVOKED"
cmake --build build --config Release -j4
```

### Compilation sous macOS (VST3, AU, Standalone)
```bash
cd plugin
cmake -B build -G Xcode
cmake --build build --config Release
```

### Serveur Relais Node.js (`server/`)
```bash
cd server
npm install
npm start
```

</details>
