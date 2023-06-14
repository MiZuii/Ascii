#include "../AAC.h"

#include <locale>
#include <codecvt>
#include <string>

#define BRAILE_CHUNKX_DIVISOR 2
#define BRAILE_CHUNKY_DIVISOR 4

/* -------------------------------------------------------------------------- */
/*                                AAC_CC_Simple                               */
/* -------------------------------------------------------------------------- */

uint8_t get_char_index(size_t interval_length, uint8_t brightness) {
    return brightness / interval_length;
}

AAC::CC_Simple::CC_Simple(std::string alphabet) : _alphabet(alphabet) {}

std::string AAC::CC_Simple::convert(AAC::Matrix<AAC::Chunk>* chunks) {

    // find the interval of the alphabet
    size_t alphabet_len = _alphabet.length();

    if( alphabet_len < 2 || alphabet_len > 255 ) {
        AAC::set_error_code(AAC::make_error_code(AAC::error_codes::INVALID_ARGUMENTS));
        throw AAC::get_error_code();
    }

    uint8_t interval_len = 255 / alphabet_len;


    // make resulting char matrix and gather chunk data
    AAC::Matrix<char> art_result = AAC::Matrix<char>(chunks->GetXSize(), chunks->GetYSize());
    std::shared_ptr<AAC::Matrix<uint8_t>> brightness_matrix = chunks->GetElementReference(0, 0).GetData();

    // iterate through chunks and generate result
    for(size_t y = 0; y < chunks->GetYSize(); y++) {
        for(size_t x = 0; x < chunks->GetXSize(); x++) {

            AAC::Chunk cchunk = chunks->GetElement(x, y);

            unsigned long sum = 0;
            unsigned long quantity = (cchunk.GetYEnd() - cchunk.GetYStart())*(cchunk.GetYEnd() - cchunk.GetYStart());

            for(size_t cy = cchunk.GetYStart(); cy < cchunk.GetYEnd(); cy++) {
                for(size_t cx = cchunk.GetXStart(); cx < cchunk.GetXEnd(); cx++) {
                    sum += brightness_matrix->GetElement(cx, cy);
                }
            }

            art_result.GetElementReference(x, y) = _alphabet[get_char_index(interval_len, sum / quantity)];
        }
    }

    std::string result = "";

    // conver to final string
    for(size_t y = 0; y < art_result.GetYSize(); y++) {
        for(size_t x = 0; x < art_result.GetXSize(); x++) {

            result.push_back(art_result.GetElement(x, y));
        }
        result += '\n';
    }

    return result;
}

/* -------------------------------------------------------------------------- */
/*                                AAC_CC_Braile                               */
/* -------------------------------------------------------------------------- */

wchar_t AAC::CC_Braile::get_braile_char(uint8_t char_val) {
    return static_cast<wchar_t>(0x2800 + char_val);
}

AAC::CC_Braile::CC_Braile(uint8_t break_point_brightness) : _bk_brightness(break_point_brightness) {}

