//
// Created by Owner on 8/27/2026.
//

#include "ModRequest.hpp"


bool ModRequest::conditionIsTrue(ModDeclareContext& context) const {
    // if the condition is empty, it should return true because its treated as not having conditions
    if (condition.empty()) return true;

    // special case when the condition simply doesn't exist
    if (!context.flags.contains(condition)) {
        // default behavior is to be false when condition does not exist
        if (!doInvertCondition()) return false;
        // if the condition should be inverted, then simply add a does not exist flag
        else return true;
    }
    // the condition should exist, so check its value
    if (!doInvertCondition() && (int)context.getFlag(condition) >= 0) return true;
    if (doInvertCondition() && (int)context.getFlag(condition) < 0) return true;

    return false;
}

std::pair<ModRequestParseResult, std::vector<ModRequest>> ModDeclareContext::parseModRequestsFromTokensList(const std::vector<std::string>& tokens) {

    std::vector<ModRequest> requests;
    // By Default, Allow 5 Warnings
    this->maxWarningsSetting = 5;
    this->maxErrorsSetting = 0;
    this->modLocation = "./";
    this->resourceLocation = "./";
    {
        int index = 0;
        ModRequest prefix;
        ModRequest request;
        for (const auto &tk: tokens) {
            // Basically: if "{" is found treat the line as the prefix rather than a request, the prefix will be the starting point for each request until it's cleared with "}"
            if (tk == "{") {
                prefix = request;
                if (!prefix.varName.empty()) {
                    std::print("{}Scopes Cannot Be Named (@ \"{}\"){}\n", Colors::RED, prefix.varName, Colors::RESET);
                    return {ModRequestParseResult::Error_NamedScope, {}};
                }
                index++;
                continue;
            }
                // Basically: Clear the prefix to a default mod request state, clearing it in essence
            else if (tk == "}") {
                request = prefix = ModRequest{};
                index++;
                continue;
            }

            switch (StringToTokenType(tk)) {
                // custom case for line separators:
                case TokenType::Separator: {
                    if (!request.name.empty()) {
                        // make sure the request has meet the conditions
                        /*if (!request.conditionIsTrue(*this)) {
                            std::print("{}condition ({}) for {} not met{}\n", Colors::YELLOW, request.condition, request.name, Colors::RESET);
                        } else {}*/
                        requests.push_back(request);
                    }
                    request = prefix;
                    break;
                }

                case TokenType::Require: request.setRequired(true); break;
                case TokenType::Desire: request.setRequired(false); break;
                case TokenType::Mod: request.name = removeQuotes(tokens[index + 1]); request.projectType = "mod"; break;
                case TokenType::Resource: request.name = removeQuotes(tokens[index + 1]); request.projectType = "resourcepack"; break;
                case TokenType::Shader: request.name = removeQuotes(tokens[index + 1]); request.projectType = "shader"; break;
                case TokenType::Loader: request.loader = removeQuotes(tokens[index + 1]); break;
                case TokenType::Author: request.author = removeQuotes(tokens[index + 1]); break;
                case TokenType::McVersion: request.mcVersion = removeQuotes(tokens[index + 1]); break;
                case TokenType::ProjectVersion: request.projectVersion = removeQuotes(tokens[index + 1]); request.setLatestVersion(false); break;
                case TokenType::LatestVersion: request.setLatestVersion(true); break;
                case TokenType::Organization: request.organization = removeQuotes(tokens[index + 1]); break;
                case TokenType::IncludeDeps: request.setIncludeDeps(true); break;
                case TokenType::IncludeAllDeps: request.setIncludeDeps(true); request.setIncludeAllDeps(true); break;
                case TokenType::AllowBetas: request.setAllowBetas(true); break;
                case TokenType::AllowAlphas: request.setAllowAlphas(true); break;
                case TokenType::Condition: request.condition = removeQuotes(tokens[index + 1]); break;
                case TokenType::NotCondition: request.condition = removeQuotes(tokens[index + 1]); request.setInvertCondition(true); break;
                case TokenType::RequestName: request.varName = tk.substr(0,tk.size()-1); break;

                case TokenType::MaxErrors: maxErrorsSetting = std::stoi(tokens[index + 1]); break;
                case TokenType::MaxWarnings: maxWarningsSetting = std::stoi(tokens[index + 1]); break;
                case TokenType::ModLocation: modLocation = removeQuotes(tokens[index + 1]); break;
                case TokenType::ResourceLocation: resourceLocation = removeQuotes(tokens[index + 1]); break;
                case TokenType::ShaderLocation: shaderLocation = removeQuotes(tokens[index + 1]); break;

                default:
                    if (tk[0] != '\"' && !isdigit(tk[0]))
                        std::print("{}\n", Colors::ColorString(Colors::YELLOW, std::format("Unknown Token '{}'", tk)));
                    break;
            }

            index++;
        }
        if (!request.name.empty()) {
            requests.push_back(request);
            if (!request.varName.empty()) {
                if (containsFlag(request.varName)) {
                    std::print("{}Named Request \"{}\" Already Exists {}\n", Colors::RED, request.varName, Colors::RESET);
                } else {
                    setFlag(request.varName, ProgramFlagState::Request);
                }
            }
        }
    }

    return {ModRequestParseResult::Success, requests};
}

