# 🎮 OpenLara PS3 Port

[![PS3](https://img.shields.io/badge/Platform-PlayStation%203-blue)](https://www.playstation.com/)
[![PSL1GHT](https://img.shields.io/badge/SDK-PSL1GHT-green)](https://github.com/ps3dev/PSL1GHT)
[![C++](https://img.shields.io/badge/Language-C%2B%2B-orange)](https://isocpp.org/)
[![Status](https://img.shields.io/badge/Status-WIP-yellow)]()

Port no oficial de **OpenLara** (reimplementación open-source del motor de *Tomb Raider 1*) a **PlayStation 3** usando el SDK libre **PSL1GHT** y renderizador por software (`_GAPI_SW`).

> ⚠️ **Estado actual: WIP (Work In Progress).** Lara ya camina y gira a velocidad normal, pero el salto no funciona y el juego se congela tras unos minutos. **No es jugable de principio a fin todavía.**

---

## 📋 Tabla de contenidos

- [🎯 Objetivo del proyecto](#-objetivo-del-proyecto)
- [✅ Metas completadas](#-metas-completadas)
- [🚧 Metas pendientes](#-metas-pendientes)
- [🏗️ Arquitectura del port](#️-arquitectura-del-port)
- [🔧 Requisitos](#-requisitos)
- [📦 Instalación y compilación](#-instalación-y-compilación)
- [📁 Estructura de archivos](#-estructura-de-archivos)
- [🕹️ Controles](#️-controles)
- [🐛 Bugs conocidos](#-bugs-conocidos)
- [🤖 Desarrollo asistido por IA](#-desarrollo-asistido-por-ia)
- [📚 Proyectos y herramientas usadas](#-proyectos-y-herramientas-usadas)
- [🙏 Créditos](#-créditos)
- [📜 Licencia](#-licencia)

---

## 🎯 Objetivo del proyecto

Portar **OpenLara** (motor open-source de Tomb Raider 1, originalmente por [XProger](https://github.com/XProger/OpenLara)) a **PlayStation 3** usando:

- **PSL1GHT** (SDK libre para PS3, homebrew)
- **Renderizador por software** (`_GAPI_SW`) para evitar la complejidad de RSX/OpenGL ES
- **SDL2** para video, audio e input
- **libvorbis/ogg** para audio, **libpng** para texturas, **libgcm** para RSX

El objetivo final es tener **Tomb Raider 1 jugable de principio a fin** en PS3 con HEN, sin necesidad de hardware modificado.

---

## ✅ Metas completadas

| Meta | Estado | Notas |
|------|--------|-------|
| Compilación con `powerpc64-ps3-elf-g++` | ✅ | Toolchain PSL1GHT funcional |
| Empaquetado `.pkg` firmado | ✅ | `make pkg` genera `.pkg` instalable |
| Arranque y salida HDMI (1280x720) | ✅ | SDL2 + framebuffer |
| Carga de `LEVEL1.PHD` (2.5 MB) | ✅ | Desde `/dev_hdd0/game/OPENLARA0/USRDIR/data/` |
| Renderizado 3D por software | ✅ | ~1600 triángulos/cuadro, 25-30 FPS |
| Corrección de endianness (Big-Endian PPC64) | ✅ | `fixEndianPC()` con `swap16Arr`/`swap32Arr` |
| Carga de `FloorData` y colisión | ✅ | `secI=0x007B`, `floorY=3072`, `stand=1` |
| Lara de pie, camina, gira | ✅ | Velocidad normal (~47 u/frame) |
| Audio SDL2 (44.1 kHz) | ✅ | Música ambiental de cueva |
| Controles DualShock 3 (mapeo básico) | ✅ | Sticks, D-Pad, botones |
| Fix `fixed::operator float()` | ✅ | `value / 65536.0f` en vez de `H + L/65535` |

---

## 🚧 Metas pendientes

| Meta | Estado | Prioridad |
|------|--------|-----------|
| **Salto funcional** (`jkX`) | ❌ | 🔴 Alta |
| **Cuelgue tras 2-5 min** | ❌ | 🔴 Alta |
| **Botones de acción** (`jkA`, `jkB`, `jkY`) | ⚠️ | 🔴 Alta |
| Menú de Game Over / Load Game | ❌ | 🟡 Media |
| Pantallas de carga `.PNG` | ❌ | 🟡 Media |
| Videos `.RPL` | ❌ | 🟢 Baja |
| Sprites de rooms 10/26 (BE) | ⚠️ | 🟢 Baja |
| Modo Demo/Attract | ❌ | 🟢 Baja |
| Música de fondo (no solo ambiente) | ❌ | 🟢 Baja |
| Guardado/carga de partida | ❌ | 🟢 Baja |

---

## 🏗️ Arquitectura del port


### Doble paquete PKG

Para evitar re-empaquetar los assets (niveles, música) en cada prueba:

| Paquete | Contenido | Peso | Cuándo se actualiza |
|---------|-----------|------|---------------------|
| `OpenLara-PS3_Extra.pkg` | Assets (`USRDIR/data/`) | ~50 MB | Rara vez |
| `OpenLara-PS3_v0.2_MMDD-HHMM.pkg` | Código (`EBOOT.BIN`) | ~2.5 MB | Cada build |

Ambos con el **mismo Title ID** (`OPENLARA0`). El `Extra.pkg` se instala una vez; el `v0.2.pkg` se actualiza en cada iteración.

---

## 🔧 Requisitos

### Hardware

- **PlayStation 3** (Super Slim, Slim o Fat) con **HEN** (Homebrew Enabler) instalado
- **DualShock 3** (o Sixaxis)
- **Monitor/TV** con HDMI (probado a 1280x720)
- **FTP** habilitado en la PS3 (para transferir logs, opcional)

### Software (PC con WSL/Linux)

- **WSL Ubuntu** (o Linux nativo)
- **Toolchain PSL1GHT** en `/usr/local/ps3dev/`:
  - `powerpc64-ps3-elf-g++`
  - `sfo`, `pkg`, `package_finalize`
  - `/usr/local/ps3dev/ppu_rules`
- **SDL2** para PS3 (PSL1GHT port)
- **libvorbis**, **libogg**, **libpng**, **libgcm**
- **Git** (para clonar el repo)
- **Python 3** (para scripts de parcheo)

### Assets del juego

- **`LEVEL1.PHD`** (y los 21 niveles de TR1 PC) — **NO incluidos** por copyright
- **Música `.ogg`** (56 pistas) — **NO incluidas** por copyright
- **Videos `.RPL`** (11 videos) — **NO incluidos** por copyright

> **⚠️ Importante:** Este repo **solo contiene el código**. Los assets de Tomb Raider 1 son propiedad de Square Enix / Crystal Dynamics. **Debes obtenerlos por tu cuenta** (copia legal del juego original).

---

## 📦 Instalación y compilación

### 1. Clonar el repo

```bash
git clone https://github.com/animekai/OpenLara-PS3-Port.git
cd OpenLara-PS3-Port
# Ver instrucciones en https://github.com/ps3dev/PSL1GHT
mkdir -p pkg_extra/USRDIR/data/level/1
mkdir -p pkg_extra/USRDIR/data/audio/1
mkdir -p pkg_extra/USRDIR/data/video/1

# Copia los 21 niveles .PHD a pkg_extra/USRDIR/data/level/1/
# Copia las 56 pistas .ogg a pkg_extra/USRDIR/data/audio/1/
# Copia los 11 videos .RPL a pkg_extra/USRDIR/data/video/1/
cd /mnt/f/OpenLara-PS3
make pkg
Genera out/OpenLara-PS3_v0.2_MMDD-HHMM.pkg (código) y, si es la primera vez, out/OpenLara-PS3_Extra.pkg (assets).
5. Instalar en PS3
Copia los .pkg a un USB (FAT32) en /PS3/UPDATE/ o /PKG/

En la PS3: Package Manager → Install Package Files → USB

Instala primero OpenLara-PS3_Extra.pkg, luego OpenLara-PS3_v0.2_MMDD-HHMM.pkg

Arranca desde el XMB (icono de OpenLara)
6. Logs
El juego escribe un log en:

text
/dev_hdd0/openlara_ps3/openlara.log
Extráelo por FTP:

bash
curl -s "ftp://<IP_PS3>/dev_hdd0/openlara_ps3/openlara.log" -o openlara.log
📁 Estructura de archivos
text
OpenLara-PS3/
├── src/                    # Código del motor OpenLara
│   ├── format.h            # Carga de niveles, endianness
│   ├── lara.h              # Lógica de Lara
│   ├── game.h              # Bucle principal
│   ├── animation.h         # Animaciones
│   ├── core.h              # Core, deltaTime
│   ├── controller.h        # Controladores
│   ├── level.h             # Niveles
│   ├── sound.h             # Audio
│   ├── input.h             # Input
│   └── ...
├── ps3/                    # Código de plataforma PS3
│   ├── ps3_main.cpp        # main(), SDL2, input
│   └── ps3_log.h           # Logging
├── pkg_extra/              # Assets (NO en git)
│   └── USRDIR/data/
│       ├── level/1/        # 21 niveles .PHD
│       ├── audio/1/        # 56 pistas .ogg
│       └── video/1/        # 11 videos .RPL
├── out/                    # PKGs generados (NO en git)
├── backups_funcionales/    # Backups (NO en git)
├── Makefile                # Build
├── README.md               # Este archivo
└── .gitignore              # Excluye assets, PKGs, backups

---

## 🕹️ Controles

### Mapeo actual (DualShock 3)

| Botón PS3 | Acción TR1 | Estado |
|-----------|-----------|--------|
| **Stick izquierdo** | Mover / girar | ✅ |
| **Stick derecho** | Cámara libre | ✅ |
| **D-Pad** | Mover (alternativo) | ✅ |
| **×** (botón 14) | Acción | ⚠️ |
| **□** (botón 15) | Saltar | ❌ **No funciona** |
| **△** (botón 12) | Sacar/guardar armas | ⚠️ |
| **○** (botón 13) | Rodar | ⚠️ |
| **R1** (botón 11) | Caminar | ⚠️ |
| **L1** (botón 10) | Mirar | ⚠️ |
| **R2/L2** (9/8) | Paso lateral | ⚠️ |
| **Select** (0) | Inventario | ⚠️ |
| **Start** (3) | Pausa | ⚠️ |

> **⚠️ El mapeo de botones está en revisión.** Algunos botones no funcionan o están invertidos. **Prioridad alta.**

---

## 🐛 Bugs conocidos

### 🔴 Críticos

1. **Salto no funciona.** Presionar □ no hace que Lara salte. **Mapeo de botones o `ANIM_CMD_JUMP` pendiente.**
2. **Cuelgue tras 2-5 min.** El motor se congela (HEARTBEAT sigue, pero el juego no responde). **Posible bucle en `Game::update()` o `getFloor`/`getSector`.**

### 🟡 Medios

3. **Botones de acción invertidos.** Rodar hacia atrás en vez de adelante.
4. **Sprites de rooms 10/26 mal en BE.** Avisos `! room X has wrong sprite 0`. No afecta al gameplay.
5. **Modo Demo/Attract** se activa a los ~8s de inactividad.

### 🟢 Menores

6. **Pantallas de carga `.PNG` no se muestran.** Omitidas temporalmente.
7. **Videos `.RPL` no se reproducen.** Omitidos temporalmente.
8. **Música de fondo no suena.** Solo ambiente.

---

## 🤖 Desarrollo asistido por IA

Este port fue desarrollado con **asistencia de IA** (vibecoding) usando:

- **Gemini** (Google) — análisis de código, diagnóstico de bugs, propuestas de fix
- **DeepSeek** — revisión de código, generación de parches, análisis de logs

**Metodología:** iteración rápida con logs de telemetría, diagnóstico de causa raíz, fix quirúrgico, verificación en emulador (RPCS3) y hardware real (PS3 Super Slim con HEN).

**Nota:** La IA **no reemplaza** el conocimiento del motor. Los bugs de endianness (Big-Endian PPC64 vs Little-Endian PC) requirieron análisis manual y verificación con logs.

---

## 📚 Proyectos y herramientas usadas

### Proyecto original

- **[OpenLara](https://github.com/XProger/OpenLara)** por [XProger](https://github.com/XProger) — reimplementación open-source del motor de Tomb Raider 1.

### SDK y librerías

- **[PSL1GHT](https://github.com/ps3dev/PSL1GHT)** — SDK libre para PS3.
- **[SDL2](https://www.libsdl.org/)** — video, audio, input.
- **[libvorbis](https://xiph.org/vorbis/) / [libogg](https://xiph.org/ogg/)** — audio Ogg Vorbis.
- **[libpng](http://www.libpng.org/)** — texturas PNG.
- **[libgcm](https://github.com/ps3dev/PSL1GHT)** — RSX (no usado en este port, renderizado software).

### Herramientas de desarrollo

- **WSL Ubuntu** — entorno de compilación.
- **RPCS3** — emulador de PS3 (para pruebas rápidas).
- **PS3 Super Slim + HEN** — hardware real (para validación final).
- **Git** — control de versiones.

---

## 🙏 Créditos

- **XProger** — OpenLara original.
- **PSL1GHT team** — SDK para PS3.
- **Gemini + DeepSeek** — asistencia IA en el port.
- **Comunidad PS3 homebrew** — documentación y soporte.

---

## 📜 Licencia

Este port se distribuye bajo la **misma licencia que OpenLara original** (ver [LICENSE](LICENSE) en el repo original).

**Los assets de Tomb Raider 1** (niveles, música, videos) **son propiedad de Square Enix / Crystal Dynamics** y **NO están incluidos** en este repo. Debes obtenerlos por tu cuenta (copia legal del juego original).

---

## 📞 Contacto

- **GitHub:** [animekai](https://github.com/animekai)
- **Repo:** [OpenLara-PS3-Port](https://github.com/animekai/OpenLara-PS3-Port)

---

**⭐ Si te gusta el proyecto, dale una estrella en GitHub!**
