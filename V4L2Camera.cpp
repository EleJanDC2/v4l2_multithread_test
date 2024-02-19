//
// Created by elejandc on 03.02.24.
//

#include <sys/poll.h>
#include "V4L2Camera.h"

#define CLEAR(x) memset(&(x), 0 , sizeof(x))

V4L2Camera::V4L2Camera(const char *device, int width, int height, int fps)
        : VIDEO_DEVICE(device), VIDEO_WIDTH(width), VIDEO_HEIGHT(height), FPS(fps) {
    // std::cout << "V4L2Camera constructor" << std::endl;

    //Open Camera
    this->fd = open(this->VIDEO_DEVICE, O_RDWR);
    if (this->fd < 0) {
        // fprintf(stderr, "V4L2Camera (%s) -> Can't open device \n", this->VIDEO_DEVICE);
        std::cerr << "V4L2Camera (" << this->VIDEO_DEVICE << ") -> Can't open device \n";
    }

    // Set the image frame format
    CLEAR(this->format);
    this->format.type = this->BUFFERS_TYPE;
    this->format.fmt.pix.height = this->VIDEO_HEIGHT;
    this->format.fmt.pix.width = this->VIDEO_WIDTH;
    this->format.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    // this->format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    // this->format.fmt.pix.pixelformat = V4L2_PIX_FMT_SBGGR8;
    if (ioctl(this->fd, VIDIOC_S_FMT, &(this->format)) < 0) {
        // fprintf(stderr, "V4L2Camera (%s) -> VIDIOC_S_FMT \n", this->VIDEO_DEVICE);
        std::cerr << "V4L2Camera (" << this->VIDEO_DEVICE << ") -> VIDIOC_S_FMT \n";
    }

    ///
    ///

    // Kopia formatu przed ustawieniem, aby zachować pierwotne ustawienia
    struct v4l2_format orig_format = this->format;

    // Wywołaj VIDIOC_G_FMT, aby pobrać aktualny format
    if (ioctl(this->fd, VIDIOC_G_FMT, &(this->format)) < 0) {
        std::cerr << "Błąd przy pobieraniu aktualnego formatu: " << strerror(errno) << std::endl;
    } else {
        // Sprawdź, czy ustawiony format pikseli jest zgodny z oczekiwanym
        if (this->format.fmt.pix.pixelformat == V4L2_PIX_FMT_SGRBG8) {
            std::cout << "Urządzenie używa formatu V4L2_PIX_FMT_SGRBG8." << std::endl;
        }  else if (this->format.fmt.pix.pixelformat == V4L2_PIX_FMT_MJPEG){
            std::cout << "Urządzenie używa formatu V4L2_PIX_FMT_MJPEG." << std::endl;
        } else if (this->format.fmt.pix.pixelformat == V4L2_PIX_FMT_YUYV){
            std::cout << "Urządzenie używa formatu V4L2_PIX_FMT_YUYV." << std::endl;
        } else {
            std::cerr << "Urządzenie nie używa oczekiwanego formatu. Używa: " << this->format.fmt.pix.pixelformat << std::endl;

        }
    }

    // Przywróć oryginalny format (opcjonalne, w zależności od dalszego kodu)
    this->format = orig_format;

    ///
    ///


    //
    CLEAR(v4L2Requestbuffers);
    this->v4L2Requestbuffers.count = this->BUFFERS_COUNT;
    this->v4L2Requestbuffers.type = this->BUFFERS_TYPE;
    this->v4L2Requestbuffers.memory = V4L2_MEMORY_MMAP;
    if (ioctl(this->fd, VIDIOC_REQBUFS, &(this->v4L2Requestbuffers)) < 0) {
        // fprintf(stderr, "V4L2Camera (%s) -> VIDIOC_REQBUFS \n", this->VIDEO_DEVICE);
        std::cerr << "V4L2Camera (" << this->VIDEO_DEVICE << ") -> VIDIOC_REQBUFS \n";
    }

    for (int i = 0; i < V4L2Camera::BUFFERS_COUNT; i++) {
        //
        CLEAR(v4L2Buffers[i]);
        v4L2Buffers[i] = v4l2_buffer(); /// NEW
        v4L2Buffers[i].index = i;
        v4L2Buffers[i].type = this->BUFFERS_TYPE;
        v4L2Buffers[i].memory = V4L2_MEMORY_MMAP;
        v4L2Buffers[i].length = format.fmt.pix.sizeimage;
        //
        if (ioctl(fd, VIDIOC_QUERYBUF, &(this->v4L2Buffers[i])) < 0) {
            // fprintf(stderr, "V4L2Camera (%s) -> VIDIOC_QUERYBUF \n", this->VIDEO_DEVICE);
            std::cerr << "V4L2Camera (" << this->VIDEO_DEVICE << ") -> VIDIOC_QUERYBUF \n";
        }
        //
        //
        this->buffers[i] = mmap(NULL, this->v4L2Buffers[i].length, PROT_READ | PROT_WRITE, MAP_SHARED, this->fd,
                                this->v4L2Buffers[i].m.offset);
        if (this->buffers[i] == MAP_FAILED) {
            // fprintf(stderr, "V4L2Camera (%s) -> nmap error for buffor: %d \n", this->VIDEO_DEVICE ,i);
            std::cerr << "V4L2Camera (" << this->VIDEO_DEVICE << ") -> nmap error for buffor: " << i << "\n";
        }
    }

    int approaches = 0;
    while (!Iniciazized && approaches < 3) {
        CLEAR(this->v4L2Streamparm);
        this->v4L2Streamparm.type = this->BUFFERS_TYPE;
//        this->v4L2Streamparm.parm.capture.timeperframe.numerator = 1;
//        this->v4L2Streamparm.parm.capture.timeperframe.denominator = this->FPS;
        //
        if (ioctl(this->fd, VIDIOC_STREAMON, &(this->v4L2Streamparm)) < 0) {
            // fprintf(stderr, "V4L2Camera (%s) -> VIDIOC_STREAMON \n", this->VIDEO_DEVICE);
            std::cerr << "V4L2Camera (" << this->VIDEO_DEVICE << ") -> VIDIOC_STREAMON \n";
            approaches++;
        } else {
            Iniciazized = true;
        }
    }

    std::cout << "End of create object on V4L2Camera for device: " << this->VIDEO_DEVICE << std::endl;
}

