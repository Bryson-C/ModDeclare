//
// Created by Owner on 6/9/2026.
//

#ifndef COGITOPLATFORM_ANSICOLORS_HPP
#define COGITOPLATFORM_ANSICOLORS_HPP

#include <string>

namespace Colors {
    const std::string RESET = "\u001B[0m";
    const std::string BLACK = "\u001B[30m";
    const std::string RED = "\u001B[31m";
    const std::string GREEN = "\u001B[32m";
    const std::string YELLOW = "\u001B[33m";
    const std::string BLUE = "\u001B[34m";
    const std::string PURPLE = "\u001B[35m";
    const std::string CYAN = "\u001B[36m";
    const std::string WHITE = "\u001B[37m";

    const std::string BLACK_BACKGROUND = "\u001B[40m";
    const std::string RED_BACKGROUND = "\u001B[41m";
    const std::string GREEN_BACKGROUND = "\u001B[42m";
    const std::string YELLOW_BACKGROUND = "\u001B[43m";
    const std::string BLUE_BACKGROUND = "\u001B[44m";
    const std::string PURPLE_BACKGROUND = "\u001B[45m";
    const std::string CYAN_BACKGROUND = "\u001B[46m";
    const std::string WHITE_BACKGROUND = "\u001B[47m";

    inline std::string ColorString(const std::string& color, const std::string& text) {
        return color+text+Colors::RESET;
    }
}


#endif //COGITOPLATFORM_ANSICOLORS_HPP
