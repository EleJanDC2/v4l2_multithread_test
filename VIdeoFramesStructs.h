//
// Created by elejandc on 05.02.24.
//

#ifndef LINUXVIDEOCAPTURES_VIDEOFRAMESSTRUCTS_H
#define LINUXVIDEOCAPTURES_VIDEOFRAMESSTRUCTS_H

#include <memory>
#include "stdlib.h"
#include "stdio.h"
#include "iostream"
#include "string.h"

class UniversalImage{
public:
    void *data = nullptr;
    unsigned int size;
    unsigned int width;
    unsigned int height;

    std::string imageType = "UniversalImage";

    UniversalImage(){
        this->data = nullptr;
        this->size = 0;
        this->width = 0;
        this->height = 0;
    }

    ~UniversalImage(){
        if (data != nullptr) {
            free(data);
            data = nullptr;
        }
    }

    UniversalImage(const UniversalImage &other){
        this->data = std::malloc(other.size);

        if (this->data == nullptr) {
            this->size = 0;
            this->width = 0;
            this->height = 0;
        } else {
            memcpy(this->data, other.data, other.size);

            this->size = other.size;
            this->width = other.width;
            this->height = other.height;
        }
    }
};

class JPGImage : public UniversalImage{
public:
    JPGImage() : UniversalImage() {
        this->imageType = "JPEG";
    };

    ~JPGImage() {
        if (data != nullptr) {
            free(data);
            data = nullptr;
        }
    }

    JPGImage(const JPGImage &other)  : UniversalImage(other) {
        this->imageType = other.imageType;
    }
};

class GrayImage : public UniversalImage{
public:
    GrayImage() : UniversalImage() {
        this->imageType = "GrayScale";
    };

    ~GrayImage() {
        if (data != nullptr) {
            free(data);
            data = nullptr;
        }
    }

    GrayImage(const GrayImage &other)  : UniversalImage(other) {
        this->imageType = other.imageType;
    }
};

struct YCbCrImage {
    unsigned char *Y;  // Składowa luminancji
    unsigned char *Cb; // Składowa chrominancji B
    unsigned char *Cr; // Składowa chrominancji R
    int width;         // Szerokość obrazu
    int height;        // Wysokość obrazu
    //
    ~YCbCrImage() {
        delete (Y);
        delete (Cb);
        delete (Cr);
    }
};

#endif //LINUXVIDEOCAPTURES_VIDEOFRAMESSTRUCTS_H
