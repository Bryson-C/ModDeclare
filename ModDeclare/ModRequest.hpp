//
// Created by Owner on 8/27/2026.
//

#ifndef MODDECLARE_MODREQUEST_HPP
#define MODDECLARE_MODREQUEST_HPP

#include <string>
#include <vector>
#include <print>

#include "Modrinth/Modrinth.hpp"
#include "Util/UrlRequest.hpp"
#include "Tokenizer.hpp"

enum class ProgramRunState {
    Download,
    Help,
    Check,
};

// negatives are errors, positives and 0 are non-errors, this way I can easily check whether a flag's condition is erroneous with a simple operation
enum class ProgramFlagState {
    // this means the flag was set on the command line
    CommandLine = 0,
    // these mean the flag was created from a request name (like "sodium_mod:"), the exact value is decided after the hit is processed
    // All named-requests start as the simple "Request" flag
    Request = 1,
    RequestSuccess = 2,
    RequestDNE = -1,
    RequestWarn = -2,
    RequestError = -3,
};

class ModDeclareContext;

struct ModRequest {

private:
    enum ReqFlags : uint8_t {
        REQUIRED            = 0b00000001,
        LATEST_VERSION      = 0b00000010,
        INCLUDE_DEPS        = 0b00000100,
        INCLUDE_ALL_DEPS    = 0b00001000,
        INVERT_CONDITION    = 0b00010000,
        ALLOW_BETAS         = 0b00100000,
        ALLOW_ALPHAS        = 0b01000000,
    };
    int modRequestFlags = (ReqFlags::REQUIRED | ReqFlags::LATEST_VERSION);
public:


    // Modrinth API Cant Search Based On Organization, So We Have To Filter After The Call Is Actually Made To The API
    std::string name, loader, author, projectType, mcVersion, organization, projectVersion, condition, varName;
    //bool required = true, latestVersion = true, includeDeps = false, includeAllDeps = false, invertCondition = false;
    [[nodiscard]] bool isRequired() const { return modRequestFlags & REQUIRED; }
    [[nodiscard]] bool isLatestVersion() const { return modRequestFlags & LATEST_VERSION; }
    [[nodiscard]] bool doIncludeDeps() const { return modRequestFlags & INCLUDE_DEPS; }
    [[nodiscard]] bool doIncludeAllDeps() const { return modRequestFlags & INCLUDE_ALL_DEPS; }
    [[nodiscard]] bool doInvertCondition() const { return modRequestFlags & INVERT_CONDITION; }
    [[nodiscard]] bool doAllowBetas() const { return modRequestFlags & ALLOW_BETAS; }
    [[nodiscard]] bool doAllowAlphas() const { return modRequestFlags & ALLOW_ALPHAS; }

    void setRequired(bool required = true) {
        if (required)
            modRequestFlags |= REQUIRED;
        else
            modRequestFlags &= ~REQUIRED;
    }
    void setLatestVersion(bool latestVersion = true) {
        if (latestVersion)
            modRequestFlags |= LATEST_VERSION;
        else
            modRequestFlags &= ~LATEST_VERSION;

    }
    void setIncludeDeps(bool includeDeps = true) {
        if (includeDeps)
            modRequestFlags |= INCLUDE_DEPS;
        else
            modRequestFlags &= ~INCLUDE_DEPS;
    }
    void setIncludeAllDeps(bool includeAllDeps = true) {
        if (includeAllDeps)
            modRequestFlags |= INCLUDE_ALL_DEPS;
        else
            modRequestFlags &= ~INCLUDE_ALL_DEPS;
    }
    void setInvertCondition(bool invertCondition = true) {
        if (invertCondition)
            modRequestFlags |= INVERT_CONDITION;
        else
            modRequestFlags &= ~INVERT_CONDITION;
    }
    void setAllowBetas(bool allow = true) {
        if (allow)
            modRequestFlags |= ALLOW_BETAS;
        else
            modRequestFlags &= ~ALLOW_BETAS;
    }
    void setAllowAlphas(bool allow = true) {
        if (allow)
            modRequestFlags |= ALLOW_ALPHAS;
        else
            modRequestFlags &= ~ALLOW_ALPHAS;
    }


    // FIXME: REMOVE/MERGE FUNCTIONALITY THAT SHOULD ONLY BE ONE FUNCTION

