/**
 * @file BmpParser.cpp
 */

#include "BmpParser.h"
#include "Exceptions.h"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <vector>

namespace ip {

namespace {

// BMP 헤더 구조체 — 디스크상의 바이너리 레이아웃과 동일해야 한다.
// 컴파일러의 패딩을 제거하기 위해 #pragma pack 사용.
#pragma pack(push, 1)

struct BitmapFileHeader {
    std::uint16_t bfType;      // "BM"
    std::uint32_t bfSize;      // 파일 전체 크기
    std::uint16_t bfReserved1;
    std::uint16_t bfReserved2;
    std::uint32_t bfOffBits;   // 픽셀 데이터 시작 오프셋
};

struct BitmapInfoHeader {
    std::uint32_t biSize;          // 정보 헤더 크기 (40)
    std::int32_t  biWidth;
    std::int32_t  biHeight;        // 양수: bottom-up, 음수: top-down
    std::uint16_t biPlanes;
    std::uint16_t biBitCount;      // 24 만 지원
    std::uint32_t biCompression;   // 0 (BI_RGB) 만 지원
    std::uint32_t biSizeImage;
    std::int32_t  biXPelsPerMeter;
    std::int32_t  biYPelsPerMeter;
    std::uint32_t biClrUsed;
    std::uint32_t biClrImportant;
};

#pragma pack(pop)

static_assert(sizeof(BitmapFileHeader) == 14, "BitmapFileHeader must be 14 bytes");
static_assert(sizeof(BitmapInfoHeader) == 40, "BitmapInfoHeader must be 40 bytes");

constexpr std::uint32_t BI_RGB = 0;

} // anonymous namespace

int BmpParser::paddedStride(int width) {
    const int rawStride = width * ImageBuffer::CHANNELS;
    const int remainder = rawStride % ROW_ALIGNMENT;
    return (remainder == 0) ? rawStride : (rawStride + (ROW_ALIGNMENT - remainder));
}

ImageBuffer BmpParser::loadFromFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw BmpParseError("Cannot open file for reading: " + path);
    }

    // ── 1. 파일 헤더 읽기 ───────────────────────────────
    BitmapFileHeader fileHeader{};
    file.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
    if (!file) {
        throw BmpParseError("Failed to read BITMAPFILEHEADER from: " + path);
    }
    if (fileHeader.bfType != BMP_MAGIC) {
        throw BmpParseError("Not a BMP file (magic mismatch): " + path);
    }

    // ── 2. 정보 헤더 읽기 ───────────────────────────────
    BitmapInfoHeader infoHeader{};
    file.read(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));
    if (!file) {
        throw BmpParseError("Failed to read BITMAPINFOHEADER from: " + path);
    }

    if (infoHeader.biBitCount != SUPPORTED_BPP) {
        throw BmpParseError(
            "Unsupported bit depth: " + std::to_string(infoHeader.biBitCount) +
            " (only 24-bit BMP is supported)");
    }
    if (infoHeader.biCompression != BI_RGB) {
        throw BmpParseError("Compressed BMP is not supported (compression=" +
                            std::to_string(infoHeader.biCompression) + ")");
    }
    if (infoHeader.biWidth <= 0) {
        throw BmpParseError("Invalid BMP width: " + std::to_string(infoHeader.biWidth));
    }

    const int width = infoHeader.biWidth;
    // 음수 height (top-down) 도 처리할 수 있도록 절댓값으로 변환.
    const bool isTopDown = (infoHeader.biHeight < 0);
    const int height = isTopDown ? -infoHeader.biHeight : infoHeader.biHeight;
    if (height <= 0) {
        throw BmpParseError("Invalid BMP height: " + std::to_string(infoHeader.biHeight));
    }

    // ── 3. 픽셀 데이터로 이동 ──────────────────────────
    file.seekg(fileHeader.bfOffBits, std::ios::beg);
    if (!file) {
        throw BmpParseError("Failed to seek to pixel data");
    }

    // ── 4. 픽셀 데이터 읽기 ────────────────────────────
    ImageBuffer image(width, height);
    const int stride = paddedStride(width);
    std::vector<std::uint8_t> rowBuffer(stride);

    // BMP는 기본적으로 bottom-up: 파일의 첫 행이 이미지의 마지막 행.
    for (int row = 0; row < height; ++row) {
        file.read(reinterpret_cast<char*>(rowBuffer.data()), stride);
        if (!file) {
            throw BmpParseError("Unexpected EOF while reading pixel rows (row=" +
                                std::to_string(row) + ")");
        }
        const int dstY = isTopDown ? row : (height - 1 - row);
        std::memcpy(image.rowPtr(dstY), rowBuffer.data(), image.rowStride());
    }

    return image;
}

void BmpParser::saveToFile(const std::string& path, const ImageBuffer& image) {
    if (image.empty()) {
        throw BmpParseError("Cannot save an empty image to: " + path);
    }

    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) {
        throw BmpParseError("Cannot open file for writing: " + path);
    }

    const int width  = image.width();
    const int height = image.height();
    const int stride = paddedStride(width);

    const std::uint32_t pixelDataSize = static_cast<std::uint32_t>(stride) *
                                        static_cast<std::uint32_t>(height);

    // ── 1. 헤더 구성 ────────────────────────────────────
    BitmapFileHeader fileHeader{};
    fileHeader.bfType      = BMP_MAGIC;
    fileHeader.bfOffBits   = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader);
    fileHeader.bfSize      = fileHeader.bfOffBits + pixelDataSize;

    BitmapInfoHeader infoHeader{};
    infoHeader.biSize        = sizeof(BitmapInfoHeader);
    infoHeader.biWidth       = width;
    infoHeader.biHeight      = height;        // bottom-up 으로 저장
    infoHeader.biPlanes      = 1;
    infoHeader.biBitCount    = SUPPORTED_BPP;
    infoHeader.biCompression = BI_RGB;
    infoHeader.biSizeImage   = pixelDataSize;

    // ── 2. 헤더 쓰기 ────────────────────────────────────
    file.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));
    file.write(reinterpret_cast<const char*>(&infoHeader), sizeof(infoHeader));

    // ── 3. 픽셀 데이터 쓰기 (bottom-up) ─────────────────
    std::vector<std::uint8_t> rowBuffer(stride, 0);
    for (int row = 0; row < height; ++row) {
        const int srcY = height - 1 - row;  // bottom-up
        std::memcpy(rowBuffer.data(), image.rowPtr(srcY), image.rowStride());
        // 패딩 영역은 이미 0으로 초기화되어 있음
        file.write(reinterpret_cast<const char*>(rowBuffer.data()), stride);
    }

    if (!file) {
        throw BmpParseError("Failed to write pixel data to: " + path);
    }
}

} // namespace ip
