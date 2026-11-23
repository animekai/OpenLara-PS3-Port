#include <stdio.h>
#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include <sys/process.h>
#include <sysutil/sysutil.h>
#include <SDL2/SDL.h>

void logBtn(const char* msg, int btn) {
    FILE *f = fopen("/dev_hdd0/openlara_ps3/mapper.log", "a");
    if (f) {
        fprintf(f, "[MAPPER] %s: %d\n", msg, btn);
        fclose(f);
    }
}

int main(int argc, char** argv) {
    system("mkdir -p /dev_hdd0/openlara_ps3");
    FILE *f = fopen("/dev_hdd0/openlara_ps3/mapper.log", "w");
    if (f) { fprintf(f, "[MAPPER] Iniciando Mapeador de Botones...\n"); fclose(f); }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) return -1;

    SDL_Window *win = SDL_CreateWindow("Mapper PS3", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
    
    int num_joys = SDL_NumJoysticks();
    logBtn("Joysticks detectados", num_joys);
    SDL_Joystick *joy = NULL;
    if (num_joys > 0) {
        joy = SDL_JoystickOpen(0);
        logBtn("Joystick abierto", 0);
    }

    bool running = true;
    SDL_Event e;
    Uint32 flashTimer = 0;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            } else if (e.type == SDL_JOYBUTTONDOWN) {
                logBtn("Boton PRESIONADO", e.jbutton.button);
                
                // Efecto visual: encender pantalla en verde por 150ms
                flashTimer = SDL_GetTicks() + 150;
                
                // Si es el botón 8 (Select), guardar y salir
                if (e.jbutton.button == 8) {
                    logBtn("Saliendo por Select", e.jbutton.button);
                    running = false;
                }
            }
        }
        
        // Dibujar pantalla
        if (SDL_GetTicks() < flashTimer) {
            SDL_SetRenderDrawColor(ren, 0, 255, 0, 255); // Verde cuando presionas
        } else {
            SDL_SetRenderDrawColor(ren, 20, 20, 20, 255); // Negro normal
        }
        SDL_RenderClear(ren);
        SDL_RenderPresent(ren);
        SDL_Delay(10);
    }

    if (joy) SDL_JoystickClose(joy);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
