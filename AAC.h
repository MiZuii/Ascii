//
// Created by Pedro on 13.03.2023.
//

/**
 * @file AAC.h
 *
 * @brief Main library header file
 *
 */

#include <iostream>
#include <string>
#include <system_error>
#include <thread>
#include "enums.h"
#include "structs.h"

#define MAX_SIZE 4000

#ifndef AAC_H
#define AAC_H

/* -------------------------------------------------------------------------- */
/*                                 ERROR CODES                                */
/* -------------------------------------------------------------------------- */

static thread_local std::error_code AAC_error_code;

void set_AAC_error_code(std::error_code ec);
std::error_code get_AAC_error_code();
void clear_AAC_error_code();

/**
 * @class AAC_error_category
 *
 * @brief Class provideing error messages for AAC library
 *
 */
class AAC_error_category : public std::error_category
{
public:
    virtual const char* name() const noexcept override {return "AAC_error_category";}

    virtual std::string message(int ec) const override {
    switch (static_cast<AAC_error_codes>(ec)){
        case AAC_error_codes::ALOCATION_ERROR:
            return "[AAC] Alocation error";
        case AAC_error_codes::INVALID_PIXEL:
            return "[AAC] Invalid pixel type";
        case AAC_error_codes::INVALID_PATH:
            return "[AAC] Invalid path";
        case AAC_error_codes::INVALID_ARGUMENTS:
            return "[AAC] Invalid arguments";
        case AAC_error_codes::IMAGE_OPEN_FAIL:
            return "[AAC] Failed to open image";
        case AAC_error_codes::IMAGE_ALLOCATION_ERROR:
            return "[AAC] Failed to allocate memory for piexels array";
        case AAC_error_codes::BRIGHTNESS_CALCULATION_FAIL:
            return "[AAC] Failed to calculate image brightness array";
        case AAC_error_codes::MATRIX_ALLOCATION_ERROR:
            return "[AAC] Failed to allocate space for AAC_Matrix";
        case AAC_error_codes::MATRIX_INDEX_OUT_OF_BOUNDS:
            return "[AAC] Index out of range";
        default:
            return "[AAC] Unknown error";
        }
    }
};

const AAC_error_category AAC_category{};

std::error_code make_error_code(AAC_error_codes ec);

/* -------------------------------------------------------------------------- */
/*                                MATRIX CLASS                                */
/* -------------------------------------------------------------------------- */

/**
 * @class AAC_Matrix
 *
 * @brief Multipurpose matrix class
 *
 */
template<typename T>
class AAC_Matrix
{
private:
    const unsigned int size_x;
    const unsigned int size_y;
    T **_matrix;

public:
    AAC_Matrix(unsigned int size_x, unsigned int size_y);
    ~AAC_Matrix();
    const T GetElement(unsigned int x, unsigned int y);
    T& GetElementReference(unsigned int x, unsigned int y);
    unsigned int GetXSize();
    unsigned int GetYSize();
};

template <typename T>
AAC_Matrix<T>::AAC_Matrix(unsigned int size_x, unsigned int size_y) : size_x(size_x), size_y(size_y)
{
    _matrix = new T *[size_y];
    if (NULL == _matrix)
    {
        set_AAC_error_code(make_error_code(AAC_error_codes::MATRIX_ALLOCATION_ERROR));
        throw get_AAC_error_code();
    }

    for (unsigned int i = 0; i < size_y; i++)
    {
        _matrix[i] = new T[size_x];

        if (NULL == _matrix[i])
        {
            set_AAC_error_code(make_error_code(AAC_error_codes::MATRIX_ALLOCATION_ERROR));
            throw get_AAC_error_code();
        }
    }
}

template <typename T>
AAC_Matrix<T>::~AAC_Matrix()
{
    for (unsigned int i = 0; i < size_y; i++)
    {
        delete[] _matrix[i];
    }
    delete[] _matrix;
}

template <typename T>
const T AAC_Matrix<T>::GetElement(unsigned int x, unsigned int y)
{
    if (x >= size_x || y >= size_y)
    {
        set_AAC_error_code(make_error_code(AAC_error_codes::MATRIX_INDEX_OUT_OF_BOUNDS));
        throw get_AAC_error_code();
    }
    return _matrix[y][x];
}

