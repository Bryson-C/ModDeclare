//
// Created by Owner on 8/27/2026.
//

#include "Tokenizer.hpp"

std::vector<std::string> ParseFileLinesToStrings(const std::vector<std::string>& fileLines) {
    // tokenizer/line reader
    std::vector<std::string> tokens;
    {
        auto isBufferEmpty = [](const std::string& buffer) -> bool {
            return (buffer.empty() || buffer == " ");
        };

        std::string buffer;
        bool inString = false;
        for (auto& line : fileLines) {
            if (line.empty()) continue;
            for (int i = 0; i < line.size(); i++) {
                auto &chr = line[i];
                // if we find a comment, skip the rest of the line, we only support single line comments
                if (chr == '#' && !inString) {
                    goto PARSER_NEXT_LINE;
                }
                // ending token conditions: ' ', SEPARATOR_CHAR, newline (covered by isspace()), '{', '}'
                if ((isspace(chr) || chr == SEPARATOR_CHAR[0] || chr == '{' || chr == '}') && !inString) {
                    // additionally make sure that the buffer should be pushed (i.e. not empty or only spaces)
                    if (!isBufferEmpty(buffer)) {
                        tokens.push_back(buffer);
                        buffer.clear();
                    }
                    // all conditions other than a space should also push a separator to the tokens array
                    switch (chr) {
                        case '\n': tokens.emplace_back(SEPARATOR_CHAR); break;
                        case '{': tokens.emplace_back("{"); break;
                        case '}': tokens.emplace_back("}"); break;
                        default: break;
                    }
                    continue;
                }

                // if we find a quote, we flip the string status
                if (chr == '\"') {
                    inString = !inString;
                }

                buffer += chr;
            }
            PARSER_NEXT_LINE:
            if (!isBufferEmpty(buffer)) {
                tokens.push_back(buffer);
                buffer.clear();
            }
            // only push an end of line token if its not a bracket
            if (!tokens.empty() && tokens.back() != "}" && tokens.back() != "{") {
                tokens.emplace_back(SEPARATOR_CHAR);
            }
        }
    }
    return tokens;
}