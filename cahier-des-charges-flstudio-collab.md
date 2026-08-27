# Cahier des charges — Plugin de collaboration en temps réel pour FL Studio

## 1. Contexte et objectif

Permettre à deux musiciens de composer ensemble à distance sur FL Studio, chacun dans son propre projet, sans que le travail en cours de l'un s'affiche chez l'autre. Seul le contenu explicitement **validé** par un utilisateur est transmis et apparaît sur une piste dédiée chez l'autre.

**Principe central : modèle "brouillon privé + validation"**, à l'image d'un commit Git. Pas de mirroring d'écran, pas de synchronisation continue de l'état du projet — uniquement un push explicite de contenu fini.

---

## 2. Périmètre fonctionnel

### V1 (MVP — à construire en premier)
- Un plugin VST3 (JUCE), installable sur une piste FL Studio
- Bouton "Valider" dans l'interface du plugin : capture le pattern MIDI en cours sur la piste et l'envoie
- Réception : dès qu'une validation arrive de l'autre utilisateur, le plugin joue ce contenu sur sa piste, comme un instrument déclenché
- Connexion à un serveur relais central (un salon = deux utilisateurs)
- **Système d'invitation par code/lien** : celui qui ouvre une session génère un code court (ex. `XK4R-92`) ou un lien cliquable ; l'autre le saisit dans le plugin (ou clique le lien, qui pré-remplit le code) pour rejoindre le même salon
- Un seul sens de contenu géré au départ : MIDI (notes + vélocité + timing)

### V2 (une fois le MVP validé)
- Support de plusieurs pistes/patterns simultanés (pas juste une piste unique)
- Historique des validations (pouvoir revenir à une version précédente)
- Mode audio en plus du MIDI (envoi du buffer audio rendu, utile si les instruments diffèrent des deux côtés)
- Indicateur visuel "l'autre est en train de préparer quelque chose" (statut, sans révéler le contenu)

### Hors périmètre (pas prévu, à écarter explicitement)
- Édition partagée en direct de tout le projet (renommer pistes, réorganiser la playlist à distance) — FL Studio n'expose pas cette donnée via son SDK, ce n'est pas faisable proprement
- Plus de 2 utilisateurs simultanés (V1/V2)
- Mixage complet à distance (automation de tous les paramètres de mixer)

---

## 3. Architecture technique

### Vue d'ensemble

```
[FL Studio - Toi]                [FL Studio - Ton pote]
   Plugin JUCE (VST3)                Plugin JUCE (VST3)
   mode récepteur                    mode émetteur
        |                                  |
        └───────► Serveur relais ◄─────────┘
                  (WebSocket)
```

Les deux instances du plugin sont identiques dans le code, seul le rôle (émission/réception) change selon l'usage à un instant donné — dans la pratique, chacun aura probablement les deux rôles actifs en même temps (il valide vers toi, tu valides vers lui).

### Composant 1 — Le plugin FL Studio (JUCE, VST3)
- Langage : C++ avec le framework JUCE (gère la compatibilité VST3, l'audio, le MIDI et l'UI)
- Rôle émetteur :
  - Capture du buffer MIDI de la piste hôte pendant l'enregistrement/lecture
  - Bouton "Valider" qui fige la dernière prise et l'envoie au serveur
