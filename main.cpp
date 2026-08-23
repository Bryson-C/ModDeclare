#include <iostream>
#include <print>
#include <format>
#include <unordered_map>

#include "File.hpp"
#include "UrlRequest.hpp"
#include "Modrinth.hpp"
#include "ANSIColors.hpp"

constexpr const char* SEPARATOR_CHAR = ",";

enum class TokenType {
    None = 0,
    Separator,
    Require,
    Mod,
    Resource,
    Loader,
    Author,
    Desire,
    McVersion,
    LatestVersion,
    ProjectVersion,
    Organization,
    IncludeDeps,
    IncludeAllDeps,
    // this will be used to check for command line flags when running a file
    // for instance, if I type "ModDeclare.exe 'mods.require' neoforge" and I have a "if 'neoforge'" condition,
    // only if the flag is specified will the code be added to the download script
    Condition,
    // These Are Not Per Project, But Per File
    MaxErrors,
    MaxWarnings,
    ModLocation,
    ResourceLocation,
};

std::string toLowerCase(const std::string& string) {
    std::string str = string;
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c){ return std::tolower(c); });
    return str;
}

enum TokenType StringToTokenType(const std::string& string) {
    std::string str = toLowerCase(string);
    if (str == SEPARATOR_CHAR || str == "\n") return TokenType::Separator;
    if (str == "require") return TokenType::Require;
    if (str == "mod") return TokenType::Mod;
    if (str == "resource" || str == "resource-pack") return TokenType::Resource;
    if (str == "loader") return TokenType::Loader;
    if (str == "author") return TokenType::Author;
    if (str == "desire") return TokenType::Desire;
    if (str == "mc-version" || str == "minecraft-version") return TokenType::McVersion;
    if (str == "proj-version" || str == "project-version") return TokenType::ProjectVersion;
    if (str == "latest" || str == "latest-version") return TokenType::LatestVersion;
    if (str == "organization" || str == "org") return TokenType::Organization;
    if (str == "include-dependencies" || str == "include-deps") return TokenType::IncludeDeps;
    if (str == "include-all-dependencies" || str == "include-all-deps") return TokenType::IncludeAllDeps;
    if (str == "max-errors") return TokenType::MaxErrors;
    if (str == "max-warnings") return TokenType::MaxWarnings;
    if (str == "mod-location") return TokenType::ModLocation;
    if (str == "resource-location") return TokenType::ResourceLocation;
    if (str == "condition" || str == "if") return TokenType::Condition;
    return TokenType::None;
}

std::string removeQuotes(const std::string& string) {
    return string.substr(1, string.size()-2);
}
std::string removeQuotesToLower(const std::string& string) {
    return toLowerCase(string.substr(1, string.size()-2));
}
template<typename T>
void pushVecToVec(std::vector<T>& vec, const std::vector<T>& other) {
    for (auto& i : other) {
        vec.push_back(i);
    }
}

struct ModRequest {
    // Modrinth API Cant Search Based On Organization, So We Have To Filter After The Call Is Actually Made To The API
    std::string name, loader, author, projectType, mcVersion, organization, projectVersion, condition;
    bool required = true, latestVersion = true, includeDeps = false, includeAllDeps = false;

    std::string buildSerachQuery() const {
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
            this->latestVersion = true;
        } else {
            this->projectVersion = version;
            this->latestVersion = false;
        }
    }
    void switchModSpecificDataViaSlugAndTitle(const std::string& slug, const std::string& title) {
        try {
            name = title;
            for (const auto& hit : Modrinth::getProjectSearchResultsCustomQuery(buildSerachQuery())) {
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
        std::print("Mod Request: {} from {} org {} version {}\n\tLoader: {}\n\tMinecraft Version: {}\n\tCondition: {}\n", name, author, organization, projectVersion, loader, mcVersion, condition);
    }
};

struct ModrinthProjectDownloadData {
    std::string fileURL, fileName;
};
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
            return data;
        }
    } catch (std::exception& e) {
        std::cout << "Error Downloading Project: " << e.what() << "\n";
    }
    return ModrinthProjectDownloadData{};
}

