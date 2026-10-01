//
// Created by Owner on 8/27/2026.
//

#ifndef MODDECLARE_TOKENIZER_HPP
#define MODDECLARE_TOKENIZER_HPP

#include <string>
#include <vector>
#include <algorithm>


enum class TokenType {
    None = 0,
    RequestName,
    Separator,
    Require,
    Mod,
    Resource,
    Shader,
    Plugin,
    Loader,
    Author,
    Desire,
    McVersion,
    LatestVersion,
    ProjectVersion,
    Organization,
    IncludeDeps,
    IncludeAllDeps,
    AllowBetas,
    AllowAlphas,
    // this will be used to check for command line flags when running a file
    // for instance, if I type "ModDeclare.exe 'mods.require' neoforge" and I have a "if 'neoforge'" condition,
    // only if the flag is specified will the code be added to the download script
    Condition,
    // Opposite of the Condition Token
    NotCondition,
    // These Are Not Per Project, But Per File
    MaxErrors,
    MaxWarnings,
    ModLocation,
    ResourceLocation,
    ShaderLocation,
    PluginLocation
};

namespace {

    constexpr const char *SEPARATOR_CHAR = ",";

    std::string toLowerCase(const std::string &string) {
        std::string str = string;
        std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
        return str;
    }

    enum TokenType StringToTokenType(const std::string &string) {
        std::string str = toLowerCase(string);
        if (str == SEPARATOR_CHAR || str == "\n") return TokenType::Separator;
        if (str == "require") return TokenType::Require;
        if (str == "mod") return TokenType::Mod;
        if (str == "plugin") return TokenType::Plugin;
        if (str == "resource" || str == "resource-pack") return TokenType::Resource;
        if (str == "shader") return TokenType::Shader;
        if (str == "loader") return TokenType::Loader;
        if (str == "author") return TokenType::Author;
        if (str == "desire") return TokenType::Desire;
        if (str == "mc-version" || str == "minecraft-version") return TokenType::McVersion;
        if (str == "proj-version" || str == "project-version") return TokenType::ProjectVersion;
        if (str == "latest" || str == "latest-version") return TokenType::LatestVersion;
        if (str == "organization" || str == "org") return TokenType::Organization;
        if (str == "include-dependencies" || str == "include-deps") return TokenType::IncludeDeps;
        if (str == "include-all-dependencies" || str == "include-all-deps") return TokenType::IncludeAllDeps;
        if (str == "allow-betas" || str == "betas") return TokenType::AllowBetas;
        if (str == "allow-alphas" || str == "alphas") return TokenType::AllowAlphas;
        if (str == "max-errors") return TokenType::MaxErrors;
        if (str == "max-warnings") return TokenType::MaxWarnings;
        if (str == "mod-location") return TokenType::ModLocation;
        if (str == "resource-location") return TokenType::ResourceLocation;
        if (str == "shader-location") return TokenType::ShaderLocation;
        if (str == "plugin-location") return TokenType::PluginLocation;
        if (str == "condition" || str == "if") return TokenType::Condition;
        if (str == "not" || str == "if-not") return TokenType::NotCondition;
        // ResourceName is a special case, it can be named whatever so long as the last token is a colon
        // the token name must also have more than 1 token (i.e. not just a colon)
        if (str.size() > 1 && str.back() == ':') return TokenType::RequestName;
        return TokenType::None;
    }

    std::string removeQuotes(const std::string &string) {
        return string.substr(1, string.size() - 2);
    }

    std::string removeQuotesToLower(const std::string &string) {
        return toLowerCase(string.substr(1, string.size() - 2));
    }

}

std::vector<std::string> ParseFileLinesToStrings(const std::vector<std::string>& fileLines);



#endif //MODDECLARE_TOKENIZER_HPP
