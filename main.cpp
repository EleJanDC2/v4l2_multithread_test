#include <iostream>
#include <libavformat/avformat.h>
#include <netinet/in.h>
#include "V4L2Camera.h"
#include "WindowInterface.h"
// sudo apt-get install libsdl2-dev
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "jpeglib.h"


///
///

std::unique_ptr<GrayImage> convertToGrayscale(unsigned char* jpegData, size_t dataSize) {
    // Inicjalizacja struktur libjpeg
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);

    // Inicjalizacja dekompresora
    jpeg_create_decompress(&cinfo);

    // Konfiguracja źródła danych JPEG
    jpeg_mem_src(&cinfo, jpegData, dataSize);

    // Odczyt nagłówka
    jpeg_read_header(&cinfo, TRUE);

    // Ustawienie parametrów konwersji
    cinfo.out_color_space = JCS_GRAYSCALE;

    // Rozpoczęcie dekompresji
    jpeg_start_decompress(&cinfo);

    // Uzyskanie wymiarów obrazu
    int width = static_cast<int> (cinfo.output_width);
    int height = static_cast<int> (cinfo.output_height);

    // Tworzenie bufora na dane obrazu
    // unsigned char* imageData  = new unsigned char[width * height];

    auto image = std::make_unique<GrayImage>();
    image->data = std::malloc(width*height);

    if(image->data != nullptr){
        image->size = width * height;
        image->width = width;
        image->height = height;
    }

    // Dekompresja i zapisanie do bufora
    JSAMPARRAY buffer;
    buffer = (cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, width * cinfo.output_components, 1);

    int row = 0;
    while (cinfo.output_scanline < height) {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        for (int i = 0; i < width; ++i) {
            // image->data[row * width + i] = buffer[0][i];
            ((unsigned char *)image->data)[row * width + i] = buffer[0][i];
        }
        ++row;
    }

    // Zakończenie dekompresji
    jpeg_finish_decompress(&cinfo);

    // Zwolnienie zasobów
    jpeg_destroy_decompress(&cinfo);

    return image;
}

///
///

#define WIDTH 320
#define HEIGTH 240

std::unique_ptr<JPGImage> obraz1;
std::unique_ptr<JPGImage> obraz2;
std::unique_ptr<JPGImage> obraz3;
std::mutex obrazDostep;

volatile bool work = true;

void watekCamera() {
    V4L2Camera cam1("/dev/video0", WIDTH, HEIGTH, 60);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    V4L2Camera cam2("/dev/video2", WIDTH, HEIGTH, 60);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    V4L2Camera cam3("/dev/video4", WIDTH, HEIGTH, 60);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

//    V4L2Camera cam1("/dev/v4l/by-path/pci-0000:00:14.0-usb-0:7:1.0-video-index0", WIDTH, HEIGTH, 30);
//    std::this_thread::sleep_for(std::chrono::milliseconds(200));
//    V4L2Camera cam2("/dev/v4l/by-path/pci-0000:00:14.0-usb-0:1.1:1.0-video-index0", WIDTH, HEIGTH, 30);
//    std::this_thread::sleep_for(std::chrono::milliseconds(200));
//    V4L2Camera cam3("/dev/v4l/by-path/pci-0000:00:14.0-usb-0:1.3:1.0-video-index0", WIDTH, HEIGTH, 30);
//    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    cam1.startCapture();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    cam2.startCapture();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    cam3.startCapture();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    while (work) {
        obrazDostep.lock();
        obraz1 = cam1.getFrame()->getVideoFrameJPG();
        obraz2 = cam2.getFrame()->getVideoFrameJPG();
        obraz3 = cam3.getFrame()->getVideoFrameJPG();
        obrazDostep.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    cam1.stopCapture();
    cam2.stopCapture();
    cam3.stopCapture();
}

void watekWyswietlacza() {
    std::this_thread::sleep_for(std::chrono::milliseconds(4000));

    std::vector<std::pair<std::pair<int,int>, UniversalImage>> pictures;

    WindowInterface win(30+2*640, 30+2*480);

    while(work){
        pictures.clear();

        obrazDostep.lock();
        pictures.push_back({{0, 0}, (JPGImage)*obraz1});
        pictures.push_back({{0, 1}, (JPGImage)*obraz2});
        pictures.push_back({{0, 2}, (JPGImage)*obraz3});
        obrazDostep.unlock();
        ///
        ///
        obrazDostep.lock();
//        std::unique_ptr<GrayImage> g1 = convertToGrayscale(static_cast<unsigned char *> (obraz1->data), obraz1->size);
//        pictures.push_back({{1, 0}, *g1});
        obrazDostep.unlock();

        win.framesToRender(pictures);
        win.interface(work);
    }
}

int main() {
    std::cout << "START!" << std::endl;

    // std::thread myThread1(userInput);
    std::thread myThread2(watekWyswietlacza);
    std::thread myThread3(watekCamera);

    // myThread1.join();
    myThread2.join();
    myThread3.join();

    std::cout << "END!" << std::endl;
    return 0;
}
