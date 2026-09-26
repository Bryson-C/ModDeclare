//
// Created by Owner on 6/23/2026.
//

#ifndef COGITOPLATFORM_MODRINTH_HPP
#define COGITOPLATFORM_MODRINTH_HPP

#include <print>
#include <regex>
#include <chrono>

#include "nlohmann/json.hpp"

#include "../Util/UrlRequest.hpp"
#include "ModrinthProject.hpp"

namespace {
    bool compareIgnoreCase(const std::string& a, const std::string& b) {
        std::string strA = a;
        std::string strB = b;
        std::transform(strA.begin(), strA.end(), strA.begin(), [](unsigned char c) { return std::tolower(c); });
        std::transform(strB.begin(), strB.end(), strB.begin(), [](unsigned char c) { return std::tolower(c); });
        return strA == strB;
    }
    bool containsIgnoreCase(const std::string& str, const std::string& substr) {
        std::string strA = str;
        std::string strB = substr;
        std::transform(strA.begin(), strA.end(), strA.begin(), [](unsigned char c) { return std::tolower(c); });
        std::transform(strB.begin(), strB.end(), strB.begin(), [](unsigned char c) { return std::tolower(c); });
        return strA.contains(strB);
    }

    std::string mostRecentIsoTimestamp(const std::string& a, const std::string& b) {
        return a.compare(b) > 0 ? a : b;
    }
    std::string removeAllNonAlpha(const std::string& str) {
        std::string s;
        for (const auto& chr : str) {
            // For whatever reason, I got a -16 from a char, so double check I guess
            if ((int)chr < -1 || (int)chr > 255) continue;
            if (isalpha(chr)) s += chr;
        }
        return s;
    }
}


namespace Modrinth {
    nlohmann::json getStagingRequest();

    std::vector<ModrinthProjectSearchResult> getProjectSearchResults(const std::string &searchName, int limit);

    std::vector<ModrinthProjectSearchResult> getProjectSearchResultsCustomQuery(const std::string &customQuery);

    struct DisplayNameAndSlug {
        std::string name, slug;
    };

    std::optional<DisplayNameAndSlug> getProjectDisplayNameAndSlugFromID(const std::string& projectId);

    std::optional<ModrinthProject> getModrinthProject(
            const std::string &name,
            const std::string &author,
            const std::string &organization,
            const std::string &mcVersion,
            const std::string &loader,
            const std::string& projectVersion = "",
            bool logVerbose = false,
            bool allowBetas = false,
            bool allowAlphas = false
    );

}




#endif //COGITOPLATFORM_MODRINTH_HPP