bool isBufferEmpty(std::string buffer) {
    return (buffer.empty() || buffer == " ");
}

enum class ProgramRunState {
    Download,
    Help,
    Check,
};

int main(int argc, char** argv) {

    ProgramRunState runState = ProgramRunState::Download;
    std::unordered_map<std::string, int> flags;

    // TODO: Add Logging Flags: Verbose, Otherwise Only Print Useful Info
    // TODO: Allow Other Project Types (Such As Resource Packs And Shaders)
    // TODO: Allow User To Specify Where They Want Resources To Be Downloaded To (Relative To The File Location)
    // TODO: Check Download Directory For Matching File Names To Avoid Redownloading -- It Would Be Nice To Know What Mods Can Be Deleted Automatically, But That Is Not Something I Care About Now

    std::vector<std::string> fileLines;
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
            flags.emplace(argv[i], 1);
        }
    }

    if (runState == ProgramRunState::Help) return 0;

    std::filesystem::path givenDirectory = std::filesystem::current_path();
    // if flags were specified
    if (argc > 1) {
        if (std::filesystem::is_directory(argv[1])) {
            std::print("folder\n");
            for (auto& file : std::filesystem::directory_iterator(std::filesystem::path(argv[1]))) {
                if (file.is_directory() || file.path().extension() != ".require") continue;
                std::print("Reading: {}\n", file.path().string());
                pushVecToVec(fileLines, ReadLines(file.path()));
            }
        }
        else if (std::filesystem::path(argv[1]).extension() == ".require") {
            std::print("file\n");
            pushVecToVec(fileLines, ReadLines(std::string(argv[1])));
            std::print("Reading: {}\n", argv[1]);
        }
        else {
            std::print("{}first argument must be a path to a folder or file{}\n", Colors::RED, Colors::RESET);
        }
    }
    // if no flags were specified
    else {
        std::print("Reading Current Working Directory: {}\n", std::filesystem::current_path().string());
    }


#else
    //string = ReadFile((std::string)"../MossNMoonProfileTest.require", true);
    pushVecToVec(fileLines, ReadLines((std::string)"../flags.require"));
