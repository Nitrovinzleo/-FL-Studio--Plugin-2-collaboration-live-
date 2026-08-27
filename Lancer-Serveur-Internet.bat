@echo off
title Serveur Relais FL Studio Collaboration (ACCES INTERNET PUBLIC)
echo ====================================================
echo 🚀 Démarrage du Serveur Relais + Tunnel Internet Public...
echo ====================================================
echo.

cd /d "%~dp0server"

start "Serveur Relais" cmd /k "node src/server.js"
timeout /t 2 /nobreak >nul

echo.
echo 🌐 Génération du lien d'accès Internet Public avec localtunnel...
echo (Partagez l'adresse générée ci-dessous avec vos amis à distance)
echo ====================================================
npx localtunnel --port 8080
pause