- Rôle récepteur :
  - Écoute la connexion WebSocket en tâche de fond (thread réseau séparé du thread audio, obligatoire pour ne pas bloquer l'audio)
  - Dès réception d'une validation, injecte les événements MIDI dans le moteur audio de FL Studio via la sortie du plugin, comme le ferait un instrument
- Interface minimale : statut de connexion, bouton Valider, nom du salon

### Composant 2 — Le serveur relais
- Rôle : relayer les messages entre les deux clients d'un même salon, ne fait aucun traitement musical
- Stack proposée : Node.js + WebSocket (ws) ou équivalent léger, hébergeable sur un petit VPS
- Stocke uniquement le dernier état validé par salon (pas d'historique en V1)
- Gère la création et l'attribution des salons (voir composant 4)

### Composant 4 — Système d'invitation (code/lien)
- Quand un utilisateur ouvre le plugin et clique "Créer une session", le serveur génère un code court aléatoire (ex. 6-8 caractères) associé à un salon vide
- Le plugin affiche ce code, et propose aussi un lien du type `https://tonapp.com/join/XK4R-92` (pratique à copier-coller dans WhatsApp)
- Le deuxième utilisateur saisit le code directement dans le champ du plugin, ou clique le lien — dans ce cas il faut qu'un petit mécanisme (deep link ou copier-coller manuel du code depuis une page web) transmette le code jusqu'au plugin, car un plugin audio ne peut pas "ouvrir" un lien tout seul
- Le salon expire automatiquement après une durée d'inactivité (ex. 24h) pour ne pas accumuler de salons morts sur le serveur
- Un salon = 2 participants max en V1

### Composant 3 — Protocole d'échange
- Format des messages : JSON pour les métadonnées (timestamp, piste concernée, utilisateur) + payload MIDI encodé (liste d'événements note-on/note-off avec timing relatif)
- Un message = un événement de validation complet (pas de streaming continu)

---

## 4. Découpage du projet en lots

| Lot | Contenu | Objectif |
|---|---|---|
| **Lot 0 — Setup** | Environnement JUCE, projet VST3 qui charge dans FL Studio (coquille vide) | Valider la chaîne de compilation et le chargement du plugin |
| **Lot 1 — Réseau minimal** | Serveur relais basique + plugin qui se connecte et échange un message test | Valider la connexion plugin ↔ serveur |
| **Lot 2 — Validation MIDI simple** | Bouton Valider qui capture une note test et la transmet, réception qui la rejoue | Valider le cœur du concept sur un cas trivial |
| **Lot 3 — Capture réelle** | Capture du vrai pattern MIDI joué/enregistré sur la piste, pas juste une note test | Rendre le plugin réellement utilisable |
| **Lot 4 — Invitation par code/lien** | Génération de code côté serveur, saisie côté plugin, page web simple pour le lien | Permettre de rejoindre une session sans échange technique manuel |
| **Lot 5 — Interface utilisateur** | UI propre : statut de connexion, code de salon affiché, historique visuel simple | Rendre l'outil agréable à utiliser à deux |
| **Lot 6 — Robustesse** | Gestion des déconnexions, reconnexion auto, erreurs réseau, salons expirés | Éviter que l'outil casse en usage réel |
| **Lot 7 (V2)** | Mode audio, multi-pistes, historique des versions | Extension une fois le MVP validé par l'usage |

---

## 5. Stack technique proposée

- **Plugin** : C++ / JUCE (framework standard pour plugins audio multiplateformes, gère VST3 nativement)
- **Réseau côté plugin** : bibliothèque WebSocket C++ compatible JUCE (JUCE a son propre module réseau, ou libwebsockets)
- **Serveur** : Node.js + ws, ou alternative légère (peut être remplacé plus tard par quelque chose de plus robuste si besoin)
- **Hébergement serveur** : un petit VPS suffit largement pour 2 utilisateurs (charge très faible)

---

## 6. Erreurs à anticiper

### Erreurs C++ / audio temps réel (les plus critiques)
- **Bloquer le thread audio** : erreur numéro 1. Appel réseau, log, ou allocation mémoire (`new`) directement dans la fonction audio → craquements voire crash de FL Studio. Traité dès le Lot 1 avec une file d'attente lock-free entre thread réseau et thread audio.
- **Fuites mémoire** : pas de ramasse-miettes en C++. Utilisation systématique des smart pointers JUCE pour éviter ça.
- **Race conditions** : deux threads qui touchent la même donnée sans protection → bugs aléatoires difficiles à reproduire. La queue lock-free du Lot 1 est un point fondateur, pas un détail traité plus tard.

### Erreurs spécifiques au projet
- **Dérive du timing réseau** : chaque PC a sa propre horloge ; sans repère commun, le MIDI reçu peut arriver décalé. Un timestamp relatif dans le protocole compense ça.
- **Tester en local uniquement** : tout marche "trop bien" quand les deux plugins tournent sur le même PC. Tests en vraies conditions réseau avec ton pote dès le Lot 2, pas après.
- **Déconnexion réseau non gérée** : si le serveur tombe ou la connexion coupe, le plugin ne doit ni planter ni geler FL Studio. Prévu formellement au Lot 6, mais pensé dès les premiers lots pour éviter de tout réécrire.
- **Format VST3 multiplateforme** : si vous êtes sur des OS différents (Windows/Mac), compilation séparée pour chaque plateforme à prévoir.
- **Code de salon prévisible ou trop court** : un code trop simple (ex. 4 chiffres) peut être deviné ou provoquer une collision avec un autre salon. Prévoir un code suffisamment long/aléatoire (Lot 4).
- **Salon abandonné qui reste ouvert** : sans expiration automatique, le serveur accumule des salons morts indéfiniment. Expiration après inactivité prévue au Lot 4.

### Erreurs de setup, avant même d'écrire du code
- Mauvaise configuration de l'environnement JUCE/VST3 SDK (chemins, versions de compilateur) : classique cause de blocage des premiers jours. Traité pas à pas au Lot 0.

---

## 7. Points de vigilance

- **Thread audio vs thread réseau** : toute opération réseau doit être hors du thread audio temps réel, sinon risque de craquements/latence audio. Point non négociable en dev JUCE.
- **FL Studio n'expose pas l'état du projet** : le plugin ne peut agir que via ce qu'il reçoit/envoie comme audio ou MIDI sur sa propre piste, pas au-delà.
- **Synchronisation des instruments** : en mode MIDI, si les deux ne jouent pas avec les mêmes instruments/samples sur la piste réceptrice, le rendu sonore ne sera pas identique à l'original. À anticiper dès le Lot 3.
- **Tests réels en duo** : prévoir des sessions de test avec ton pote dès le Lot 2, pas seulement en solo, pour valider l'usage concret et la latence perçue.
- **Un plugin ne peut pas ouvrir un lien lui-même** : le lien d'invitation doit passer par une page web légère qui affiche le code, que l'utilisateur recopie ensuite dans le plugin (pas d'ouverture automatique du plugin depuis un navigateur).

---

## 8. Résumé

Le projet consiste à développer un **plugin VST3 en C++/JUCE** installé sur une piste dans FL Studio des deux côtés, connecté à un **serveur relais léger**. Chaque utilisateur travaille en privé dans son propre projet ; un bouton "Valider" capture et envoie le contenu MIDI figé à l'autre, qui le reçoit et le joue automatiquement sur sa piste. La connexion entre les deux se fait via un **code de salon ou un lien** généré par celui qui crée la session. Aucune édition partagée en direct du projet n'est prévue, car FL Studio ne le permet pas techniquement — seule la validation ponctuelle de contenu MIDI transite entre les deux sessions.

Le développement est découpé en 7 lots progressifs, du plugin vide qui charge dans FL Studio jusqu'à une interface utilisable en usage réel, avec une V2 prévue pour étendre au mode audio et au multi-pistes une fois le concept validé. Une attention particulière est portée dès les premiers lots aux erreurs classiques de programmation audio temps réel (thread audio, timing réseau, robustesse des connexions).
