//
// Created by Owner on 6/23/2026.
//

#ifndef COGITOPLATFORM_MODRINTHPROJECT_HPP
#define COGITOPLATFORM_MODRINTHPROJECT_HPP

#include <iostream>
#include <string>
#include <vector>
#include <print>

#include "nlohmann/json.hpp"

#include "../Util/ANSIColors.hpp"
#include "../Util/UrlRequest.hpp"

class ModrinthProjectSearchResult {
private:
    static std::string getJsonStringOrEmpty(const nlohmann::json& json, const std::string& key) {
        if (json.contains(key) && !json[key].is_null()) {
            return json[key].get<std::string>();
        }
        return "";
    }
    static int getJsonIntOr0(const nlohmann::json& json, const std::string& key) {
        if (json.contains(key) && !json[key].is_null()) {
            return json[key].get<int>();
        }
        return 0;
    }
public:

    enum class ProjectType {
        Unknown,
        // mod includes plugins and datapacks im pretty sure
        Mod,
        ModPack,
        ResourcePack,
        Shader,
    };

    static ProjectType projectTypeFromString(std::string str) {
        if (str == "mod") return ProjectType::Mod;
        if (str == "modpack") return ProjectType::ModPack;
        if (str == "resourcepack") return ProjectType::ResourcePack;
        if (str == "shader") return ProjectType::Shader;
        return ProjectType::Unknown;
    }
    static std::string projectTypeToString(ProjectType type) {
        switch (type) {
            case ProjectType::Mod: return "mod";
            case ProjectType::ModPack: return "modpack";
            case ProjectType::ResourcePack: return "resourcepack";
            case ProjectType::Shader: return "shader";
            default: break;
        }
        return "unknown";
    }

    enum class RequirementStatus {
        Required,
        Unsupported
    };

    static RequirementStatus requirementStatusFromString(std::string str) {
        return str == "required" ? RequirementStatus::Required : RequirementStatus::Unsupported;
    }

    std::string icon_url;
    ProjectType project_type;
    int color;
    std::string author;
    std::string date_created;
    int follows;
    std::string description;
    std::string title;
    RequirementStatus client_side;
    std::string license;
    std::string featured_gallery;
    std::string date_modified;
    std::string latest_version;
    std::string project_id;
    std::vector<std::string> versions;
    int downloads;
    std::string organization;
    std::string organization_id;
    std::vector<std::string> display_categories;
    std::vector<std::string> categories;
    RequirementStatus server_side;
    std::string author_id;
    std::string slug;
    std::vector<std::string> gallery;

    ModrinthProjectSearchResult() = default;

    static ModrinthProjectSearchResult getProjectFromJson(nlohmann::json json) {
        ModrinthProjectSearchResult project;
        project.icon_url = getJsonStringOrEmpty(json, "icon_url");
        project.project_type = projectTypeFromString(getJsonStringOrEmpty(json, "project_type"));
        project.color = getJsonIntOr0(json, "color");
        project.author = getJsonStringOrEmpty(json, "author");
        project.date_created = getJsonStringOrEmpty(json, "date_created");
        project.follows = getJsonIntOr0(json, "follows");
        project.description = getJsonStringOrEmpty(json, "description");
        project.title = getJsonStringOrEmpty(json, "title");
        project.client_side = requirementStatusFromString(getJsonStringOrEmpty(json, "client_side"));
        project.license = getJsonStringOrEmpty(json, "license");
        project.featured_gallery = getJsonStringOrEmpty(json, "featured_gallery");
        project.date_modified = getJsonStringOrEmpty(json, "date_modified");
        project.latest_version = getJsonStringOrEmpty(json, "latest_version");
        project.project_id = getJsonStringOrEmpty(json, "project_id");

        project.versions = std::vector<std::string>();
        for (const auto &version: json["versions"]) {
            project.versions.push_back((std::string) version);
        }

        project.downloads = json["downloads"];
        project.organization = getJsonStringOrEmpty(json, "organization");
        project.organization_id = getJsonStringOrEmpty(json, "organization_id");

        project.display_categories = std::vector<std::string>();
        for (const auto &category: json["display_categories"]) {
            project.display_categories.push_back((std::string) category);
        }

        project.server_side = requirementStatusFromString(getJsonStringOrEmpty(json, "server_side"));
        project.author_id = getJsonStringOrEmpty(json, "author_id");
        project.slug = getJsonStringOrEmpty(json, "slug");

        project.gallery = std::vector<std::string>();
        for (const auto &imageUrl: json["gallery"]) {
            project.gallery.push_back((std::string) imageUrl);
        }
        return project;
    }

    ModrinthProjectSearchResult(nlohmann::json json) {
        *this = getProjectFromJson(json);
    }

    std::string toColoredString() const {
        return (std::string) Colors::BLUE + title + Colors::RESET + " -- " + Colors::GREEN + description + Colors::RESET;
    }
};

/**
 * ModrinthProject is a slug with a version number
 * (along with any additional details which may be useful for downloading a project such as the download link)
 */
class ModrinthProject {
private:
    std::string slugOrId, version;
    nlohmann::json rawJson;

    std::vector<std::pair<std::string, ModrinthProject>> dependencies;

    void getJsonDataIfNotAssigned() {
        if ((rawJson.empty() || rawJson.is_null()) && (!slugOrId.empty() && !version.empty())) {
            rawJson = getJsonFromRequestUrl(cleanStringForUrl(
               std::format(R"(https://api.modrinth.com/v2/project/{}/version/{})", slugOrId, version)
            ));
        } else {
            std::print("Cannot Get Json Data For Modrinth Project: Not Enough Info To Send Request\n");
        }
    }
public:
    explicit ModrinthProject(const nlohmann::json& json) {
        slugOrId = json["project_id"].get<std::string>();
        version = json["id"].get<std::string>();
        rawJson = json;
    }
    ModrinthProject(const std::string& slugOrId, const std::string& version) : slugOrId{slugOrId}, version{version} {
        getJsonDataIfNotAssigned();
    }

    void addDependency(const ModrinthProject& dep, const std::string& dependencyType) {
        dependencies.emplace_back(dependencyType, dep);
    }

    // takes the tree of dependencies and their dependencies (and so on) and puts them in a 1d array along with their requirement status
    std::vector<std::pair<std::string, ModrinthProject>> flattenDependencies() {
        std::vector<std::pair<std::string, ModrinthProject>> depends;
        // then flatten the dependencies from a tree like structure to a simple array to iterate over
        std::function<void(const ModrinthProject&)> getAllDeps = [&](const ModrinthProject& project) {
            for (auto& dep : project.getDependencies()) {
                depends.emplace_back(dep);
                getAllDeps(dep.second);
            }
        };
        getAllDeps(*this);
        return depends;
    }

    const std::string& getVersion() const { return version; }
    const std::string& getSlugOrId() const { return slugOrId; }
    const std::vector<std::pair<std::string,ModrinthProject>>& getDependencies() const { return dependencies; }

    [[nodiscard]] std::string getFileName() const {
        return rawJson["files"][0]["filename"].get<std::string>();
    }
    [[nodiscard]] std::string getFileUrl() const {
        return rawJson["files"][0]["url"].get<std::string>();
    }
    [[nodiscard]] std::string getProjectType() const {
        return rawJson["project_type"].get<std::string>();
    }

};

#endif //COGITOPLATFORM_MODRINTHPROJECT_HPP
