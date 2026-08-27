# FL Studio Live Collaboration Plugin & Relay Server

Plugin de collaboration MIDI & Audio en temps réel pour **FL Studio**, **Logic Pro**, **Ableton Live** et autres DAW.

---

## 🎵 Concept

Ce plugin utilise le principe du **"Brouillon Privé + Validation Explicite"** :
1. **Brouillon Privé** : Chaque musicien compose tranquillement dans son DAW sans perturber son collaborateur.
2. **Validation Explicite** : Cliquez sur **VALIDER PATTERN** pour envoyer les notes MIDI ou le son audio au collaborateur.
3. **Écoute instantanée** : Le collaborateur reçoit la prise validée et l'entend jouer en direct sur sa piste.

---

## 📖 Tutoriel d'Installation & Utilisation

<details open>
<summary><b>🪟 Windows (Tutoriel d'installation)</b></summary>

<br/>

### 1. Installation du Plugin VST3
- Téléchargez l'archive **`FL_Studio_Collab_v1.0.0_Portable.zip`** depuis les [Releases GitHub](https://github.com/Nitrovinzleo/-FL-Studio--Plugin-2-collaboration-live-/releases).
- Copiez le fichier `FL Studio Collab Plugin.vst3` dans votre dossier VST3 Windows :
  `C:\Program Files\Common Files\VST3\`
- Ouvrez FL Studio et lancez un **Scan plugins**.

### 2. Version Autonome (Sans FL Studio)
- Si vous souhaitez utiliser le logiciel directement sans DAW, lancez `FL Studio Collab Plugin.exe`.

### 3. Comment rejoindre une session
1. Ouvrez le plugin dans votre DAW.
2. **Créer une session** : Cliquez sur **`Créer une session`** pour obtenir votre code unique (ex: `XK4R-92`).
3. **Rejoindre** : Votre collaborateur entre `XK4R-92` et clique sur **`Rejoindre`**.
4. **C'est prêt !** Transmettez vos patterns en cliquant sur **`VALIDER PATTERN`**.

</details>

<br/>

<details open>
<summary><b>🍎 macOS (Tutoriel d'installation)</b></summary>

<br/>

### 1. Formats supportés sous Mac
Le plugin fonctionne sur Mac pour **FL Studio**, **Logic Pro**, **GarageBand**, **Ableton Live**, etc. :
- **VST3** -> `~/Library/Audio/Plug-Ins/VST3/`
- **AU (Audio Unit)** -> `~/Library/Audio/Plug-Ins/Components/`
- **Standalone** (Application autonome macOS `.app`)

### 2. Comment rejoindre une session
1. Ouvrez le plugin VST3 ou AU dans votre DAW Mac.
2. Cliquez sur **`Créer une session`** pour obtenir le code de salon (ex: `XK4R-92`).
3. Votre ami entre le code `XK4R-92` et clique sur **`Rejoindre`**.
4. La connexion s'établit instantanément pour la collaboration en direct.

</details>

---

## 📦 Package Portable (Clé USB)

L'archive **`FL_Studio_Collab_v1.0.0_Portable.zip`** contient les exécutables prêts à l'emploi :
- **`VST3/`** : Plugin `FL Studio Collab Plugin.vst3`
- **`Standalone/`** : Application `FL Studio Collab Plugin.exe`

---

<details>
<summary><b>🛠️ Section Développeur (Compilation depuis les sources)</b></summary>

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

</details>
