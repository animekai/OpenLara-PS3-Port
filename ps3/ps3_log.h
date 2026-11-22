#ifndef PS3_LOG_H
#define PS3_LOG_H
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <SDL2/SDL.h>
#ifndef PKG_BUILD_STAMP
#define PKG_BUILD_STAMP "v0.1 (dev)"
#endif
static FILE *g_logFile = NULL;
static SDL_mutex *g_logMutex = NULL;
static SDL_Thread *g_hbThread = NULL;
static volatile bool g_hbRunning = false;
static volatile int g_hbCount = 0;
static char g_currentStage[128] = "BOOT";
static inline void ps3SetStage(const char *stage) {
    if (g_logMutex) SDL_LockMutex(g_logMutex);
    strncpy(g_currentStage, stage, sizeof(g_currentStage) - 1);
    g_currentStage[sizeof(g_currentStage) - 1] = '\0';
    if (g_logMutex) SDL_UnlockMutex(g_logMutex);
}
static inline void ps3Log(const char *fmt, ...) {
    if (g_logMutex) SDL_LockMutex(g_logMutex);
    if (!g_logFile) {
        mkdir("/dev_hdd0/openlara_ps3", 0777);
        g_logFile = fopen("/dev_hdd0/openlara_ps3/openlara.log", "a");
    }
    if (g_logFile) {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char tbuf[32] = "0000-00-00 00:00:00";
        if (t) strftime(tbuf, sizeof(tbuf), "%Y-%m-%d %H:%M:%S", t);
        fprintf(g_logFile, "[%s] [%s] ", tbuf, PKG_BUILD_STAMP);
        va_list args; va_start(args, fmt); vfprintf(g_logFile, fmt, args); va_end(args);
        fprintf(g_logFile, "\n"); fflush(g_logFile);
    }
    if (g_logMutex) SDL_UnlockMutex(g_logMutex);
}
static int ps3HeartbeatThread(void *data) {
    (void)data;
    while (g_hbRunning) {
        SDL_Delay(500);
        if (!g_hbRunning) break;
        ps3Log("[HEARTBEAT #%d | 500ms] STAGE=%s", ++g_hbCount, g_currentStage);
    }
    return 0;
}
static inline void ps3LogInit() {
    mkdir("/dev_hdd0/openlara_ps3", 0777);
    g_logMutex = SDL_CreateMutex();
    FILE *f = fopen("/dev_hdd0/openlara_ps3/openlara.log", "w");
    if (f) fclose(f);
    ps3Log("INICIO DE EJECUCION - %s", PKG_BUILD_STAMP);
    g_hbRunning = true;
    g_hbThread = SDL_CreateThread(ps3HeartbeatThread, "PS3_HB_500ms", NULL);
}
static inline void ps3LogFree() {
    g_hbRunning = false;
    if (g_hbThread) { SDL_WaitThread(g_hbThread, NULL); g_hbThread = NULL; }
    ps3Log("CIERRE LIMPIO DEL SISTEMA - %s", PKG_BUILD_STAMP);
    if (g_logFile) { fclose(g_logFile); g_logFile = NULL; }
    if (g_logMutex) { SDL_DestroyMutex(g_logMutex); g_logMutex = NULL; }
}
#endif
