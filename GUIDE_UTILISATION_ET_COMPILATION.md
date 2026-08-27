# Guide Technique & Tutoriel - FL Studio Collaboration Live

Plugin VST3/AU pour la collaboration temps réel sur FL Studio, Logic Pro, Ableton Live et autres DAW.

---

## 1. Fonctionnement

Chaque utilisateur travaille localement sur son projet FL Studio. Lorsqu'un pattern MIDI ou une prise audio est prête :
1. Cliquez sur **VALIDER PATTERN**.
2. Les données sont envoyées via le serveur relais.
3. Le collaborateur distant reçoit la prise et l'écoute en direct sur sa piste dédiée.

---

## 2. Tutoriels par Système d'Exploitation

<details open>
<summary><b>🪟 Windows (Tutoriel d'installation & utilisation)</b></summary>

<br/>

### Installation
- **Plugin VST3** : Copier `FL Studio Collab Plugin.vst3` dans `C:\Program Files\Common Files\VST3\`
- **Standalone** : Application autonome `FL Studio Collab Plugin.exe`

### Utilisation
1. Ouvrir le plugin dans FL Studio (ou lancer le Standalone).
2. Cliquez sur **`Créer une session`** pour obtenir votre code unique (ex: `XK4R-92`).
3. Votre collaborateur saisit le code `XK4R-92` et clique sur **`Rejoindre`**.

### Compilation sous Windows (Développeur)
```powershell
$env:PATH = "c:\Users\vhoeb\Desktop\CAMILLE CDL FL STUDIO\tools\w64devkit\bin;" + $env:PATH
cd plugin
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release "-DCMAKE_RC_FLAGS=--preprocessor=gcc --preprocessor-arg=-E --preprocessor-arg=-xc-header --preprocessor-arg=-DRC_INVOKED"
cmake --build build --config Release -j4
```

</details>

<br/>

<details open>
<summary><b>🍎 macOS (Tutoriel d'installation & utilisation)</b></summary>

<br/>

### Installation sous macOS
- **VST3** (FL Studio macOS, Ableton Live, Cubase, Bitwig) -> `~/Library/Audio/Plug-Ins/VST3/`
- **Audio Unit (AU)** (Logic Pro, GarageBand) -> `~/Library/Audio/Plug-Ins/Components/`
- **Standalone** (Application autonome macOS `.app`)

### Utilisation
1. Ouvrir le plugin VST3 ou AU dans votre DAW Mac.
2. Cliquez sur **`Créer une session`** pour obtenir votre code de salon.
3. Votre ami entre le code et clique sur **`Rejoindre`**.

### Compilation sous macOS (Développeur)
```bash
cd plugin
cmake -B build -G Xcode
cmake --build build --config Release
```

</details>
