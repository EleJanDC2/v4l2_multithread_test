//
// Created by elejandc on 03.02.24.
//

#include "VideoFrame.h"

#include <utility>

VideoFrame::VideoFrame(void *frameData, unsigned int frameSize, unsigned int width, unsigned int height,
                       std::string camera_name) {
    std::lock_guard<std::mutex> lock(guard);

    this->camera_name = std::move(camera_name);

    this->data = std::malloc(frameSize);

    if (this->data == nullptr) {
        // this->complete = false;
        this->size = 0;
        this->width = 0;
        this->height = 0;

        fprintf(stderr, "VideoFrame - malloc error");
    } else {
        memcpy(this->data, frameData, frameSize);

        this->size = frameSize;
        this->width = width;
        this->height = height;

        // this->complete = true;
    }
}

VideoFrame::VideoFrame(std::string camera_name) {
    std::lock_guard<std::mutex> lock(guard);

    this->data = nullptr;
    this->size = 0;
    this->height = 0;
    this->width = 0;

    this->camera_name = std::move(camera_name);
}

VideoFrame::VideoFrame(const VideoFrame& other) {
    std::lock_guard<std::mutex> lock(guard);

    this->camera_name = other.camera_name;

    this->data = std::malloc(other.size);

    if (this->data == nullptr) {
        this->complete = false;
        this->size = 0;
        this->width = 0;
        this->height = 0;

        fprintf(stderr, "VideoFrame - malloc error");
    } else {
        memcpy(this->data, other.data, other.size);
        //
        this->size = other.size;
        this->width = other.width;
        this->height = other.height;

        this->complete = true;
    }
}

VideoFrame::~VideoFrame() {
    std::lock_guard<std::mutex> lock(guard);

    if (this->data != nullptr) {
        free(this->data);
        data = nullptr;
    }
}

// TODO: obsługa złej alokacji pamięci.
std::unique_ptr<JPGImage> VideoFrame::getVideoFrameJPG() {
    std::lock_guard<std::mutex> lock(guard);

    // JPGImage *image = new JPGImage;
    auto image = std::make_unique<JPGImage>();

    image->data = std::malloc(this->size);

    if (image->data == nullptr) {
        return nullptr;
    } else {
        memcpy(image->data, this->data, this->size);

        image->size = this->size;
        image->width = this->width;
        image->height = this->height;

        return image;
    }
}

std::unique_ptr<YCbCrImage> VideoFrame::getVideoFrameYCbCr() {
    std::lock_guard<std::mutex> lock(guard);

    // YCbCrImage result;
    auto result = std::make_unique<YCbCrImage>();

    // Inicjalizacja struktur libjpeg
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    // Ustawienie danych JPEG
    jpeg_mem_src(&cinfo, (unsigned char *) this->data, this->size);

    // Wczytanie nagłówka JPEG
    jpeg_read_header(&cinfo, TRUE);

    // Start dekompresji
    jpeg_start_decompress(&cinfo);

    // Przygotowanie struktury wynikowej
    result->width = cinfo.output_width;
    result->height = cinfo.output_height;
    result->Y = new unsigned char[result->width * result->height];
    result->Cb = new unsigned char[result->width * result->height];
    result->Cr = new unsigned char[result->width * result->height];

    // Pobranie danych skompresowanych
    JSAMPARRAY buffer = (JSAMPARRAY) malloc(sizeof(JSAMPROW) * cinfo.output_height);
    for (int i = 0; i < cinfo.output_height; i++) {
        buffer[i] = (JSAMPROW) malloc(sizeof(JSAMPLE) * cinfo.output_width * cinfo.output_components);
    }

    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, buffer + cinfo.output_scanline, cinfo.output_height - cinfo.output_scanline);
    }

    // Konwersja danych do Y'CbCr
    for (int i = 0; i < cinfo.output_height; i++) {
        for (int j = 0; j < cinfo.output_width; j++) {
            result->Y[i * result->width + j] = buffer[i][j * cinfo.output_components];
            result->Cb[i * result->width + j] = buffer[i][j * cinfo.output_components + 1];
            result->Cr[i * result->width + j] = buffer[i][j * cinfo.output_components + 2];
        }
    }

    // Zakończenie dekompresji
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    // Zwolnienie pamięci bufora
    for (int i = 0; i < cinfo.output_height; i++) {
        free(buffer[i]);
    }
    free(buffer);

    return result;
}
