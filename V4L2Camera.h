//
// Created by elejandc on 03.02.24.
//

#ifndef LINUXVIDEOCAPTURES_V4L2CAMERA_H
#define LINUXVIDEOCAPTURES_V4L2CAMERA_H

#include "VideoFrame.h"
//
#include "linux/videodev2.h"
#include <iostream>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <stdexcept>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <vector>
#include <fstream>

class V4L2Camera {
public:

    V4L2Camera(const char *device, int width, int height, int fps);

    ~V4L2Camera();

    void startCapture();

    void stopCapture();

    VideoFrame * getFrame();

    bool isCampuring();

// Variable
private:
    VideoFrame * videoFrame; // Polimorfia

    const char *VIDEO_DEVICE;
    int VIDEO_WIDTH;
    int VIDEO_HEIGHT;
    int FPS;

    int fd;

    static const int BUFFERS_COUNT = 1;
    const v4l2_buf_type BUFFERS_TYPE = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    //Structs
    struct v4l2_format format{};
    struct v4l2_requestbuffers v4L2Requestbuffers{};
    struct v4l2_buffer v4L2Buffers[BUFFERS_COUNT]{};
    struct v4l2_streamparm v4L2Streamparm{};

    void * buffers[BUFFERS_COUNT]{};

    bool IsCapturing = false;
    std::thread capturingThread;

    std::mutex frameMutex;

    bool Iniciazized = false;
// Functions
private:
    void capture();

private:
    // NIE MOJE
    bool tryIoctl(unsigned long ioctlCode, void *parameter, bool failIfBusy = true, int attempts = 10) const;
};

#endif //LINUXVIDEOCAPTURES_V4L2CAMERA_H