std::unordered_map<std::string, std::pair<ModRequest, ModDeclareContext::ModrinthProjectDownloadData>> ModDeclareContext::getDownloadListFromRequests(const std::vector<ModRequest> &requests) {
    std::unordered_map<std::string, std::pair<ModRequest, ModrinthProjectDownloadData>> downloadsList;

    // display the download color labels, Find New Symbols To Help The Colorblind
    std::print("[{}* Success{} | {}! Not Found{} | {}? Conditions Not Met{} | {}: Added As Dependency{}]\n",
               Colors::GREEN, Colors::RESET,
               Colors::YELLOW, Colors::RESET,
               Colors::PURPLE, Colors::RESET,
               Colors::BLUE, Colors::RESET
   );

    bool logVerbose = flags.contains("verbose") && flags["verbose"] == ProgramFlagState::CommandLine;
    for (int j = 0; j < requests.size(); j++) {
        auto& req = requests[j];
        // make sure to not repeat the same mod if declared more than once
        if (downloadsList.contains(req.name)) continue;

        // before building the query and doing all the work, check the request's condition (if possible)
        // this would allow for checking if a mod was successfully downloaded
        if (!req.conditionIsTrue(*this)) {
            std::string conditionString = ((req.doInvertCondition()) ? "Not " : "") + req.condition;
            std::print("[{}] {}{}: Condition = {} {}\n", Colors::ColorString(Colors::PURPLE, "?"), Colors::PURPLE, req.name, conditionString, Colors::RESET);
            // conditions shouldn't give errors or warnings as they are used in case of either warnings or errors
            continue;
        }

        auto projectHit = Modrinth::getModrinthProject(
                req.name, req.author, req.organization, req.mcVersion, req.loader,
                req.projectVersion, logVerbose, req.doAllowBetas(), req.doAllowAlphas()
        );

        if (!projectHit.has_value()) {
            std::print("[{}] {}{}{}\n", Colors::ColorString((req.isRequired()) ? Colors::RED : Colors::YELLOW, "!"), ((req.isRequired()) ? Colors::RED : Colors::YELLOW), req.name + ((req.isRequired()) ? ": Required" : ": Not Required"), Colors::RESET);
            setFlag(req.varName, req.isRequired() ? ProgramFlagState::RequestError : ProgramFlagState::RequestWarn);
            if (req.isRequired()) errors++; else warnings++;
            continue;
        }

        std::print("[{}] {}{}{}\n", Colors::ColorString(Colors::GREEN, "*"), Colors::GREEN, req.name, Colors::RESET);
        ModrinthProjectDownloadData data{};
        data.fileName = projectHit->getFileName();
        data.fileURL = projectHit->getFileUrl();
        data.projectVersion = projectHit->getVersion();
        downloadsList[req.name] = {req, data};
        setFlag(req.varName, ProgramFlagState::RequestSuccess);


        if (req.doIncludeDeps()) {
            projectHit->flattenDependencies();
            for (auto& md : projectHit->flattenDependencies()) {
                bool required = md.first == "required";
                if ((!required && req.doIncludeAllDeps()) || required) {
                    // there is a slight concern here (but not one I can really fix do to the nature of the program)
                    // when a user asks for all dependencies of a mod to be added, there is a case where the title/slug may not be
                    // gotten correctly, so it in theory should default to the project ID, in this case conditions relying on
                    // dependencies become unpredictable, but dependencies are already unpredictable so it doesnt really matter.
                    auto displayName = Modrinth::getProjectDisplayNameAndSlugFromID(md.second.getSlugOrId());
                    std::string name = displayName.has_value() ? displayName->name : md.second.getSlugOrId();

                    // build the new dependency request
                    ModRequest depRequest;
                    depRequest.name = name;
                    depRequest.projectVersion = md.second.getVersion();
                    // inherent the project type from the parent
                    depRequest.projectType = req.projectType;

                    downloadsList[name] = {depRequest, getModrinthProjectDownloadDataFromSlug(md.second.getSlugOrId(), md.second.getVersion())};
                    setFlag(name, ProgramFlagState::RequestSuccess);
                    std::print("   [{}] {}{}{}{}\n", Colors::ColorString(Colors::BLUE, ":"), Colors::BLUE, name, (required ? ": Required" : ": Optional"), Colors::RESET);
                }
            }
            std::print("\n");
        }
    }
    std::print("\n");
    return downloadsList;
}

