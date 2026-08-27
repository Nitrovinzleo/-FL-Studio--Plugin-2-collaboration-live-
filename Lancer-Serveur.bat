@echo off
title Serveur Relais FL Studio Collaboration Live
echo ====================================================
echo 🚀 Démarrage du Serveur Relais FL Studio Collab...
echo ====================================================
echo.
cd /d "%~dp0server"
node src/server.js
pause