    std::string buildSearchQuery() const {
        constexpr int LIMIT = 5;
        std::string query = "https://api.modrinth.com/v2/search?query=" + cleanStringForUrl(name) + "&limit=" + std::to_string(LIMIT);

        std::string facets;
        int facetCount = 0;

        // facets are features of mods only
        facets += "&facets=[";

        if (!loader.empty()) {
            facets += cleanStringForUrl(std::format("[\"categories:{}\"]", loader));
            facetCount++;
        }
        if (!author.empty()) {
            facets += ((facetCount > 0 ? "," : "")) + (cleanStringForUrl(std::format("[\"author:{}\"]", author)));
            facetCount++;
        }
        if (!projectType.empty()) {
            facets += ((facetCount > 0 ? "," : "")) + (cleanStringForUrl(std::format("[\"project_type:{}\"]", projectType)));
            facetCount++;
        }
        if (!mcVersion.empty()) {
            facets += ((facetCount > 0 ? "," : "")) + (cleanStringForUrl(std::format("[\"versions:{}\"]", mcVersion)));
            facetCount++;
        }

        facets += "]";

        //"https://api.modrinth.com/v2/search?query=%22Sodium%22&facets=[[%22categories:fabric%22],[%22versions:1.17.1%22],[%22project_type:mod%22],[%22license:mit%22]]"

        return cleanStringForUrl(query + facets);
    }

    std::string buildVersionQuery(const std::string& slug, const std::string& version) const {
        return cleanStringForUrl(std::format("https://api.modrinth.com/v2/project/{}/version/{}", slug, version));
    }

    void switchModSpecificData(const std::string& name, const std::string& author, const std::string& organization, const std::string& version) {
        this->name = name;
        this->author = author;
        this->organization = organization;
        if (version.empty()) {
            this->projectVersion = "";
            this->setLatestVersion(true);
        } else {
            this->projectVersion = version;
            this->setLatestVersion(false);
        }
    }

    void switchModSpecificDataViaSlugAndTitle(const std::string& slug, const std::string& title) {
        try {
            name = title;
            for (const auto& hit : Modrinth::getProjectSearchResultsCustomQuery(buildSearchQuery())) {
                if (hit.slug != slug) {
                    continue;
                } else {
                    switchModSpecificData(title, hit.author, hit.organization, hit.latest_version);
                    return;
                }
            }
        } catch(std::exception& e) {
            std::print("Error Switching Mod Via Slug: {}\n", e.what());
        }
    }

    void print() const {
        std::print("Mod Request: {} ({})\n", name, loader);
    }

    void printVerbose() const {
        std::print("Mod Request: {} from {} org {} version {}\n\tLoader: {}\n\tMinecraft Version: {}\n\tCondition: {}\n\tVariable Name: {}\n", name, author, organization, projectVersion, loader, mcVersion, condition, varName);
    }

    [[nodiscard]] bool conditionIsTrue(ModDeclareContext& context) const;
};


template<typename T>
void pushVecToVec(std::vector<T>& vec, const std::vector<T>& other) {
    for (auto& i : other) {
        vec.push_back(i);
    }
}

enum class ModDeclareContextResult {
    Success = 0,
    HelpMenu = 1,
    Error = -1,
};

enum class ModRequestParseResult {
    Error_Unknown = -1,
    Error_NamedScope = -2,
    Success = 0,
};


class ModDeclareContext {
private:
    friend struct ModRequest;

    // download by default
    ProgramRunState runState = ProgramRunState::Download;
    // all the flags that a program has set
    std::unordered_map<std::string, ProgramFlagState> flags;

    // this is to store read file lines, under no context should it be modified after initially read
    std::vector<std::string> fileLines;

    // the directory specified on the command line, i.e. if I ran "../" that's what the CWD should be set to
    // when using relative paths, by default it should just be the current directory
    std::string givenDirectory = "./";

    // Project Settings, not file specific
    int maxWarningsSetting = 5;
    int maxErrorsSetting = 0;
    std::string modLocation = "./";
    std::string resourceLocation = "./";
    std::string shaderLocation = "./";
    int warnings = 0, errors = 0;

public:
    struct ModrinthProjectDownloadData {
        std::string fileURL, fileName, projectVersion;
    };
private:
    ModrinthProjectDownloadData getModrinthProjectDownloadDataFromSlug(const std::string& slug, const std::string& projectVersion) {
        //std::string url = std::format(R"(https://api.modrinth.com/v2/project/{}/version?loaders=["{}"]&game_versions=["{}"])", project.slug, request.loader, request.mcVersion);
        // ^ this would be the url to get a version if we did not have it previously based off of the search
        try {
            std::string url = std::format(R"(https://api.modrinth.com/v2/project/{}/version/{})", slug, projectVersion);
            auto json = getJsonFromRequestUrl(cleanStringForUrl(url));
            if (!json.empty()) {
                ModrinthProjectDownloadData data{};
                data.fileName = json["files"][0]["filename"];
                data.fileURL = json["files"][0]["url"];
                data.projectVersion = projectVersion;
                return data;
            }
        } catch (std::exception& e) {
            std::cout << "Error Downloading Project: " << e.what() << "\n";
        }
        return ModrinthProjectDownloadData{};
    }



public:

