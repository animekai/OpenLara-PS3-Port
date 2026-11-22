@echo off
title PS3 FTP Manager - OpenLara PS3 Port (192.168.100.82)
:MENU
cls
echo === GESTOR FTP PS3: OpenLara PS3 Port (192.168.100.82) ===
echo  [COMBOS MANDO webMAN MOD]
echo   * Reiniciar PS3 (forzado/LV2): L3 + R2 + O (Circulo)
echo   * Reiniciar PS3 (suave/VSH)  : L3 + R1 + O (Circulo)
echo   * Apagar PS3    (forzado/LV2): L3 + R2 + X (Equis)
echo   * Apagar PS3    (suave/VSH)  : L3 + R1 + X (Equis)
echo ============================================================
echo 1. Enviar OpenLara-PS3*.pkg a /dev_hdd0/packages
echo 2. Extraer log a F:\openlara_ps3.log y borrar de PS3
echo 3. Cerrar juego y salir al XMB (xmb.ps3$exit)
echo 4. Reiniciar PS3 (restart.ps3)
echo 5. Apagar PS3 (shutdown.ps3)
echo 6. Borrar TODOS los .pkg en /dev_hdd0/packages
echo 7. Respaldar ultimo PKG en backups_funcionales
echo 8. Salir
set /p op="Opcion (1-8): "
if "%op%"=="1" goto SUBIR
if "%op%"=="2" goto BAJAR
if "%op%"=="3" goto SALIR_XMB
if "%op%"=="4" goto REINICIAR
if "%op%"=="5" goto APAGAR
if "%op%"=="6" goto LIMPIAR_PKG
if "%op%"=="7" goto BACKUP
if "%op%"=="8" exit
goto MENU
:SUBIR
for %%F in ("F:\OpenLara-PS3\out\OpenLara-PS3*.pkg") do set "PKGFILE=%%F"
(echo open 192.168.100.82 21 & echo user anonymous anonymous & echo binary & echo cd /dev_hdd0/packages & echo put "%PKGFILE%" & echo bye) > "%TEMP%\ftp.txt"
ftp -n -s:"%TEMP%\ftp.txt" & del "%TEMP%\ftp.txt" & pause & goto MENU
:BAJAR
if exist "%TEMP%\openlara_ps3.log" del /f /q "%TEMP%\openlara_ps3.log"
(echo open 192.168.100.82 21 & echo user anonymous anonymous & echo binary & echo get /dev_hdd0/openlara_ps3/openlara.log "%TEMP%\openlara_ps3.log" & echo delete /dev_hdd0/openlara_ps3/openlara.log & echo bye) > "%TEMP%\ftp.txt"
ftp -n -s:"%TEMP%\ftp.txt" & del "%TEMP%\ftp.txt"
if exist "%TEMP%\openlara_ps3.log" (move /y "%TEMP%\openlara_ps3.log" "F:\openlara_ps3.log" >nul & type "F:\openlara_ps3.log") else (echo Sin log nuevo)
pause & goto MENU
:SALIR_XMB
powershell -Command "try { Invoke-WebRequest -Uri 'http://192.168.100.82/xmb.ps3$exit' -TimeoutSec 3 -UseBasicParsing | Out-Null } catch {}"
pause & goto MENU
:REINICIAR
cls
echo Enviando orden de reinicio (restart.ps3) a 192.168.100.82...
powershell -Command "try { Invoke-WebRequest -Uri 'http://192.168.100.82/restart.ps3' -TimeoutSec 3 -UseBasicParsing | Out-Null } catch {}"
echo.
echo [OK] Senal enviada. La PS3 deberia reiniciarse en unos segundos.
echo.
pause
goto MENU
:APAGAR
powershell -Command "try { Invoke-WebRequest -Uri 'http://192.168.100/shutdown.ps3' -TimeoutSec 3 -UseBasicParsing | Out-Null } catch {}"
pause & goto MENU
:LIMPIAR_PKG
(echo open 192.168.100.82 21 & echo user anonymous anonymous & echo prompt off & echo cd /dev_hdd0/packages & echo mdelete *.pkg & echo bye) > "%TEMP%\ftp.txt"
ftp -n -s:"%TEMP%\ftp.txt" & del "%TEMP%\ftp.txt" & pause & goto MENU
:BACKUP
for %%F in ("F:\OpenLara-PS3\out\OpenLara-PS3*.pkg") do copy /y "%%F" "F:\OpenLara-PS3\backups_funcionales\"
pause & goto MENU
