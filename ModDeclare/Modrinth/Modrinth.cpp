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
        //std::print("search: {} | actual: {}\n", search, actual);

        if ((compareIgnoreCase(actual, search) || containsIgnoreCase(actual, search))) {
            //std::print("\tCase 1; Got: {}\n", potential.title);
            return {potential};
        }
        if ((compareIgnoreCase(potential.organization, organization) || compareIgnoreCase(potential.author, author))) {
            //std::print("\tCase 2; Got: {}\n", potential.title);
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
std::pair<std::optional<ModrinthProject>, std::string> getProjectVersionOrLatestStable(const std::string& slugOrId, const std::string& projectVersion, const std::string& mcVersion, const std::string& loader, std::unordered_set<std::string>& requestedDependencies, bool logVerbose = false, bool allowBetas = false, bool allowAlphas = false) {
    // make sure if we already have the slug/id in the set, we don't get it again and can simply return nothing
    if (requestedDependencies.contains(slugOrId)) {
        return {};
    }

    // possible version options
    std::vector<nlohmann::json> possibleVersions;

    // any notes that the program may need to add
    std::string loggingNotes;

    // This can be examined to give the user more detail on if their hit can be fulfilled by adding alphas, betas, et cetera
    // to include all release types add: "release", "beta", and "alpha"
    std::unordered_set<std::string> releaseTypeWhiteList{"release"};
    if (allowBetas) releaseTypeWhiteList.emplace("beta");
    if (allowAlphas) releaseTypeWhiteList.emplace("alpha");
    // [0] = release, [1] = beta, [2] = alpha
    int releaseTypeHits[3] = {0,0,0};


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

    if (logVerbose) {
        std::string releasesString = std::format("{} {} Releases {}", releaseTypeHits[0] > 0 ? Colors::GREEN : Colors::RESET, releaseTypeHits[0], Colors::RESET);
        std::string betasString = std::format("{} {} Betas {}", releaseTypeHits[1] > 0 ? Colors::GREEN : Colors::RESET, releaseTypeHits[1], Colors::RESET);
        std::string alphasString = std::format("{} {} Alphas {}", releaseTypeHits[2] > 0 ? Colors::GREEN : Colors::RESET, releaseTypeHits[2], Colors::RESET);
        loggingNotes += std::format("{} [{} | {} | {}]\n", slugOrId, releasesString, betasString, alphasString);
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
        // This is roughly the same error message, but the actual string must vary to help the user

        // Im going to be honest, if there is a release version that somehow isn't chosen, then fuck me I guess
        if (releaseTypeHits[0] > 0)
        ;
        // Betas available, but if it's not selected, then the user can be prompted to include betas
        else if (releaseTypeHits[1] > 0)
            loggingNotes += std::format("{}Could Not Find Recent Version For '{}', Found Beta Version, Try Including Betas, Or Wait For Releases; check: https://modrinth.com/mod/{} {}\n", Colors::RED, slugOrId, slugOrId, Colors::RESET);
        // Alphas available, but if it's not selected, then the user can be prompted to include alphas and/or betas
        else if (releaseTypeHits[2] > 0)
            loggingNotes += std::format("{}Could Not Find Recent Version For '{}', Found Alpha Version, Try Including Alphas, Or Wait For Releases Or Betas; check: https://modrinth.com/mod/{} {}\n", Colors::RED, slugOrId, slugOrId, Colors::RESET);
        else
            loggingNotes += std::format("{}Could Not Find Any Versions For '{}'; check: https://modrinth.com/mod/{} {}\n", Colors::RED, slugOrId, slugOrId, Colors::RESET);
        return {{}, loggingNotes};
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

        if (dep.first.has_value()) {
            project.addDependency(dep.first.value(), dependency["dependency_type"]);
            requestedDependencies.insert(dependency["project_id"].get<std::string>());
        } else {
            // once we start to get no more dependencies that are valid, we can exit
            break;
        }
    }

    return {project, loggingNotes};
}

std::pair<std::optional<ModrinthProject>, std::string> Modrinth::getModrinthProject(const std::string& name, const std::string& author, const std::string &organization, const std::string& mcVersion, const std::string& loader, const std::string& projectVersion, bool logVerbose, bool allowBetas, bool allowAlphas) {
    // get (or attempt to get) the slug of the project (basically its unique identifier)
    std::optional<ModrinthProjectSearchResult> proj = getProjectSlug(name, author, organization);

    // check to make sure a valid project was found
    if (!proj.has_value()) {
        // nothing else we can do here, so just return
        return {{}, std::format("{}Could Not Find Project '{}'{}\n", Colors::RED, name, Colors::RESET)};
    }

    std::unordered_set<std::string> requestedDependencies{};
    return getProjectVersionOrLatestStable(proj->slug, projectVersion, mcVersion, loader, requestedDependencies, logVerbose, allowBetas, allowAlphas);
}
