//
// Created by elejandc on 12.02.24.
//

#ifndef LINUXVIDEOCAPTURES_WINDOWINTERFACE_H
#define LINUXVIDEOCAPTURES_WINDOWINTERFACE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <vector>
#include "VideoFrame.h"

class WindowInterface {
public:
    WindowInterface(int width, int heigth);
    ~WindowInterface();

    void framesToRender(std::vector<std::pair<std::pair<int,int>, UniversalImage>> &frames);
    void interface(volatile bool &work);

private:
    int WINDOW_WIDTH = 100;
    int WINDOW_HEIGTH = 100;

    SDL_Window *window = nullptr;
    SDL_Renderer *render = nullptr;

    int x_pos = 0;
    int y_pos = 0;

    const int max_x = 2000;
    const int max_y = 2000;
};

#endif //LINUXVIDEOCAPTURES_WINDOWINTERFACE_H
