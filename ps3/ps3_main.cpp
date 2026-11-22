#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <sysmodule/sysmodule.h>
#include <sysutil/video.h>
#include <SDL2/SDL.h>
#include "ps3_log.h"
#define _OS_PS3 1
#define _GAPI_SW 1
#define INV_QUALITY
#define INV_STEREO
#include "game.h"
#define WIN_W 1280
#define WIN_H 720
unsigned int startTime;
int osGetTimeMS() {
    static uint64 base = 0;
    uint64 tb = __builtin_ppc_get_timebase();
    if (!base) base = tb;
    return int((tb - base) / 79800ULL) + 1;
}
bool osJoyReady(int index) { return index == 0; }
void osJoyVibrate(int index, float L, float R) { (void)index; (void)L; (void)R; }
#define SND_FRAME_SIZE 4
#define SND_FRAMES 1024
Sound::Frame *sndData = NULL;
SDL_AudioDeviceID sdl_audiodev = 0;
void sndFill(void *ud, Uint8 *stream, int len) { (void)ud; (void)len; Sound::fill(sndData, SND_FRAMES); memcpy(stream, sndData, SND_FRAMES * SND_FRAME_SIZE); }
bool sndInit() {
    ps3SetStage("SND_INIT");
    SDL_AudioSpec d = {44100, AUDIO_S16SYS, 2, 0, SND_FRAMES, 0, 0, sndFill, NULL}, o;
    sdl_audiodev = SDL_OpenAudioDevice(NULL, 0, &d, &o, 0);
    if (!sdl_audiodev) { ps3Log("[AUDIO] Aviso: %s", SDL_GetError()); return false; }
    sndData = new Sound::Frame[SND_FRAMES]; memset(sndData, 0, SND_FRAMES * SND_FRAME_SIZE);
    SDL_PauseAudioDevice(sdl_audiodev, 0); ps3Log("[AUDIO] OK freq=%d", o.freq); return true;
}
void sndFree() { if (sdl_audiodev) { SDL_PauseAudioDevice(sdl_audiodev, 1); SDL_CloseAudioDevice(sdl_audiodev); } delete[] sndData; }
SDL_Joystick *sdl_joys[4]; int sdl_njoys = 0; vec2 joyL, joyR; uint32 joyBtns = 0;
JoyKey mapBtn(int b) {
    JoyKey m[] = {jkSelect,jkL,jkR,jkStart,jkUp,jkRight,jkDown,jkLeft,jkLT,jkRT,jkLB,jkRB,jkY,jkB,jkA,jkX};
    return (b >= 0 && b < 16) ? m[b] : jkNone;
}
float joyAxis(int v) { return (v > -8192 && v < 8192) ? 0.0f : v / 32767.0f; }
void inputInit() {
    ps3SetStage("INPUT_INIT"); joyL = joyR = vec2(0); sdl_njoys = SDL_NumJoysticks();
    ps3Log("[INPUT] Joysticks: %d", sdl_njoys);
    for (int i = 0; i < sdl_njoys && i < 4; i++) sdl_joys[i] = SDL_JoystickOpen(i);
}
void inputUpdate() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) Core::isQuit = true;
        else if (e.type == SDL_JOYBUTTONDOWN) {
            joyBtns |= (1 << e.jbutton.button); Input::setJoyDown(0, mapBtn(e.jbutton.button), 1);
            if (sdl_joys[0] && SDL_JoystickGetButton(sdl_joys[0], 0) && SDL_JoystickGetButton(sdl_joys[0], 3)) Core::isQuit = true;
        } else if (e.type == SDL_JOYBUTTONUP) { joyBtns &= ~(1 << e.jbutton.button); Input::setJoyDown(0, mapBtn(e.jbutton.button), 0); }
        else if (e.type == SDL_JOYAXISMOTION) {
            if (e.jaxis.axis == 0) joyL.x = joyAxis(e.jaxis.value); else if (e.jaxis.axis == 1) joyL.y = joyAxis(e.jaxis.value);
            else if (e.jaxis.axis == 2) joyR.x = joyAxis(e.jaxis.value); else if (e.jaxis.axis == 3) joyR.y = joyAxis(e.jaxis.value);
            Input::setJoyPos(0, jkL, joyL); Input::setJoyPos(0, jkR, joyR);
        }
    }
}
void chkFile(const char *p) { struct stat s; ps3Log(stat(p, &s) == 0 ? "[FILE] OK %s (%ldB)" : "[FILE] FALTA %s", p, (long)s.st_size); }
int main(int argc, char **argv) {
    (void)argc; (void)argv; ps3LogInit(); ps3SetStage("SYSMODULE");
    ps3Log("[SYS] PNGDEC=0x%x FS=0x%x", sysModuleLoad(SYSMODULE_PNGDEC), sysModuleLoad(SYSMODULE_FS));
    videoState st; videoResolution res;
    if (videoGetState(0, 0, &st) == 0 && videoGetResolution(st.displayMode.resolution, &res) == 0)
        ps3Log("[VIDEO] %dx%d state=%d", res.width, res.height, st.state);
    ps3SetStage("SDL_INIT");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) { ps3Log("[SDL] ERR: %s", SDL_GetError()); ps3LogFree(); return -1; }
    SDL_Window *win = SDL_CreateWindow("OpenLara", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_W, WIN_H, SDL_WINDOW_SHOWN);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren) ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
    SDL_Texture *tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIN_W, WIN_H);
    uint32 *pix = new uint32[WIN_W * WIN_H]; memset(pix, 0, WIN_W * WIN_H * 4);
    Core::width = WIN_W; Core::height = WIN_H; GAPI::swColor = pix; GAPI::resize();
    for (int i = 0; i < WIN_W * WIN_H; i++) pix[i] = 0xFF14344B;
    for (int y = 340; y < 380; y++) for (int x = 340; x < 940; x++) pix[y * WIN_W + x] = 0xFFD4AF37;
    for (int f = 0; f < 5; f++) { SDL_PumpEvents(); SDL_UpdateTexture(tex, NULL, pix, WIN_W * 4); SDL_RenderClear(ren); SDL_RenderCopy(ren, tex, NULL, NULL); SDL_RenderPresent(ren); SDL_Delay(30); }
    ps3Log("[VIDEO-TEST] Frame de arranque (azul/dorado) presentado en pantalla!");
    strcpy(contentDir, "/dev_hdd0/game/OPENLARA0/USRDIR/data/"); strcpy(cacheDir, "/dev_hdd0/openlara_ps3/"); strcpy(saveDir, cacheDir);
    chkFile("/dev_hdd0/game/OPENLARA0/USRDIR/data/level/1/LEVEL1.PHD");
    timeval t; gettimeofday(&t, NULL); startTime = t.tv_sec; sndInit(); inputInit();
    ps3SetStage("GAME_INIT"); Core::settings.audio.music = 20; Game::init((char*)"level/1/LEVEL1.PHD");
    ps3Log("[ENGINE] Game::init OK");
    ps3SetStage("MAIN_LOOP"); int fc = 0;
    ps3Log("[TEST] Intentando Game::render() directo...");
    Game::render();
    ps3Log("[TEST] Game::render() OK! Volcando a HDMI...");
    SDL_UpdateTexture(tex, NULL, pix, WIN_W * 4);
    SDL_RenderClear(ren); SDL_RenderCopy(ren, tex, NULL, NULL); SDL_RenderPresent(ren);
    float deathTimer = 0.0f;
    bool  wasDead    = false;
    while (!Core::isQuit) {
        inputUpdate();
        // Reinicio automatico al morir (evita state=8 permanente sin menu)
        {
            Lara *laraCheck = Game::level ? Game::level->players[0] : NULL;
            if (laraCheck && laraCheck->health <= 0.0f) {
                if (!wasDead) {
                    wasDead = true;
                    deathTimer = 0.0f;
                    ps3Log("[DEATH] Lara murio, reiniciando nivel en 2s...");
                }
                deathTimer += 0.016f;
                if (deathTimer > 2.0f) {
                    ps3Log("[DEATH] Reiniciando LEVEL1...");
                    wasDead = false;
                    deathTimer = 0.0f;
                    Game::init((char*)"level/1/LEVEL1.PHD");
                }
            } else {
                wasDead = false;
                deathTimer = 0.0f;
            }
        }
        if (Game::update()) {
            Game::render(); SDL_UpdateTexture(tex, NULL, pix, WIN_W * 4);
            SDL_RenderClear(ren); SDL_RenderCopy(ren, tex, NULL, NULL); SDL_RenderPresent(ren);
            if (++fc == 1 || fc % 30 == 0) {
                Lara *lara = Game::level ? Game::level->players[0] : NULL;
                float fl = -9999.0f; int secFloor = -999, secIdx = -1, secBelow = -1;
                if (lara && Game::level) {
                    int16 rIdx = (int16)lara->getRoomIndex();
                    TR::Room::Sector *sec = Game::level->level.getSector(rIdx, lara->pos);
                    if (sec) {
                        secFloor = (int)sec->floor;
                        secIdx   = (int)sec->floorIndex;
                        secBelow = (int)sec->roomBelow;
                        fl = Game::level->level.getFloor(sec, lara->pos);
                        if (fc == 1 && Game::level) {
                            TR::Level &lv = Game::level->level;
                            ps3Log("[DEBUG] floorsCount=%d secI=0x%04X secF=0x%02X", lv.floorsCount, (unsigned)sec->floorIndex, (unsigned)(uint8)sec->floor);
                            ps3Log("[DEBUG] floors[0]=0x%04X floors[1]=0x%04X floors[2]=0x%04X floors[3]=0x%04X",
                                (unsigned)lv.floors[0].value, (unsigned)lv.floors[1].value, (unsigned)lv.floors[2].value, (unsigned)lv.floors[3].value);
                            int idx = (int)sec->floorIndex;
                            if (idx >= 0 && idx + 1 < lv.floorsCount)
                                ps3Log("[DEBUG] floors[%d]=0x%04X floors[%d]=0x%04X", idx, (unsigned)lv.floors[idx].value, idx+1, (unsigned)lv.floors[idx+1].value);
                        }
                    }
                }
                if (lara && Game::level) {
                    TR::Animation *an = &Game::level->level.anims[Game::level->level.models[0].animation + lara->animation.index];
                    ps3Log("[SPEED] getSpeed=%.2f getDist=%.2f time=%.4f delta=%.4f vel=(%.1f,%.1f,%.1f)",
                        lara->animation.getSpeed(), lara->animation.getDist(),
                        lara->animation.time, Core::deltaTime,
                        lara->velocity.x, lara->velocity.y, lara->velocity.z);
                    ps3Log("[SPEED] getSpeed=%.2f getDist=%.2f time=%.4f delta=%.4f vel=(%.1f,%.1f,%.1f)",
                        lara->animation.getSpeed(), lara->animation.getDist(),
                        lara->animation.time, Core::deltaTime,
                        lara->velocity.x, lara->velocity.y, lara->velocity.z);
                    ps3Log("[ANIM] idx=%d state=%u speed=%d accel=%d frameRate=%u frameStart=%u frameEnd=%u acCount=%u",
                        lara->animation.index, (unsigned)an->state,
                        (int)(an->speed.value >> 16), (int)(an->accel.value >> 16),
                        (unsigned)an->frameRate, (unsigned)an->frameStart, (unsigned)an->frameEnd, (unsigned)an->acCount);
                }
                ps3Log("[GAME] F#%d | hp=%.0f stand=%d state=%d wpn=%d | pos=(%.0f,%.0f,%.0f) floorY=%.0f secF=%d secI=%d secB=%d vel=(%.1f,%.1f,%.1f) | btns=0x%X joyL=(%.2f,%.2f) joyR=(%.2f,%.2f)",
                    fc, lara ? lara->health : -1.0f, lara ? (int)lara->stand : -1, lara ? lara->state : -1, lara ? (int)lara->wpnState : -1,
                    lara ? lara->pos.x : 0.0f, lara ? lara->pos.y : 0.0f, lara ? lara->pos.z : 0.0f, fl, secFloor, secIdx, secBelow,
                    lara ? lara->velocity.x : 0.0f, lara ? lara->velocity.y : 0.0f, lara ? lara->velocity.z : 0.0f,
                    joyBtns, joyL.x, joyL.y, joyR.x, joyR.y);
            }
        }
    }
    sndFree(); Game::deinit(); delete[] pix; SDL_DestroyTexture(tex); SDL_DestroyRenderer(ren); SDL_DestroyWindow(win); SDL_Quit(); ps3LogFree();
    return 0;
}
void* osMutexInit() { return SDL_CreateMutex(); }
void  osMutexFree(void *obj) { if (obj) SDL_DestroyMutex((SDL_mutex*)obj); }
void  osMutexLock(void *obj) { if (obj) SDL_LockMutex((SDL_mutex*)obj); }
void  osMutexUnlock(void *obj) { if (obj) SDL_UnlockMutex((SDL_mutex*)obj); }
void* osRWLockInit() { return SDL_CreateMutex(); }
void  osRWLockFree(void *obj) { if (obj) SDL_DestroyMutex((SDL_mutex*)obj); }
void  osRWLockRead(void *obj) { if (obj) SDL_LockMutex((SDL_mutex*)obj); }
void  osRWUnlockRead(void *obj) { if (obj) SDL_UnlockMutex((SDL_mutex*)obj); }
void  osRWLockWrite(void *obj) { if (obj) SDL_LockMutex((SDL_mutex*)obj); }
void  osRWUnlockWrite(void *obj) { if (obj) SDL_UnlockMutex((SDL_mutex*)obj); }