std::string AAC::CC_Braile::convert(AAC::Matrix<AAC::Chunk>* chunks) {

    // check if necessery chunk size is provided
    if( (chunks->GetElementReference(0, 0).GetXEnd() - chunks->GetElementReference(0, 0).GetXStart()) < BRAILE_CHUNKX_DIVISOR ||
      (chunks->GetElementReference(0, 0).GetYEnd() - chunks->GetElementReference(0, 0).GetYStart()) < BRAILE_CHUNKY_DIVISOR ) {
        AAC::set_error_code(AAC::make_error_code(AAC::error_codes::CHUNK_SIZE_ERROR));
        throw AAC::get_error_code();
    }

    // make resulting char matrix and gather chunk data
    AAC::Matrix<wchar_t> art_result = AAC::Matrix<wchar_t>(chunks->GetXSize(), chunks->GetYSize());
    std::shared_ptr<AAC::Matrix<uint8_t>> brightness_matrix = chunks->GetElementReference(0, 0).GetData();
    AAC::Matrix<uint8_t> mini_matrix(BRAILE_CHUNKX_DIVISOR, BRAILE_CHUNKY_DIVISOR);

    // calculate columns and rows divide
    AAC::Chunk tchunk = chunks->GetElement(0, 0);
    unsigned int column_sizes[BRAILE_CHUNKX_DIVISOR + 1];
    unsigned int row_sizes[BRAILE_CHUNKY_DIVISOR + 1];

    unsigned int column_size =  (tchunk.GetXEnd() - tchunk.GetXStart()) / BRAILE_CHUNKX_DIVISOR;
    uint8_t column_oversize = (tchunk.GetXEnd() - tchunk.GetXStart()) % BRAILE_CHUNKX_DIVISOR;

    unsigned int row_size =  (tchunk.GetYEnd() - tchunk.GetYStart()) / BRAILE_CHUNKY_DIVISOR;
    uint8_t row_oversize = (tchunk.GetYEnd() - tchunk.GetYStart()) % BRAILE_CHUNKY_DIVISOR;

    // apply sizes and refractor into prefix sum
    column_sizes[0] = 0;
    column_sizes[1] = column_size + (column_oversize == 1);
    column_sizes[2] = 2 * column_size + (column_oversize == 1);

    row_sizes[0] = 0;
    row_sizes[1] = row_size + (row_oversize > 2);
    row_sizes[2] = 2 * row_size + (row_oversize > 0) + (row_oversize > 2);
    row_sizes[3] = 3 * row_size + (row_oversize > 1) + (row_oversize > 0) + (row_oversize > 2);
    row_sizes[4] = 4 * row_size + (row_oversize > 1) + (row_oversize > 0) + (row_oversize > 2);

    // iterate through chunks and generate result
    for(size_t y = 1; y < chunks->GetYSize() - 1; y++) {
        for(size_t x = 1; x < chunks->GetXSize() - 1; x++) {

            AAC::Chunk cchunk = chunks->GetElement(x, y);

            // calculate chunk average brightness values for mini matrix
            for( uint8_t cell_row = 0; cell_row < BRAILE_CHUNKY_DIVISOR; cell_row++) {
                for( uint8_t cell_column = 0; cell_column < BRAILE_CHUNKX_DIVISOR; cell_column++ ) {

                    // iterate through given cell subcell and calculate average brightness
                    unsigned long sum = 0;
                    unsigned long quantity = (row_sizes[cell_row + 1] - row_sizes[cell_row]) * (column_sizes[cell_column + 1] - column_sizes[cell_column]);
                    size_t real_x_index, real_y_index;

                    for(real_x_index = cchunk.GetXStart() + column_sizes[cell_column]; real_x_index < cchunk.GetXStart() + column_sizes[cell_column + 1]; real_x_index++) {
                        for(real_y_index = cchunk.GetYStart() + row_sizes[cell_row]; real_y_index < cchunk.GetYStart() + row_sizes[cell_row + 1]; real_y_index++) {
                            sum += brightness_matrix->GetElement(real_x_index, real_y_index);
                        }
                    }

                    mini_matrix.GetElementReference(cell_column, cell_row) = sum / quantity;
                }
            }

            art_result.GetElementReference(x, y) = get_braile_char((mini_matrix.GetElement(0, 0) > _bk_brightness) + 
                                                                   2*(mini_matrix.GetElement(0, 1) > _bk_brightness) + 
                                                                   4*(mini_matrix.GetElement(0, 2) > _bk_brightness) + 
                                                                   8*(mini_matrix.GetElement(1, 0) > _bk_brightness) + 
                                                                   16*(mini_matrix.GetElement(1, 1) > _bk_brightness) + 
                                                                   32*(mini_matrix.GetElement(1, 2) > _bk_brightness) + 
                                                                   64*(mini_matrix.GetElement(0, 3) > _bk_brightness) +
                                                                   128*(mini_matrix.GetElement(1, 3) > _bk_brightness));
        }
    }

    std::wstring result = L"";

    // conver to final string
    for(size_t y = 0; y < art_result.GetYSize(); y++) {
        for(size_t x = 0; x < art_result.GetXSize(); x++) {

            result.push_back(art_result.GetElement(x, y));
        }
        result += '\n';
    }

    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::string converted_result = converter.to_bytes(result);
    return converted_result;
}