    ModDeclareContextResult parseCommandLineArgs(int argc, char** argv) {

#define RELEASE
#ifdef RELEASE

        for (int i = 0; i < argc; i++) {
            if (toLowerCase(std::string(argv[i])) == "help") {
                std::print("Usage:\n");
                std::print("The first command line argument (besides the .exe) must be a folder or file if \"help\" flag is not specified\n");
                runState = ProgramRunState::Help;
            }
                // if the flag is a user flag
            else {
                flags.emplace(toLowerCase(argv[i]), ProgramFlagState::CommandLine);
            }
        }

        // I dont know what the fuck this was written for
        if (runState == ProgramRunState::Help) ModDeclareContextResult::HelpMenu;

        givenDirectory = std::filesystem::current_path().string();
        // FIXME: If Only The Executable Is Given As A Command i.e.
        //  "ModDeclare.exe" Then Take The Path It Was Executed In As The Directory And Scan From There
        // if flags were specified
        if (argc > 1) {
            if (std::filesystem::is_directory(argv[1])) {
                // take the given directory as the working path
                givenDirectory = std::filesystem::path(argv[1]).string();
                for (auto& file : std::filesystem::directory_iterator(std::filesystem::path(argv[1]))) {
                    if (file.is_directory() || file.path().extension() != ".require") continue;
                    std::print("Reading: {}\n", file.path().string());
                    pushVecToVec(fileLines, ReadLines(file.path()));
                }
            }
            else if (std::filesystem::path(argv[1]).extension() == ".require") {
                // take the directory of the given file path as the working path
                givenDirectory = std::filesystem::path(argv[1]).parent_path().string();
                pushVecToVec(fileLines, ReadLines(std::string(argv[1])));
                std::print("Reading: {}\n", argv[1]);
            }
            else {
                if (runState != ProgramRunState::Help)
                    std::print("{}first argument must be a path to a folder or file{}\n", Colors::RED, Colors::RESET);
            }
        }
            // if no flags were specified
        else {
            std::print("Reading Current Working Directory: {}\n", givenDirectory);
        }


#else
        //string = ReadFile((std::string)"../MossNMoonProfileTest.require", true);
        pushVecToVec(fileLines, ReadLines((std::string)"../flags.require"));
#endif


        return ModDeclareContextResult::Success;
    }

    [[nodiscard]] const std::vector<std::string>& getParsedFileLines() const {
        return fileLines;
    }

    std::pair<ModRequestParseResult, std::vector<ModRequest>> parseModRequestsFromTokensList(const std::vector<std::string>& tokens);

    std::unordered_map<std::string, std::pair<ModRequest, ModrinthProjectDownloadData>> getDownloadListFromRequests(const std::vector<ModRequest>& requests);

    void filterDownloadsListFromDownloadMetaFile(std::unordered_map<std::string, std::pair<ModRequest, ModrinthProjectDownloadData>>& downloadsList);

    void downloadFromDownloadsList(const std::unordered_map<std::string, std::pair<ModRequest, ModrinthProjectDownloadData>>& downloadsList) const;

    bool containsFlag(const std::string& flag) const {
        return !flag.empty() && flags.contains(flag);
    }

    ProgramFlagState getFlag(const std::string& flag) const {
        return flags.at(flag);
    }

    void setFlag(const std::string& flag, ProgramFlagState flagState) {
        flags[flag] = flagState;
    }

    [[nodiscard]] int getMaxErrors() const { return maxErrorsSetting; }
    [[nodiscard]] int getMaxWarnings() const { return maxWarningsSetting; }
    [[nodiscard]] std::string getModDownloadLocation() const { return givenDirectory+modLocation; }
    [[nodiscard]] std::string getResourceDownloadLocation() const { return givenDirectory+resourceLocation; }
    [[nodiscard]] std::string getShaderDownloadLocation() const { return givenDirectory+shaderLocation; }

    [[nodiscard]] int getWarnings() const { return warnings; }
    [[nodiscard]] int getErrors() const { return errors; }
};


#endif //MODDECLARE_MODREQUEST_HPP
