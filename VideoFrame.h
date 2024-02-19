//
// Created by elejandc on 03.02.24.
//

#ifndef LINUXVIDEOCAPTURES_VIDEOFRAME_H
#define LINUXVIDEOCAPTURES_VIDEOFRAME_H

#include "VIdeoFramesStructs.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <jpeglib.h>
#include <mutex>
#include "memory"

class VideoFrame{
public:
    VideoFrame(void *frameData, unsigned int frameSize, unsigned int width, unsigned int height, std::string camera_name);
    VideoFrame(std::string camera_name);
    VideoFrame(const VideoFrame& other);

    ~VideoFrame();


    void printInfo(){
        // printf("size: %d \n",this->size);
        std::cout << "Camera: " << this->camera_name << "- size: " << this->size << std::endl;
    }

    std::unique_ptr<JPGImage> getVideoFrameJPG();
    std::unique_ptr<YCbCrImage> getVideoFrameYCbCr();

private:
    void * data;
    unsigned int size;
    unsigned int width;
    unsigned int height;

    boolean  complete = false;

    std::string camera_name;
    std::mutex guard;
};

#endif //LINUXVIDEOCAPTURES_VIDEOFRAME_H
