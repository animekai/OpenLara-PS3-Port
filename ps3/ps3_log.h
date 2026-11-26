#ifndef PS3_LOG_H
#define PS3_LOG_H
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <SDL2/SDL.h>
#ifndef PKG_BUILD_STAMP
#define PKG_BUILD_STAMP "v0.1 (dev)"
#endif
#ifndef PKG_FILE_NAME
#define PKG_FILE_NAME "OpenLara_PS3_v0.1.pkg"
#endif
static FILE *g_logFile = NULL;
static SDL_mutex *g_logMutex = NULL;
static volatile bool g_logEnabled = false;
static char g_stage[128] = "BOOT";
static inline void ps3SetStage(const char *s) { if (s) { strncpy(g_stage, s, 127); g_stage[127] = 0; } }
static inline void ps3Log(const char *fmt, ...) {
    if (!g_logEnabled) return;
    if (g_logMutex) SDL_LockMutex(g_logMutex);
    if (!g_logFile) { g_logFile = fopen("/dev_hdd0/openlara_ps3.log", "a"); }
    if (g_logFile) {
        time_t now = time(NULL); struct tm *t = localtime(&now); char tb[32] = "00:00:00";
        if (t) strftime(tb, sizeof(tb), "%H:%M:%S", t);
        fprintf(g_logFile, "[%s] [%s] ", tb, PKG_BUILD_STAMP);
        va_list a; va_start(a, fmt); vfprintf(g_logFile, fmt, a); va_end(a);
        fprintf(g_logFile, "\n"); fflush(g_logFile);
    }
    if (g_logMutex) SDL_UnlockMutex(g_logMutex);
}
static inline void ps3LogEnable() { if (!g_logEnabled) { g_logEnabled = true; ps3Log("=== LOG ACTIVADO POR USUARIO (L3+R3) ==="); } }
static inline void ps3LogInit() { FILE *f = fopen("/dev_hdd0/openlara_ps3.log", "w"); if (f) { fprintf(f, "PKG_FILE_NAME=%s\n", PKG_FILE_NAME); fclose(f); } g_logMutex = SDL_CreateMutex(); g_logEnabled = true; ps3Log("INICIO - %s", PKG_BUILD_STAMP); }
static inline void ps3LogFree() {
    if (g_logEnabled) ps3Log("CIERRE LIMPIO DEL SISTEMA - %s", PKG_BUILD_STAMP);
    if (g_logFile) { fclose(g_logFile); g_logFile = NULL; }
    if (g_logMutex) { SDL_DestroyMutex(g_logMutex); g_logMutex = NULL; }
}
static inline int ps3HeartbeatThread(void *data) {
    (void)data;
    while (g_logEnabled) {
        SDL_Delay(500);
        if (!g_logEnabled) break;
        ps3Log("[HEARTBEAT] stage=%s", g_stage);
    }
    return 0;
}
static inline void ps3HeartbeatStart() { SDL_CreateThread(ps3HeartbeatThread, "heartbeat", NULL); }
#endif