#endif

    std::print("{}\n\n", std::string(10,'='));

    // tokenizer/line reader
    std::vector<std::string> tokens;
    {
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


    std::vector<ModRequest> requests;
    // By Default, Allow 5 Warnings
    int maxWarningsSetting = 5;
    int maxErrorsSetting = 0;
    std::string modLocation = "./";
    std::string resourceLocation = "./";
    {
        int index = 0;
        ModRequest prefix;
        ModRequest request;
        for (const auto &tk: tokens) {
            // Basically: if "{" is found treat the line as the prefix rather than a request, the prefix will be the starting point for each request until it's cleared with "}"
            if (tk == "{") {
                prefix = request;
                index++;
                continue;
            }
                // Basically: Clear the prefix to a default mod request state, clearing it in essence
            else if (tk == "}") {
                prefix = ModRequest{};
                index++;
                continue;
            }

            switch (StringToTokenType(tk)) {
                // custom case for line separators:
                case TokenType::Separator: {
                    if (!request.name.empty()) {
                        // make sure the request has meet the conditions
                        if (!request.condition.empty() && !flags.contains(request.condition)) {
                            std::print("{}condition ({}) for {} not met{}\n", Colors::YELLOW, request.condition, request.name, Colors::RESET);
                        } else {
                            requests.push_back(request);
                        }
                    }
                    request = prefix;
                    break;
                }

                case TokenType::Require: request.required = true; break;
                case TokenType::Desire: request.required = false; break;
                case TokenType::Mod: request.name = removeQuotes(tokens[index + 1]); request.projectType = "mod"; break;
                case TokenType::Resource: request.name = removeQuotes(tokens[index + 1]); request.projectType = "resourcepack"; break;
                case TokenType::Loader: request.loader = removeQuotes(tokens[index + 1]); break;
                case TokenType::Author: request.author = removeQuotes(tokens[index + 1]); break;
                case TokenType::McVersion: request.mcVersion = removeQuotes(tokens[index + 1]); break;
                case TokenType::ProjectVersion: request.projectVersion = removeQuotes(tokens[index + 1]); request.latestVersion = false; break;
                case TokenType::LatestVersion: request.latestVersion = true; break;
                case TokenType::Organization: request.organization = removeQuotes(tokens[index + 1]); break;
                case TokenType::IncludeDeps: request.includeDeps = true; break;
                case TokenType::IncludeAllDeps: request.includeDeps = true; request.includeAllDeps = true; break;
                case TokenType::Condition: request.condition = removeQuotes(tokens[index + 1]); break;

                case TokenType::MaxErrors: maxErrorsSetting = std::stoi(tokens[index + 1]); break;
                case TokenType::MaxWarnings: maxWarningsSetting = std::stoi(tokens[index + 1]); break;
                case TokenType::ModLocation: modLocation = removeQuotes(tokens[index + 1]); break;
                case TokenType::ResourceLocation: resourceLocation = removeQuotes(tokens[index + 1]); break;


                default:
                    if (tk[0] != '\"')
                        std::print("{}\n", Colors::ColorString(Colors::YELLOW, std::format("Unknown Token '{}'", tk)));
                    break;
            }

            index++;
        }
        if (!request.name.empty()) {
            requests.push_back(request);
        }
    }


    std::unordered_map<std::string, std::pair<ModRequest, ModrinthProjectDownloadData>> downloadsList;
    int warnings = 0, errors = 0;
    for (int j = 0; j < requests.size(); j++) {
        const auto& req = requests[j];
        // The Final Hit That Will Be Downloaded After All Processing Is Done
        ModrinthProject finalHit;

        auto query = req.buildSerachQuery();
#ifdef PRINT_QUERIES
        std::print("{} Sending Request: {} {}\n", Colors::CYAN, query, Colors::RESET);
#endif
        auto hits = Modrinth::getProjectSearchResultsCustomQuery(query);
        // Default Cases That Don't Require Filtering
        if (hits.empty()) {
            std::print("{}No Hits For '{}' -- {}{}\n", ((req.required) ? Colors::RED+"Error: " : Colors::YELLOW+"Warning: "), req.name, ((req.required) ? "Required" : "Not Required"), Colors::RESET);
            if (req.required) errors++; else warnings++;
        }
        else if (hits.size() == 1) {
            std::print("{}Hit{}: {}\n", Colors::GREEN, Colors::RESET, hits[0].toColoredString());
            finalHit = hits[0];
            goto CONTINUE_TO_DOWNLOAD;
        }
        // Cases That May Require Filtering
        else if (hits.size() > 1) {

            // Filter More In Depth Over All The Hits; Remove Non-Matching
            // Another thing to note: when using queries, facets are case-sensitive, so only after getting the query result we can lowercase both sides
            for (int i = 0; i < hits.size(); i++) {
                if (toLowerCase(hits[i].organization) != toLowerCase(req.organization)) {
                    hits.erase(hits.begin()+i);
                    // make sure the iteration stays the same cause erasure moves the array
                    i--;
                    continue;
                }
                // if the mod title is exactly what would be found on the modrinth app, then just go for it
                if (toLowerCase(hits[i].title) == toLowerCase(req.name)) {
                    ModrinthProject hit = hits[i];
                    hits.clear();
                    hits.push_back(hit);
                    break;
                }
            }

            if (hits.size() == 1) {
                std::print("{}Hit{}: {}\n", Colors::GREEN, Colors::RESET, hits[0].toColoredString());
                finalHit = hits[0];
                goto CONTINUE_TO_DOWNLOAD;
            }
            // if organization is set, and no matches were found, give a warning
            else if (hits.size() > 1) {
                std::print("{}Multiple Hits ({}), Specify More Parameters {}\n", (req.required ? Colors::RED+"Error: " : Colors::YELLOW+"Warning: "), hits.size(), Colors::RESET);

                for (const auto& hit: hits) {
                    std::print("\t{}Possibly{}: {} by {}\n", Colors::YELLOW, Colors::RESET, Colors::ColorString(Colors::BLUE, hit.title), Colors::ColorString(Colors::GREEN, hit.author));
                }
                if (req.required) errors++; else warnings++;
            } else if (hits.empty()) {
                std::print("{}No Hits After Filtering For '{}' -- {}{}\n", ((req.required) ? Colors::RED+"Error: " : Colors::YELLOW+"Warning: "), req.name, ((req.required) ? "Required" : "Not Required"), Colors::RESET);
                if (req.required) errors++; else warnings++;
            }
        }
        CONTINUE_TO_DOWNLOAD:
        if (!finalHit.title.empty()) {
            downloadsList[finalHit.title] = {req, getModrinthProjectDownloadDataFromSlug(finalHit.slug, req.latestVersion ? finalHit.latest_version : req.projectVersion)};
            // since we know the mod here, we can finally get the dependencies (if user specified)
            if (req.includeDeps) {
                std::string version = req.latestVersion ? finalHit.latest_version : req.projectVersion;
                auto modVersion = getJsonFromRequestUrl(req.buildVersionQuery(finalHit.slug, version));
                for (const auto &dep: modVersion["dependencies"]) {
                    auto getDep = getJsonFromRequestUrl(cleanStringForUrl(std::format("https://api.modrinth.com/v2/project/{}", dep["project_id"].get<std::string>())));
                    bool required = dep["dependency_type"] == "required";
                    if ((!required && req.includeAllDeps) || required) {
                        std::print("{}Adding {} dependency: {}{} {}\n", Colors::BLUE, req.name, getDep["title"].get<std::string>(), Colors::RESET, required ? Colors::ColorString(Colors::RED, "(Required)") : Colors::ColorString( Colors::YELLOW, "(Optional)"));
                        // by this point we know that the dependency is wanted, either required or optionally requested
                        // so we can just add it to the end of the request queue (vector)

                        try {
                            ModRequest depRequest;
                            depRequest.name = getDep["title"].get<std::string>();
                            depRequest.projectVersion = getDep["versions"].back().get<std::string>();
                            depRequest.projectType = getDep["project_type"].back().get<std::string>();

                            // FIXME: Make Sure That We Are Downloading The Right Version Of The Dependency Rather Than Just The Latest Version
                            downloadsList[getDep["title"]] = {depRequest, getModrinthProjectDownloadDataFromSlug(getDep["slug"].get<std::string>(), getDep["versions"].back().get<std::string>())};
                        } catch (std::exception& e) {
                            std::print("Error Adding Dependency: {}\n", e.what());
                        }

                    }
                }
            }
        }
        std::print("\n");
    }

    std::print("{}\n{}\n\n", Colors::ColorString(Colors::YELLOW, "Warnings: "+std::to_string(warnings)+" | Max Allowed: "+std::to_string(maxWarningsSetting)),
                             Colors::ColorString(Colors::RED, "Errors: "+std::to_string(errors)+" | Max Allowed: "+std::to_string(maxErrorsSetting)));

    if (warnings > maxWarningsSetting) {
        std::print("{}\n", Colors::ColorString(Colors::RED, "Exceeded Maximum Allowed Warnings; Exiting"));
        return -1;
    }
    if (errors > maxErrorsSetting) {
        std::print("{}\n", Colors::ColorString(Colors::RED, "Exceeded Maximum Allowed Errors; Exiting"));
        return -1;
    }

    // We Have the Downloads List Now, So Make Sure That Warnings And Errors Won't Cause Issues
    for (auto& project : downloadsList) {
        // version string: if requested latest version, use the latest version from the project json, otherwise, use the user defined version
        std::print("Downloading: {} version: {}\n", project.first, project.second.first.latestVersion ? "latest" : project.second.first.projectVersion);
        std::print("\t{} from {}\n", project.second.second.fileName, project.second.second.fileURL);

#define RELEASE_DOWNLOAD
#ifdef RELEASE_DOWNLOAD

        std::string downloadPath = modLocation;

        if (project.second.first.projectType == "resourcepack") {
            downloadPath = resourceLocation;
        }
        downloadFileFromUrl(project.second.second.fileURL, downloadPath, project.second.second.fileName);
#endif
    }


    return 0;
}