bool fileIsEmpty(std::ifstream& pFile) {
    return pFile.tellg() == 0 && pFile.peek() == std::ifstream::traits_type::eof();
}

void ModDeclareContext::filterDownloadsListFromDownloadMetaFile(std::unordered_map<std::string, std::pair<ModRequest, ModrinthProjectDownloadData>>& downloadsList) {
    // if the file doesn't exist, then no filtering is required and we can craete the file later
    if (!std::filesystem::exists(givenDirectory+"meta.require.json")) {
        return;
    }
    std::ifstream ifs(givenDirectory+"meta.require.json");
    if (ifs.is_open() && !fileIsEmpty(ifs)) {
        try {
            int originalDownloadListSize = downloadsList.size();
            // try to parse, this has a real possibility of failing if the file doesn't exist before this function was called
            nlohmann::json jf = nlohmann::json::parse(ifs);
            for (auto& entry : jf.items()) {
                if (downloadsList.contains(entry.key())) {
                    //std::print("{} exists in download meta, deleting from download list\n", entry.key());
                    downloadsList.erase(entry.key());
                }
            }
            std::print("{}{}/{} Already Downloaded{}\n", Colors::GREEN, originalDownloadListSize-downloadsList.size(), originalDownloadListSize, Colors::RESET);
        } catch (std::exception& e) {
            // try writing an empty json
            ifs.close();
            std::ofstream ofs(givenDirectory+"meta.require.json");
            if (ofs.is_open()) {
                ofs << "{}";
                ofs.close();
            }
        }
    }
}

void ModDeclareContext::downloadFromDownloadsList(const std::unordered_map<std::string, std::pair<ModRequest, ModDeclareContext::ModrinthProjectDownloadData>>& downloadsList) const {

    nlohmann::json downloadMetaJson;

    constexpr int progressBarSize = 20;

    int downloadCount = downloadsList.size(), downloaded = 0;
    // We Have the Downloads List Now, So Make Sure That Warnings And Errors Won't Cause Issues
    for (auto& project : downloadsList) {
        // version string: if requested latest version, use the latest version from the project json, otherwise, use the user defined version
        //std::print("Downloading: {} version: {}\n", project.first, project.second.first.latestVersion ? "latest" : project.second.first.projectVersion);
        //std::print("\t{} from {}\n", project.second.second.fileName, project.second.second.fileURL);

        std::string downloadPath = getModDownloadLocation();

        if (project.second.first.projectType == "resourcepack") {
            downloadPath = getResourceDownloadLocation();
        } else if (project.second.first.projectType == "shader") {
            downloadPath = getShaderDownloadLocation();
        }

        std::string savedFileName;
        if (!containsFlag("check")) {
            constexpr bool USE_USER_GIVEN_FILE_NAME = false;
            if constexpr (USE_USER_GIVEN_FILE_NAME) {
                // Download Based On Give Name From User File i.e. "Sodium"
                size_t extensionStartPos = project.second.second.fileName.rfind('.');
                std::string fileExtension = project.second.second.fileName.substr(extensionStartPos);
                savedFileName = project.first + fileExtension;
            } else {
                // Download Based On File Name Given From Modrinth API
                savedFileName = project.second.second.fileName;
            }
            downloadFileFromUrl(project.second.second.fileURL, downloadPath, savedFileName);

        }
        nlohmann::json downloadJson = {
                {"resource_name", project.second.second.fileName},
                {"saved_as",      savedFileName},
                {"version",       project.second.second.projectVersion}
        };
        downloadMetaJson[project.first] = downloadJson;

        downloaded++;
        float percentDownloaded = ((float) downloaded / (float) (downloadCount)) * 100.0f;
        int progressBarDownloadedCount = (int) ceil(percentDownloaded) / (100 / progressBarSize);

        std::print("[{}{}", Colors::GREEN, std::string(progressBarDownloadedCount, '#'));
        std::print("{}{}{}] ({:.1f}%)", Colors::WHITE, std::string(progressBarSize-progressBarDownloadedCount, '#'), Colors::RESET, percentDownloaded);
        std::print("[{}{}{}]\n", Colors::CYAN, project.first, Colors::RESET);
    }
    if (!containsFlag("check")) {
        std::ifstream ifs(givenDirectory+"meta.require.json");
        if (ifs.is_open()) {
            try {
                // try to parse, this has a real possibility of failing if the file doesn't exist before this function was called
                downloadMetaJson = nlohmann::json::parse(ifs);
            } catch (std::exception& e) {
                // if there is an error, we need to rebuild the json
            }
        }
        std::ofstream downloadMetaFile(givenDirectory+"meta.require.json");

        downloadMetaFile << downloadMetaJson.dump(1, '\t');

        downloadMetaFile.close();
    }
}