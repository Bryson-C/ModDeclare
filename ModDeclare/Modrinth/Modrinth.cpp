//
// Created by Owner on 6/23/2026.
//

#include "Modrinth.hpp"

// TODO: Eventually I want to clean up the modrinth code to have all the information about a mod I could want in 1 place,
//  unfortunately, this does involve about 3 queries to get ABSOLUTELY every thing I could want

std::vector<ModrinthProjectSearchResult> Modrinth::getProjectSearchResults(const std::string& searchName, int limit) {

    // format the url to be valid for the api
    std::string name = std::regex_replace(searchName, std::regex("\""), "%22");
    name = std::regex_replace(searchName, std::regex(" "), "%20");

    std::string query = "https://api.modrinth.com/v2/search?query=" + name + "&limit=" + std::to_string(limit);
    auto json = getJsonFromRequestUrl(query);

    std::vector<ModrinthProjectSearchResult> projects;

    for (auto& project : json["hits"]) {
        projects.push_back(ModrinthProjectSearchResult::getProjectFromJson(project));
    }

    return projects;
}

std::vector<ModrinthProjectSearchResult> Modrinth::getProjectSearchResultsCustomQuery(const std::string& query) {
    auto json = getJsonFromRequestUrl(query);

    std::vector<ModrinthProjectSearchResult> projects;

    for (auto& project : json["hits"]) {
        projects.push_back(ModrinthProjectSearchResult::getProjectFromJson(project));
    }

    return projects;
}

std::optional<ModrinthProjectSearchResult> getProjectSlug(const std::string& name, const std::string& author, const std::string &organization) {
    for (const auto &potential : Modrinth::getProjectSearchResults(name, 10)) {
        // if name and at least author or organization match
        std::string search = removeAllNonAlpha(name), actual = removeAllNonAlpha(potential.title);
        if (
                (compareIgnoreCase(actual, search) || containsIgnoreCase(actual, search)) &&
                (compareIgnoreCase(potential.organization, organization) || compareIgnoreCase(potential.author, author))) {
            return {potential};
        }
    }
    return {};
}


std::optional<Modrinth::DisplayNameAndSlug> Modrinth::getProjectDisplayNameAndSlugFromID(const std::string& projectId) {
    try {
        nlohmann::json json = getJsonFromRequestUrl(std::format(R"(https://api.modrinth.com/v2/project/{})", projectId));
        DisplayNameAndSlug nameAndSlug;
        nameAndSlug.name = json["title"].get<std::string>();
        nameAndSlug.slug = json["slug"].get<std::string>();
        return nameAndSlug;
    } catch (std::exception& e) {
        return {};
    }
}