template <typename T>
T &AAC_Matrix<T>::GetElementReference(unsigned int x, unsigned int y)
{
    if (x >= size_x || y >= size_y)
    {
        set_AAC_error_code(make_error_code(AAC_error_codes::MATRIX_INDEX_OUT_OF_BOUNDS));
        throw get_AAC_error_code();
    }
    return _matrix[y][x];
}

template <typename T>
unsigned int AAC_Matrix<T>::GetXSize() {
    return size_x;
}

template <typename T>
unsigned int AAC_Matrix<T>::GetYSize() {
    return size_y;
}

/* -------------------------------------------------------------------------- */
/*                                 PIXEL CLASS                                */
/* -------------------------------------------------------------------------- */

/**
 * @class AAC_Pixel
 *
 * @brief Pixel class for storing AAC_Image pixels in more organised way
 *
 */
template <AAC_Pixel_Type E>
class AAC_Pixel
{
private:
    const AAC_Pixel_Type _pixel_type = E;
};

/* -------------------------------- GREY TYPE ------------------------------- */

template <>
class AAC_Pixel<AAC_Pixel_Type::G>
{
private:
    AAC_Pixel_G _pixel_values;

public:
    // constructors
    AAC_Pixel();
    AAC_Pixel(uint8_t grey);

    // getters and setters
    struct AAC_Pixel_G GetPixelValues();
    void SetPixelValues(uint8_t grey);
};

/* ----------------------------- GREY ALPHA TYPE ---------------------------- */

template <>
class AAC_Pixel<AAC_Pixel_Type::GA>
{
private:
    AAC_Pixel_GA _pixel_values;

public:
    // constructors
    AAC_Pixel();
    AAC_Pixel(uint8_t grey, uint8_t alpha);

    // getters and setters
    struct AAC_Pixel_GA GetPixelValues();
    void SetPixelValues(uint8_t grey, uint8_t alpha);
};

/* --------------------------- RED GREEN BLUE TYPE -------------------------- */

template <>
class AAC_Pixel<AAC_Pixel_Type::RGB>
{
private:
    AAC_Pixel_RGB _pixel_values;

public:
    // constructors
    AAC_Pixel();
    AAC_Pixel(uint8_t red, uint8_t green, uint8_t blue);

    // getters and setters
    struct AAC_Pixel_RGB GetPixelValues();
    void SetPixelValues(uint8_t red, uint8_t green, uint8_t blue);
};

/* ------------------------ RED GREEN BLUE ALPHA TYPE ----------------------- */

template <>
class AAC_Pixel<AAC_Pixel_Type::RGBA>
{
private:
    AAC_Pixel_RGBA _pixel_values;

public:
    // constructors
    AAC_Pixel();
    AAC_Pixel(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);

    // getters and setters
    struct AAC_Pixel_RGBA GetPixelValues();
    void SetPixelValues(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);
};

/* ------------------------------- EMPTY TYPE ------------------------------- */

template <>
class AAC_Pixel<AAC_Pixel_Type::EMPTY>
{
private:
    AAC_Pixel_EMPTY _pixel_values;

public:
    AAC_Pixel();
};

/* -------------------------------------------------------------------------- */
/*                                 IMAGE CLASS                                */
/* -------------------------------------------------------------------------- */

/**
 * @class AAC_Image
 *
 * @brief Contains full image as pixels matrix
 *
 */
class AAC_Image
{
private:
    const std::string _path;
    const unsigned int _n;

    void* _pixels_matrix;

    // helper for constructor
    // convers raw data to an array of appropriet size and AAC pixel types
    

public:
    const AAC_Pixel_Type pixel_type;
    const unsigned int size_x;
    const unsigned int size_y;

    AAC_Image(std::string path, unsigned int size_x, unsigned int size_y, unsigned int n, unsigned char *data);
    ~AAC_Image();
    void* GetMatrix();
};

