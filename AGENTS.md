# Reglas y Contexto: OpenLara PS3 Port

## 1. Entorno y Rutas (WSL + Disco F:)
- **Toolchain PS3:** Instalado en la raíz de `F:` (`/mnt/f/ps3toolchain` y `/mnt/f/psl1ght`).
- **Variables de entorno antes de compilar:**
  ```bash
  export PS3DEV=/mnt/f/ps3toolchain
  export PSL1GHT=/mnt/f/psl1ght
  export PATH=\$PATH:PS3DEV/bin:PS3DEV/ppu/bin:PS3DEV/spu/bin:PSL1GHT/bin
  ```
- **Carpeta de datos en PS3:** `/dev_hdd0/openlara_ps3/` (los assets pesados de audio, video y niveles van separados del PKG base).

## 2. Reglas de Compilación, Versionado y Respaldo
- **Versión base actual:** `v0.1` (PROHIBIDO subir a `v0.2` o `v0.3` sin autorización explícita mía).
- **Nomenclatura obligatoria de cada PKG generado:**
  - Formato: `OpenLara_PS3_v0.1_MMDD-HHMM.pkg` (usando fecha y hora actual de compilación).
  - Título interno (`TITLE` en `PARAM.SFO`): `OpenLara PS3 Port v0.1 (MMDD-HHMM)`.
- **Respaldo automático:** Guardar cada `.pkg` compilado en una carpeta `backups/` local para poder hacer rollback si algo falla.

## 3. Reglas de Código y Diagnóstico (C++ / PS3)
- **Logs obligatorios:** Todo PKG compilado debe escribir diagnóstico en `/dev_hdd0/openlara_ps3.log` con:
  - Timestamps exactos en cada línea.
  - Nombre exacto del PKG ejecutándose al inicio del log (para verificar que no sea un log viejo).
  - Un pulso **heartbeat cada 500ms** para detectar en qué función exacta se congela la PS3.
- **Arquitectura PS3:** Recordar que Cell PPU es **Big-Endian**; cuidar byte-swapping al leer archivos `.PHD` / `.TR1` y respetar los límites de RAM de la consola.

## 4. Despliegue a PS3 (FTP + webMAN MOD)
- **Consola:** PS3 Super Slim con HEN en IP `192.168.100.82`, puerto FTP `21`.
- **Flujo de prueba en consola (ejecutar solo si `make` tuvo éxito):**
  1. Borrar el log anterior en la PS3: `curl -s "ftp://192.168.100.82:21/dev_hdd0/openlara_ps3.log" -Q "DELE /dev_hdd0/openlara_ps3.log"`
  2. Subir el nuevo `.pkg` a `/dev_hdd0/packages/` vía FTP (`curl -T`).
  3. Opcionalmente instalar/reiniciar vía comandos webMAN MOD (`http://192.168.100.82/...`) si se solicita.

## 5. Modo de Trabajo Autónomo
- Tras editar cualquier archivo `.cpp` o `.h`, compila inmediatamente con `make` y corrige errores de sintaxis o enlazado sin pedirme permiso.
- Muestra bloques de código cortos (máximo 20 líneas) cuando expliques cambios.