V4L2Camera::~V4L2Camera() {
    // std::cout << "V4L2Camera destructor" << std::endl;

    stopCapture();

    // Zwalnianie pamięci nmap
    for (int i = 0; i < V4L2Camera::BUFFERS_COUNT; i++) {
        if (munmap(this->buffers[i], this->v4L2Buffers[i].length)) {
            // fprintf(stderr, "V4L2Camera (%s) - destructor -> munmap error for buffor: %d \n", this->VIDEO_DEVICE,i);
            std::cerr << "V4L2Camera (" << this->VIDEO_DEVICE << ") - destructor -> munmap error for buffor: " << i
                      << "\n";
        }
    }

    close(this->fd);

    std::cout << "~V4L2Camera" << std::endl;
}

void V4L2Camera::startCapture() {
    // std::cout << "V4L2Camera startCapture" << std::endl;

    if (!(this->Iniciazized)) {
        std::cerr << "V4L2Camera (" << this->VIDEO_DEVICE << ") - startCapture -> not Iniciazized" << "\n";
        return;
    }

    if (this->IsCapturing) return;

    this->IsCapturing = true;
    capturingThread = std::thread(&V4L2Camera::capture, this);

    if (this->capturingThread.joinable())
        std::cout << "Wątek utworzony poprawnie dla obiektu : " << this->VIDEO_DEVICE << " | z id wątku : "
                  << this->capturingThread.get_id() << std::endl;
    else std::cerr << "Wątek nie został utworzony dla obiektu : " << this->VIDEO_DEVICE << std::endl;
}

void V4L2Camera::stopCapture() {
    // std::cout << "V4L2Camera stopCapture" << std::endl;

    if (!(this->IsCapturing)) return;
    IsCapturing = false;
    capturingThread.join();
}