/* --------------------------- GLOBAL IMAGE OPENER -------------------------- */
/**
 *  AAC_OpenImage
 *
 * @brief Global image opener
 */
AAC_Image* AAC_OpenImage(std::string path);

/* -------------------------------------------------------------------------- */
/*                                 CHUNK CLASS                                */
/* -------------------------------------------------------------------------- */

/**
 * @class AAC_Chunk
 *
 * @brief Representation of groups of pixels which are going to be replaced by single char
 *
 */
class AAC_Chunk
{
private:
    unsigned int _X_start_index; // inclusive
    unsigned int _X_end_index; // exclusive
    unsigned int _Y_start_index; // inclusive
    unsigned int _Y_end_index; // exclusive

    // pointer to external image brightness array(do not free on destroy)
    std::shared_ptr<AAC_Matrix<uint8_t>> _data;

public:
    AAC_Chunk();
    AAC_Chunk(unsigned int X_start_index, unsigned int X_end_index, unsigned int Y_start_index, unsigned int Y_end_index, std::shared_ptr<AAC_Matrix<uint8_t>> data);
    void SetChunk(unsigned int X_start_index, unsigned int X_end_index, unsigned int Y_start_index, unsigned int Y_end_index, std::shared_ptr<AAC_Matrix<uint8_t>> data);
    std::shared_ptr<AAC_Matrix<uint8_t>> GetData();
    unsigned int GetXStart();
    unsigned int GetXEnd();
    unsigned int GetYStart();
    unsigned int GetYEnd();
};

/* -------------------------------------------------------------------------- */
/*                         RIGHTNESS CONVERTER CLASSES                        */
/* -------------------------------------------------------------------------- */

/**
 * @class AAC_BrightnessConverter
 *
 * @brief Specifies group off classes converting AAC_Image to brightness matrix
 *
 */
class AAC_BrightnessConverter
{
public:
    virtual std::shared_ptr<AAC_Matrix<uint8_t>> convert(AAC_Image* img) = 0;
};

/**
 * @class AAC_BC_Simple
 *
 * @brief Simplest possible brightness converter
 *
 */
class AAC_BC_Simple : public AAC_BrightnessConverter
{
private:
    const float _red_weight, _green_weight, _blue_weight;

public:
    AAC_BC_Simple(float red_weight, float green_weight, float blue_weight);
    AAC_BC_Simple();
    std::shared_ptr<AAC_Matrix<uint8_t>> convert(AAC_Image* img) override;
};

/* -------------------------------------------------------------------------- */
/*                           CHUNK CONVERTER CLASSES                          */
/* -------------------------------------------------------------------------- */

/**
 * @class AAC_ChunkConverter
 *
 * @brief Converts chunks matrix into final string
 *
 */
class AAC_ChunkConverter
{
public:
    virtual std::string convert(AAC_Matrix<AAC_Chunk>* chunks) = 0;
};

/**
 * @class AAC_CC_Simple
 *
 * @brief Simplest possible chunk converter
 *
 */
class AAC_CC_Simple : public AAC_ChunkConverter
{
private:
    const std::string _alphabet;

public:
    AAC_CC_Simple(std::string alphabet);
    std::string convert(AAC_Matrix<AAC_Chunk>* chunks) override;

};

/* -------------------------------------------------------------------------- */
/*                               CONVERTER CLASS                              */
/* -------------------------------------------------------------------------- */

/**
 * @class AAC_Converter
 *
 * @brief Creates main converter combining all other steps to create art
 *
 */
class AAC_Converter
{
private:

    static const float _ratio;
    AAC_BrightnessConverter* _brightness_conv;
    AAC_ChunkConverter* _chunk_conv;
    
    AAC_Matrix<AAC_Chunk>* generateChunks(AAC_Image* img, size_t chunk_size, std::shared_ptr<AAC_Matrix<uint8_t>> brightness_matrix);

public:
    AAC_Converter(AAC_BrightnessConverter* brightness_conv, AAC_ChunkConverter* chunk_conv);
    std::string CreateArt(AAC_Image* img, size_t chunk_size);
};

#endif //AAC_H
