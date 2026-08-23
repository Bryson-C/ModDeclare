//
// Created by Owner on 6/23/2026.
//

#ifndef COGITOPLATFORM_MODRINTH_HPP
#define COGITOPLATFORM_MODRINTH_HPP

#include <print>
#include <regex>

#include "nlohmann/json.hpp"

#include "../Util/UrlRequest.hpp"
#include "ModrinthProject.hpp"


namespace Modrinth {
    nlohmann::json getStagingRequest();
    std::vector<ModrinthProject> getProjectSearchResults(const std::string& searchName, int limit);
    std::vector<ModrinthProject> getProjectSearchResultsCustomQuery(const std::string& customQuery);
}



#endif //COGITOPLATFORM_MODRINTH_HPP