// to avoid circular dependencies and large dependency trees, add the project id to a set, and check if we already have it in the set, if so, skip
std::optional<ModrinthProject> getProjectVersionOrLatestStable(const std::string& slugOrId, const std::string& projectVersion, const std::string& mcVersion, const std::string& loader, std::unordered_set<std::string>& requestedDependencies, bool logVerbose = false, bool allowBetas = false, bool allowAlphas = false) {
    // make sure if we already have the slug/id in the set, we don't get it again and can simply return nothing
    if (requestedDependencies.contains(slugOrId)) {
        return {};
    }

    // possible version options
    std::vector<nlohmann::json> possibleVersions;

    // get the project versions for the specified minecraft version
    if (projectVersion.empty()) {
        try {
            std::string query = std::format(R"(https://api.modrinth.com/v2/project/{}/version)", slugOrId);
            int facetCount = 0;
            if (!loader.empty()) {
                query += "?";
                query += "loaders=[\"" + loader + "\"]";
                facetCount++;
            }
            if (!mcVersion.empty()) {
                query += (facetCount > 0) ? "&" : "?";
                query += "game_versions=[\"" + mcVersion + "\"]";
                facetCount++;
            }

            nlohmann::json projectVersions = getJsonFromRequestUrl(cleanStringForUrl(query));

            // to include all release types add: "release", "beta", and "alpha"
            std::unordered_set<std::string> releaseTypeWhiteList{"release"};
            if (allowBetas) releaseTypeWhiteList.emplace("beta");
            if (allowAlphas) releaseTypeWhiteList.emplace("alpha");
            // [0] = release, [1] = beta, [2] = alpha
            int releaseTypeHits[3] = {0,0,0};
            for (const auto &version: projectVersions) {
                if (version["version_type"] == "release") releaseTypeHits[0]++;
                else if (version["version_type"] == "beta") releaseTypeHits[1]++;
                else if (version["version_type"] == "alpha") releaseTypeHits[2]++;

                if (releaseTypeWhiteList.contains(version["version_type"])) {
                    std::string releaseType = version["version_type"];
                    possibleVersions.push_back(version);
                    continue;
                }
            }
            if (logVerbose) {
                std::string releasesString = std::format("{} {} Releases {}", releaseTypeHits[0] > 0 ? Colors::GREEN : Colors::RESET, releaseTypeHits[0], Colors::RESET);
                std::string betasString = std::format("{} {} Betas {}", releaseTypeHits[1] > 0 ? Colors::GREEN : Colors::RESET, releaseTypeHits[1], Colors::RESET);
                std::string alphasString = std::format("{} {} Alphas {}", releaseTypeHits[2] > 0 ? Colors::GREEN : Colors::RESET, releaseTypeHits[2], Colors::RESET);
                std::print("{} [{} | {} | {}]\n", slugOrId, releasesString, betasString, alphasString);
            }
        } catch (std::exception &e) {
            std::print("Failed Parsing Json For {}: {}\n", slugOrId, e.what());
            return {};
        }
    }
    else {
        try {
            auto specificVersion = getJsonFromRequestUrl(
                cleanStringForUrl(std::format(R"(https://api.modrinth.com/v2/project/{}/version/{})", slugOrId, projectVersion))
            );
            // here for whatever reason, .empty is not working on the json object,
            // so we need to make sure the json can be interpreted as a valid object,
            // if not, then this cannot be a valid return from the API
            if (!specificVersion.empty() && specificVersion.is_object()) {
                possibleVersions.emplace_back(specificVersion);
            }
        } catch (std::exception &e) {
            std::print("Failed Parsing Json For {}: {}\n", slugOrId, e.what());
            return {};
        }
    }

    // filter, currently only pick the most recently uploaded
    int i = 0, recent = 0;
    for (const auto &version: possibleVersions) {
        // select the most recent between the most recent saved and the current
        const std::string& current = version["date_published"].get<std::string>();
        const std::string& mostRecent = possibleVersions[recent]["date_published"].get<std::string>();
        if (mostRecentIsoTimestamp(current, mostRecent) == current) {
            recent = i;
        }
        i++;
    }

    // if no recent version has been iterated over (i.e. the list was empty) return none
    if (possibleVersions.empty()) {
        return {};
    }

    // just get the basics of what the mod needs first
    ModrinthProject project(possibleVersions[recent]);

    // now get the dependencies
    // NOTE: The # of dependencies may vary from the amount of iterations, this is due to sub-dependencies having the same dependency as their parent
    //       And if we only need 1 copy of the dependency, then we can simply ignore it
    for (const auto& dependency : possibleVersions[recent]["dependencies"]) {
        auto dep = getProjectVersionOrLatestStable(
            dependency["project_id"],
            (dependency["version_id"].is_null() ? "" : dependency["version_id"]),
            mcVersion,
            loader,
            requestedDependencies,
            logVerbose
        );

        if (dep.has_value()) {
            project.addDependency(dep.value(), dependency["dependency_type"]);
            requestedDependencies.insert(dependency["project_id"].get<std::string>());
        } else {
            // once we start to get no more dependencies that are valid, we can exit
            break;
        }
    }

    return project;
}

std::optional<ModrinthProject> Modrinth::getModrinthProject(const std::string& name, const std::string& author, const std::string &organization, const std::string& mcVersion, const std::string& loader, const std::string& projectVersion, bool logVerbose, bool allowBetas, bool allowAlphas) {
    // get (or attempt to get) the slug of the project (basically its unique identifier)
    std::optional<ModrinthProjectSearchResult> proj = getProjectSlug(name, author, organization);

    // check to make sure a valid project was found
    if (!proj.has_value()) {
        // nothing else we can do here, so just return
        return {};
    }

    std::unordered_set<std::string> requestedDependencies{};
    return getProjectVersionOrLatestStable(proj->slug, projectVersion, mcVersion, loader, requestedDependencies, logVerbose, allowBetas, allowAlphas);
}