void V4L2Camera::capture() {
    static std::vector<bool> bufferUsed(BUFFERS_COUNT, false);

    while (this->IsCapturing) {
        // std::cout << "capture:" << this->VIDEO_DEVICE << " | p1" << std::endl;
        // std::cout << "capture: " << this->VIDEO_DEVICE << "\n";

        // "oddawanie wolnych bufforów"
        for (int i = 0; i < BUFFERS_COUNT; i++) {
            if (!bufferUsed[i]) {
                if (ioctl(this->fd, VIDIOC_QBUF, &(this->v4L2Buffers[i])) < 0) {
                    // std::cerr << "V4L2Camera (" << this->VIDEO_DEVICE << ") - capture -> VIDIOC_QBUF for buffor: " << i << "\n";
                } else {
                    bufferUsed[i] = true; // Oznacz bufor jako używany
                }
            }
        }

        int buf_index = 0;

        // Ustaw timeout
        struct pollfd pfd{};
        pfd.fd = this->fd;
        pfd.events = POLLIN;
        pfd.revents = 0;

        // Wykonaj ioctl() z monitorem timera
        int ret = poll(&pfd, 1, 200); // Timeout 100ms

        if (ret == -1) {
            // Błąd
            std::cerr << "ret error" << "\n";
        } else if (ret == 0) {
            // Timeout
            std::cerr << "V4L2Camera (" << this->VIDEO_DEVICE << ") - capture -> VIDIOC_DQBUF - Timeout\n";

            if (strcmp(VIDEO_DEVICE, "/dev/video4") == 0) {
                std::cout << "PS3 - 3" << "\n";
            }
        } else if (pfd.revents & POLLIN) {
            // Dane są dostępne, wywołaj ioctl() i przetwórz dane

            if (ioctl(fd, VIDIOC_DQBUF, &v4L2Buffers[buf_index]) < 0) {
                std::cerr << "V4L2Camera (" << this->VIDEO_DEVICE << ") - takePictures -> VIDIOC_DQBUF: " << errno
                          << "\n";
            } else {
                if (strcmp(VIDEO_DEVICE, "/dev/video4") == 0) {
                    std::cout << "PS3 - 1" << "\n";
                }

                // Zapis do VideoFrame.
                {
                    if (this->v4L2Buffers[buf_index].length > 0) {

                        if (strcmp(VIDEO_DEVICE, "/dev/video4") == 0) {
                            std::cout << "PS3 - 2" << "\n";
                            std::cout << "v4L2Buffers[buf_index].length : " << v4L2Buffers[buf_index].length << "\n";
                        }

                        this->frameMutex.lock();
                        delete this->videoFrame;
                        this->videoFrame = new VideoFrame(buffers[buf_index], v4L2Buffers[buf_index].length,
                                                          this->VIDEO_WIDTH,
                                                          this->VIDEO_HEIGHT, this->VIDEO_DEVICE);
                        this->videoFrame->printInfo();

                        this->frameMutex.unlock();
                    }
                }
            }
        }

        // Oznacz bufor jako dostępny
        bufferUsed[buf_index] = false;
    }
}

VideoFrame *V4L2Camera::getFrame() {
    std::lock_guard<std::mutex> lock(frameMutex);
    if (videoFrame) return videoFrame;
    else {
        // std::cerr << "videoFrame = null" << std::endl;
        return new VideoFrame(this->VIDEO_DEVICE);
    }
}

bool V4L2Camera::isCampuring() {
    return this->capturingThread.joinable();
}

bool V4L2Camera::tryIoctl(unsigned long ioctlCode, void *parameter, bool failIfBusy, int attempts) const {
    while (true) {
        errno = 0;
        int result = ioctl(this->fd, ioctlCode, parameter);
        int err = errno;
//        CV_LOG_DEBUG(NULL, "VIDEOIO(V4L2:" << deviceName << "): call ioctl(" << this->fd << ", "
//                                           << decode_ioctl_code(ioctlCode) << "(" << ioctlCode << "), ...) => "
//                                           << result << "    errno=" << err << " (" << strerror(err) << ")"
//        );

        if (result != -1)
            return true;  // success

        const bool isBusy = (err == EBUSY);
        if (isBusy && failIfBusy) {
//            CV_LOG_INFO(NULL, "VIDEOIO(V4L2:" << deviceName << "): ioctl returns with errno=EBUSY");
            return false;
        }
        if (!(isBusy || errno == EAGAIN))
            return false;

        if (--attempts == 0) {
            return false;
        }

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(this->fd, &fds);

        /* Timeout. */
        static int param_v4l_select_timeout = 1;
        struct timeval tv;
        tv.tv_sec = param_v4l_select_timeout;
        tv.tv_usec = 0;

        errno = 0;
        result = select(this->fd + 1, &fds, NULL, NULL, &tv);
        err = errno;

        if (0 == result) {
//            CV_LOG_WARNING(NULL, "VIDEOIO(V4L2:" << deviceName << "): select() timeout.");
            return false;
        }

//        CV_LOG_DEBUG(NULL, "VIDEOIO(V4L2:" << deviceName << "): select(" << this->fd << ") => "
//                                           << result << "   errno = " << err << " (" << strerror(err) << ")"
//        );

        if (EINTR == err)  // don't loop if signal occurred, like Ctrl+C
        {
            return false;
        }
    }
    return true;
}

