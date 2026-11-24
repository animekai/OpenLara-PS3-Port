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
#define PKG_BUILD_STAMP "v0.2 (dev)"
#endif
static FILE *g_logFile = NULL;
static SDL_mutex *g_logMutex = NULL;
static volatile bool g_logEnabled = false;
static inline void ps3SetStage(const char *s) { (void)s; }
static inline void ps3Log(const char *fmt, ...) {
    if (!g_logEnabled) return;
    if (g_logMutex) SDL_LockMutex(g_logMutex);
    if (!g_logFile) { mkdir("/dev_hdd0/openlara_ps3", 0777); g_logFile = fopen("/dev_hdd0/openlara_ps3/openlara.log", "a"); }
    if (g_logFile) {
        time_t now = time(NULL); struct tm *t = localtime(&now); char tb[32] = "00:00:00";
        if (t) strftime(tb, sizeof(tb), "%H:%M:%S", t);
        fprintf(g_logFile, "[%s] [%s] ", tb, PKG_BUILD_STAMP);
        va_list a; va_start(a, fmt); vfprintf(g_logFile, fmt, a); va_end(a);
        fprintf(g_logFile, "\n"); fflush(g_logFile);
    }
    if (g_logMutex) SDL_UnlockMutex(g_logMutex);
}
static inline void ps3LogEnable() { if (!g_logEnabled) { g_logEnabled = false; ps3Log("=== LOG ACTIVADO POR USUARIO (L3+R3) ==="); } }
static inline void ps3LogInit() { mkdir("/dev_hdd0/openlara_ps3", 0777); FILE *f = fopen("/dev_hdd0/openlara_ps3/openlara.log", "w"); if (f) fclose(f); g_logMutex = SDL_CreateMutex(); g_logEnabled = true; ps3Log("INICIO - %s", PKG_BUILD_STAMP); }
static inline void ps3LogFree() {
    if (g_logEnabled) ps3Log("CIERRE LIMPIO DEL SISTEMA - %s", PKG_BUILD_STAMP);
    if (g_logFile) { fclose(g_logFile); g_logFile = NULL; }
    if (g_logMutex) { SDL_DestroyMutex(g_logMutex); g_logMutex = NULL; }
}
#endif
