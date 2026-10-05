#pragma once

/**
 * @file CommandLineParser.h
 * @brief 커맨드라인 인자 파싱.
 *
 * 본 클래스는 기본 옵션(--input, --output, --filter)만 파싱한다.
 * 고급 옵션(--pipeline, --threads, --log 등)을 추가하려면
 * 본 클래스를 확장하거나 ProgramOptions 에 필드를 추가하라.
 */

#include <string>
#include <vector>

namespace ip {

/// 파싱 결과를 담는 단순 구조체.
struct ProgramOptions {
    std::string inputPath;                ///< --input    / -i
    std::string outputPath;               ///< --output   / -o
    std::string filterName;               ///< --filter   / -f
    std::vector<std::string> filterNames; ///< --pipeline / -p
};

class CommandLineParser {
public:
    CommandLineParser() = delete;  // 인스턴스화 금지 (정적 메서드만 제공)

    /**
     * @brief argv 를 파싱하여 ProgramOptions 를 반환한다.
     * @throws ArgumentError 필수 인자 누락, 알 수 없는 옵션, 형식 오류 등.
     */
    static ProgramOptions parse(int argc, char* argv[]);

    /// 사용법을 표준 출력에 출력한다.
    static void printUsage(const std::string& exeName);
};

} // namespace ip
