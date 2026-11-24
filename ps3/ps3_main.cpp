#include <string.h>
#include <malloc.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <io/pad.h>
#include <sysmodule/sysmodule.h>
#include <sysutil/sysutil.h>
#include <sysutil/video.h>
#include <rsx/rsx.h>
#include <rsx/gcm_sys.h>
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
int osGetTimeMS() { static uint64 base = 0; uint64 tb = __builtin_ppc_get_timebase(); if (!base) base = tb; return int((tb - base) / 79800ULL) + 1; }
bool osJoyReady(int index) { return index == 0; }
void osJoyVibrate(int index, float L, float R) { (void)index; (void)L; (void)R; }
gcmContextData *gcmCtx = NULL; uint32_t *gcmBuf[2]; uint32_t gcmOff[2]; int gcmCur = 0;
void gcmInitVideo() {
    void *host = memalign(1024*1024, 1024*1024); rsxInit(&gcmCtx, 0x10000, 1024*1024, host);
    videoConfiguration vc; memset(&vc, 0, sizeof(vc)); vc.resolution = VIDEO_RESOLUTION_720; vc.format = VIDEO_BUFFER_FORMAT_XRGB; vc.pitch = WIN_W * 4;
    videoConfigure(0, &vc, NULL, 0); gcmSetFlipMode(GCM_FLIP_VSYNC);
    for (int i = 0; i < 2; i++) { gcmBuf[i] = (uint32_t*)rsxMemalign(64, WIN_W*WIN_H*4); rsxAddressToOffset(gcmBuf[i], &gcmOff[i]); gcmSetDisplayBuffer(i, gcmOff[i], WIN_W*4, WIN_W, WIN_H); }
    gcmResetFlipStatus();
}
void gcmPresent(const uint32 *pix) {
    memcpy(gcmBuf[gcmCur], pix, WIN_W * WIN_H * 4);
    gcmSetFlip(gcmCtx, gcmCur); rsxFlushBuffer(gcmCtx);
    while (gcmGetFlipStatus() != 0) usleep(200);
    gcmResetFlipStatus(); gcmCur ^= 1;
}
#define SND_FRAME_SIZE 4
#define SND_FRAMES 1024
Sound::Frame *sndData = NULL; SDL_AudioDeviceID sdl_audiodev = 0;
void sndFill(void *ud, Uint8 *stream, int len) { (void)ud; (void)len; Sound::fill(sndData, SND_FRAMES); memcpy(stream, sndData, SND_FRAMES * SND_FRAME_SIZE); }
bool sndInit() {
    ps3SetStage("SND_INIT"); SDL_AudioSpec d = {44100, AUDIO_S16SYS, 2, 0, SND_FRAMES, 0, 0, sndFill, NULL}, o;
    sdl_audiodev = SDL_OpenAudioDevice(NULL, 0, &d, &o, 0); if (!sdl_audiodev) return false;
    sndData = new Sound::Frame[SND_FRAMES]; memset(sndData, 0, SND_FRAMES * SND_FRAME_SIZE); SDL_PauseAudioDevice(sdl_audiodev, 0); ps3Log("[AUDIO] OK freq=%d", o.freq); return true;
}
void sndFree() { if (sdl_audiodev) { SDL_PauseAudioDevice(sdl_audiodev, 1); SDL_CloseAudioDevice(sdl_audiodev); } delete[] sndData; }
vec2 joyL, joyR; uint32 joyBtns = 0;
float padAxis(uint8_t v) { int d = int(v) - 128; if (d > -32 && d < 32) return 0.0f; float f = d / 127.0f; return f < -1.0f ? -1.0f : (f > 1.0f ? 1.0f : f); }
void inputInit() { ps3SetStage("INPUT_INIT"); joyL = joyR = vec2(0); ioPadInit(7); ps3Log("[INPUT] ioPadInit OK"); }
void inputUpdate() {
    sysUtilCheckCallback(); padInfo pi; static padData pd; padData tmp; ioPadGetInfo(&pi);
    if (!pi.status[0]) return;
    if (ioPadGetData(0, &tmp) == 0 && tmp.len > 0) pd = tmp;
    joyBtns = (uint32(pd.button[2]) << 8) | uint32(pd.button[3]);
    Input::setJoyDown(0, jkA, pd.BTN_CROSS ? 1 : 0); Input::setJoyDown(0, jkX, pd.BTN_SQUARE ? 1 : 0);
    Input::setJoyDown(0, jkY, pd.BTN_TRIANGLE ? 1 : 0); Input::setJoyDown(0, jkB, pd.BTN_CIRCLE ? 1 : 0);
    Input::setJoyDown(0, jkLB, pd.BTN_L1 ? 1 : 0); Input::setJoyDown(0, jkRB, pd.BTN_R1 ? 1 : 0);
    Input::setJoyDown(0, jkLT, pd.BTN_L2 ? 1 : 0); Input::setJoyDown(0, jkRT, pd.BTN_R2 ? 1 : 0);
    Input::setJoyDown(0, jkUp, pd.BTN_UP ? 1 : 0); Input::setJoyDown(0, jkDown, pd.BTN_DOWN ? 1 : 0);
    Input::setJoyDown(0, jkLeft, pd.BTN_LEFT ? 1 : 0); Input::setJoyDown(0, jkRight, pd.BTN_RIGHT ? 1 : 0);
    Input::setJoyDown(0, jkSelect, pd.BTN_SELECT ? 1 : 0); Input::setJoyDown(0, jkStart, pd.BTN_START ? 1 : 0);
    bool hasAna = (pd.len >= 8) && !(pd.ANA_L_H == 0 && pd.ANA_L_V == 0 && pd.ANA_R_H == 0 && pd.ANA_R_V == 0); joyL = hasAna ? vec2(padAxis(pd.ANA_L_H), padAxis(pd.ANA_L_V)) : vec2(0.0f); joyR = hasAna ? vec2(padAxis(pd.ANA_R_H), padAxis(pd.ANA_R_V)) : vec2(0.0f);
    Input::setJoyPos(0, jkL, joyL); Input::setJoyPos(0, jkR, joyR);
    if (pd.BTN_L3 && pd.BTN_R3) ps3LogEnable();
    if (pd.BTN_SELECT && pd.BTN_START) Core::isQuit = true;
}
void chkFile(const char *p) { struct stat s; ps3Log(stat(p, &s) == 0 ? "[FILE] OK %s (%ldB)" : "[FILE] FALTA %s", p, (long)s.st_size); }
int main(int argc, char **argv) {
    (void)argc; (void)argv; ps3LogInit(); ps3SetStage("SYSMODULE");
    ps3Log("[SYS] PNGDEC=0x%x FS=0x%x", sysModuleLoad(SYSMODULE_PNGDEC), sysModuleLoad(SYSMODULE_FS));
    gcmInitVideo(); ps3SetStage("SDL_AUDIO"); SDL_Init(SDL_INIT_AUDIO);
    uint32 *pix = new uint32[WIN_W * WIN_H]; memset(pix, 0, WIN_W * WIN_H * 4);
    Core::width = WIN_W; Core::height = WIN_H; GAPI::swColor = pix; GAPI::resize();
    for (int i = 0; i < WIN_W * WIN_H; i++) pix[i] = 0xFF14344B;
    for (int y = 340; y < 380; y++) for (int x = 340; x < 940; x++) pix[y * WIN_W + x] = 0xFFD4AF37;
    gcmPresent(pix); ps3Log("[VIDEO-TEST] GCM nativo 1280x720 listo!");
    strcpy(contentDir, "/dev_hdd0/game/OPENLARA0/USRDIR/data/"); strcpy(cacheDir, "/dev_hdd0/openlara_ps3/"); strcpy(saveDir, cacheDir);
    chkFile("/dev_hdd0/game/OPENLARA0/USRDIR/data/level/1/LEVEL1.PHD");
    timeval t; gettimeofday(&t, NULL); startTime = t.tv_sec; sndInit(); inputInit();
    ps3SetStage("GAME_INIT"); Core::settings.audio.music = 20; Game::init((char*)"level/1/LEVEL1.PHD");
    ps3SetStage("MAIN_LOOP"); int fc = 0; Game::render(); gcmPresent(pix);
    while (!Core::isQuit) {
        inputUpdate();
        if (Game::update()) {
            Game::render(); gcmPresent(pix);
            if (++fc == 1 || fc % 30 == 0) {
                Lara *lara = Game::level ? Game::level->players[0] : NULL;
                ps3Log("[GAME] F#%d | hp=%.0f stand=%d state=%d | pos=(%.0f,%.0f,%.0f) vel=(%.1f,%.1f,%.1f) | btns=0x%X joyL=(%.2f,%.2f)",
                    fc, lara ? lara->health : -1.0f, lara ? (int)lara->stand : -1, lara ? lara->state : -1,
                    lara ? lara->pos.x : 0.0f, lara ? lara->pos.y : 0.0f, lara ? lara->pos.z : 0.0f,
                    lara ? lara->velocity.x : 0.0f, lara ? lara->velocity.y : 0.0f, lara ? lara->velocity.z : 0.0f,
                    joyBtns, joyL.x, joyL.y);
            }
        }
    }
    ioPadEnd(); sndFree(); Game::deinit(); delete[] pix; SDL_Quit(); ps3LogFree(); return 0;
}
void* osMutexInit() { return SDL_CreateMutex(); } void osMutexFree(void *o) { if (o) SDL_DestroyMutex((SDL_mutex*)o); }
void osMutexLock(void *o) { if (o) SDL_LockMutex((SDL_mutex*)o); } void osMutexUnlock(void *o) { if (o) SDL_UnlockMutex((SDL_mutex*)o); }
void* osRWLockInit() { return SDL_CreateMutex(); } void osRWLockFree(void *o) { if (o) SDL_DestroyMutex((SDL_mutex*)o); }
void osRWLockRead(void *o) { if (o) SDL_LockMutex((SDL_mutex*)o); } void osRWUnlockRead(void *o) { if (o) SDL_UnlockMutex((SDL_mutex*)o); }
void osRWLockWrite(void *o) { if (o) SDL_LockMutex((SDL_mutex*)o); } void osRWUnlockWrite(void *o) { if (o) SDL_UnlockMutex((SDL_mutex*)o); }
