/**
 * @file main.cpp
 * @brief ImageProcessor 진입점 — 지원자가 작성해야 할 파일입니다.
 *
 * BMP 입출력과 커맨드라인 파싱은 제공된 코드가 처리합니다.
 * 본 과제에서 작성해야 할 것은 단 하나입니다:
 *
 *     ▶ 이미지 처리 필터 2개 이상 구현 + main 의 TODO 위치에 연결
 *
 * 또한 일관된 컨벤션과 예외 처리, 메모리 안정성도 함께 평가됩니다.
 */

#include "BmpParser.h"
#include "CommandLineParser.h"
#include "ImageBuffer.h"
#include "Exceptions.h"

#include <iostream>
#include <algorithm>

 // TODO: 본인이 구현한 필터 헤더를 include 하세요.
 // #include "GrayscaleFilter.h"
 // #include "ThresholdFilter.h"
 // ...

#include "filter/Flip.h"
#include "filter/GrayScale.h"
#include "filter/GaussianBlur.h"

int main(int argc, char* argv[]) {
    try {
        // ── CLI 인자 파싱 (제공된 코드) ─────────────────────────
        const ip::ProgramOptions options = ip::CommandLineParser::parse(argc, argv);

        // ── BMP 로드 (제공된 코드) ──────────────────────────────
        ip::ImageBuffer image = ip::BmpParser::loadFromFile(options.inputPath);
        std::cout << "Loaded: " << image.width() << " x " << image.height() << "\n";

        // ───────────────────────────────────────────────────────
        // TODO: options.filterName 에 따라 적절한 필터를 생성하고
        //       image 에 적용하세요.
        //
        //   예시 코드 (참고용):
        //
        //     if (options.filterName == "grayscale") {
        //         GrayscaleFilter filter;
        //         filter.apply(image);
        //     }
        //     else if (options.filterName == "threshold:128") {
        //         ThresholdFilter filter(128);
        //         filter.apply(image);
        //     }
        //     else {
        //         throw ip::FilterError("Unknown filter: " + options.filterName);
        //     }
        //
        //   ※ 가산점 항목:
        //     - 추상 클래스(FilterBase) 기반 다형성 설계
        //     - 필터 파이프라인 체인 (CLI 옵션 확장 필요)
        //     - 멀티쓰레드 처리
        //     - 로그 파일 출력 (CLI 옵션 확장 필요)
        // ───────────────────────────────────────────────────────

        // ↓ 여기에 필터 적용 코드를 작성하세요.
        bool multipleFilter = options.filterNames.size() > 0 ? true : false;
        ImageBuffer cpy = ImageBuffer(image.width(), image.height());

        if (multipleFilter) {
            int typeCnt = options.filterNames.size();
            std::vector<std::string> availableFilters = { "grayscale", "blur", "flip_horizontal", "flip_vertical" };

            // Inspect Filter Names
            for (const std::string& filterName : options.filterNames) {
                if (std::find(availableFilters.begin(), availableFilters.end(), filterName) == availableFilters.end()) {
                    throw ip::FilterError("Unknown filter: " + filterName);
                }
            }

            for (const std::string& filterName : options.filterNames) {
                if (filterName == "grayscale") {
                    GrayScale filter = GrayScale();
                    filter.apply(cref(image), ref(cpy));

                    image = cpy;
                }
                else if (filterName == "blur") {
                    GaussianBlur::apply(cref(image), ref(cpy), 3, 0.8);
                    image = cpy;
                }
                else if (filterName == "flip_horizontal") {
                    Flip::flip(Flip::FlipType::horizontal, cref(image), ref(cpy));
                    image = cpy;
                }
                else if (filterName == "flip_vertical") {
                    Flip::flip(Flip::FlipType::vertical, cref(image), ref(cpy));
                    image = cpy;
                }
                else {
                    throw ip::FilterError("Unknown filter: " + filterName);
                }
            }
        }

        if (!multipleFilter) {
            if (options.filterName == "grayscale") {
                GrayScale filter = GrayScale();
                filter.apply(cref(image), ref(cpy));

                image = cpy;
            }
            else if (options.filterName == "blur") {
                GaussianBlur::apply(cref(image), ref(cpy), 3, 0.8);
                image = cpy;
            }
            else if (options.filterName == "flip_horizontal") {
                Flip::flip(Flip::FlipType::horizontal, cref(image), ref(cpy));
                image = cpy;
            }
            else if (options.filterName == "flip_vertical") {
                Flip::flip(Flip::FlipType::vertical, cref(image), ref(cpy));
                image = cpy;
            }
            else {
                throw ip::FilterError("Unknown filter: " + options.filterName);
            }
        }

        // ── BMP 저장 (제공된 코드) ──────────────────────────────
        ip::BmpParser::saveToFile(options.outputPath, image);
        std::cout << "Saved:  " << options.outputPath << "\n";
        return 0;
    }
    catch (const ip::ArgumentError& e) {
        std::cerr << e.what() << "\n\n";
        ip::CommandLineParser::printUsage(argc > 0 ? argv[0] : "ImageProcessor");
        return 4;
    }
    catch (const ip::BmpParseError& e) {
        std::cerr << e.what() << std::endl;
        return 2;
    }
    catch (const ip::FilterError& e) {
        std::cerr << e.what() << std::endl;
        return 3;
    }
    catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        return 1;
    }
}
