//
// Created by Owner on 6/23/2026.
//

#ifndef COGITOPLATFORM_MODRINTHPROJECT_HPP
#define COGITOPLATFORM_MODRINTHPROJECT_HPP

#include <iostream>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

#include "../Util/ANSIColors.hpp"

class ModrinthProject {
private:
    static std::string getJsonStringOrEmpty(nlohmann::json json, std::string key) {
        if (json.contains(key) && !json[key].is_null()) {
            return json[key].get<std::string>();
        }
        return "";
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

    ModrinthProject() = default;

    static ModrinthProject getProjectFromJson(nlohmann::json json) {
        ModrinthProject project;
        project.icon_url = getJsonStringOrEmpty(json, "icon_url");
        project.project_type = projectTypeFromString(getJsonStringOrEmpty(json, "project_type"));
        project.color = json["color"];
        project.author = getJsonStringOrEmpty(json, "author");
        project.date_created = getJsonStringOrEmpty(json, "date_created");
        project.follows = json["follows"];
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

    ModrinthProject(nlohmann::json json) {
        *this = getProjectFromJson(json);
    }

    std::string toColoredString() const {
        return (std::string) Colors::BLUE + title + Colors::RESET + " -- " + Colors::GREEN + description + Colors::RESET;
    }
};

#endif //COGITOPLATFORM_MODRINTHPROJECT_HPP
