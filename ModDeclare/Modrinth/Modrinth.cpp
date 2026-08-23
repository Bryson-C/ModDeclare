//
// Created by Owner on 6/23/2026.
//

#include "Modrinth.hpp"

// TODO: Eventually I want to clean up the modrinth code to have all the information about a mod I could want in 1 place,
//  unfortunately, this does involve about 3 queries to get ABSOLUTELY every thing I could want

std::vector<ModrinthProject> Modrinth::getProjectSearchResults(const std::string& searchName, int limit) {

    // format the url to be valid for the api
    std::string name = std::regex_replace(searchName, std::regex("\""), "%22");
    name = std::regex_replace(searchName, std::regex(" "), "%20");

    std::string query = "https://api.modrinth.com/v2/search?query=" + name + "&limit=" + std::to_string(limit);
    auto json = getJsonFromRequestUrl(query);

    std::vector<ModrinthProject> projects;

    for (auto& project : json["hits"]) {
        projects.push_back(ModrinthProject::getProjectFromJson(project));
    }

    return projects;
}

std::vector<ModrinthProject> Modrinth::getProjectSearchResultsCustomQuery(const std::string& query) {
    auto json = getJsonFromRequestUrl(query);

    std::vector<ModrinthProject> projects;

    for (auto& project : json["hits"]) {
        projects.push_back(ModrinthProject::getProjectFromJson(project));
    }

    return projects;
}

