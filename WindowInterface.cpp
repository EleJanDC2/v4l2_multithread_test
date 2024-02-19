//
// Created by elejandc on 12.02.24.
//

#include <iostream>
#include "WindowInterface.h"

#define DEBUG false

WindowInterface::WindowInterface(int width, int heigth) {
    this->WINDOW_WIDTH = width;
    this->WINDOW_HEIGTH = heigth;


    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        if(DEBUG) std::cerr << "SDL_INIT_EVERYTHING error: " << SDL_GetError() << std::endl;
        return;
    }
    //
    window = SDL_CreateWindow("Interface", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, this->WINDOW_WIDTH, this->WINDOW_HEIGTH,
                              SDL_WINDOW_SHOWN);
    //window = SDL_CreateWindow("tytul", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGTH, SDL_WINDOW_BORDERLESS);
    //
    if (window == nullptr) {
        if(DEBUG) std::cerr << "Window creation error: " << SDL_GetError() << std::endl;
        return;
    }
    //
    render = SDL_CreateRenderer(window, -1, 0);
    //
    if (render == nullptr) {
        if(DEBUG) std::cerr << "Render creation error: " << SDL_GetError() << std::endl;
        return;
    }
    //
    int imgFlags = IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        if(DEBUG) std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        return;
    }
    //
    SDL_Rect destRect = {this->x_pos, this->y_pos, this->WINDOW_WIDTH, this->WINDOW_HEIGTH};
}

WindowInterface::~WindowInterface() {
    SDL_DestroyRenderer(this->render);
    SDL_DestroyWindow(this->window);
    SDL_Quit();
}

void WindowInterface::framesToRender(std::vector<std::pair<std::pair<int,int>, UniversalImage>> &frames) {

    SDL_SetRenderDrawColor(render, 175, 175, 175, 32);
    SDL_RenderClear(render);

    for(int fid = 0; fid < frames.size(); fid++){


        if(frames[fid].second.imageType == "JPEG" || frames[fid].second.imageType == "UniversalImage"){
                // SDL_Rect destRect = { 10, (480 + 10) * fid + 10, 640, 480 }; // Wymiary mniejszego okna
                SDL_Rect destRect = { (640 + 10) * frames[fid].first.first + 10 - x_pos, (480 + 10) * frames[fid].first.second + 10 - y_pos, 640, 480 }; // Wymiary mniejszego okna

                SDL_RWops * rw = SDL_RWFromMem(frames[fid].second.data,static_cast<int> (frames[fid].second.size));

                if (rw == nullptr) {
                    if(DEBUG) std::cerr << "rw = NULL: " << IMG_GetError() << "| for fid: " << fid << std::endl;
                    continue;
                }

                SDL_Surface * imageSurface = IMG_Load_RW(rw, true); // ZWALNIA RÓWNIEŻ rw !!!

                if (imageSurface == nullptr) {
                    if(DEBUG) std::cerr << "Unable to load image from memory! SDL_image Error: " << IMG_GetError() << std::endl;
                    continue;
                } else {
                    SDL_Texture * texture = SDL_CreateTextureFromSurface(this->render, imageSurface);

                    //SDL_RenderCopy(render, texture, nullptr, &destRect);
                    SDL_RenderCopyEx(this->render, texture, nullptr, &destRect, 0, nullptr, SDL_FLIP_NONE);

                    SDL_DestroyTexture(texture);
                }

                SDL_FreeSurface(imageSurface);

            continue;
            }

        if(frames[fid].second.imageType == "GrayScale"){
            SDL_Rect destRect = { (640 + 10) * frames[fid].first.first + 10 - x_pos, (480 + 10) * frames[fid].first.second + 10 - y_pos, 640, 480 }; // Wymiary mniejszego okna

            SDL_Surface * screenSurface = SDL_GetWindowSurface(this->window);

            SDL_Texture * texture = SDL_CreateTexture(this->render, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, 640, 480);

            if (texture == NULL) {
                printf("Texture could not be created! SDL_Error: %s\n", SDL_GetError());
                std::cerr << "Texture could not be created! SDL_Error: " << SDL_GetError() << "\n";
                continue;
            }

            SDL_UpdateTexture(texture, NULL, frames[fid].second.data, 640);

            SDL_RenderCopy(this->render, texture, NULL, &destRect);

            continue;
        }

        if(frames[fid].second.imageType == "UniversalImage") {
            std::cout << "imageType: UniversalImage" << "\n";
            continue;
        }
    }
    //
    SDL_RenderPresent(this->render);
}

void WindowInterface::interface(volatile bool &work) {
        SDL_Event event;
        SDL_PollEvent(&event);

        int x_step = 20;
        int Y_step = 20;

        switch (event.type) {
            case SDL_QUIT:
                work = false;
                break;
            case SDL_KEYDOWN:
                {
                    switch (event.key.keysym.sym) {
                        case SDLK_UP:
                            if(this->y_pos - Y_step >= 0) this->y_pos -= Y_step;
                            break;
                        case SDLK_DOWN:
                            if(this->y_pos + Y_step <= this->max_y) this->y_pos += Y_step;
                            break;
                        case SDLK_RIGHT:
                            if(this->x_pos + x_step <= this->max_x) this->x_pos += x_step;
                            break;
                        case SDLK_LEFT:
                            if(this->x_pos - x_step >= 0) this->x_pos -= x_step;
                            break;
                    }
                }
                break;
        }
        //
        // std::cout << "x_pos: " << x_pos << "  |  " << "y_pos" << y_pos << "\n";
}


