#include <iostream>
#include <print>
#include <format>
#include <unordered_map>

#include "File.hpp"
#include "UrlRequest.hpp"
#include "Modrinth.hpp"
#include "ANSIColors.hpp"
#include "ModDeclare/ModRequest.hpp"
#include "ModDeclare/Tokenizer.hpp"




int main(int argc, char** argv) {


    // testing new modrinth pipeline
/*
    if (auto j = Modrinth::getModrinthProjectJson("Sodium", "jellysquid3", "", "26.2", "fabric"); j.has_value()) {
        std::print("Passed\n");
    }
    if (auto j = Modrinth::getModrinthProjectJson("Sodium", "", "CaffeineMC", "26.2", "fabric", "A0IzDzaH"); j.has_value()) {
        std::print("Passed\n");
    }
    if (auto j = Modrinth::getModrinthProjectJson("Sodium", "jellysquid3", "", "26.2", "fabric", "_"); !j.has_value()) {
        std::print("Passed\n");
    }
*/

    // TODO: Add Logging Flags: Verbose, Otherwise Only Print Useful Info
    // TODO: Allow Other Project Types (Such As Resource Packs And Shaders)
    // TODO: Allow User To Specify Where They Want Resources To Be Downloaded To (Relative To The File Location)
    // TODO: Check Download Directory For Matching File Names To Avoid Redownloading -- It Would Be Nice To Know What Mods Can Be Deleted Automatically, But That Is Not Something I Care About Now


    ModDeclareContext context;
    ModDeclareContextResult contextResult = context.parseCommandLineArgs(argc, argv);

    if (contextResult == ModDeclareContextResult::HelpMenu) {
        // TODO: Handle Help Menu Instead Of Downloading Anything, Then Return
        return 0;
    }

    std::vector<std::string> tokens = ParseFileLinesToStrings(context.getParsedFileLines());

    auto [modRequestResult, requests] = context.parseModRequestsFromTokensList(tokens);
    if (modRequestResult != ModRequestParseResult::Success) {
        std::print("{} Error Occurred Parsing Mod Requests From Tokens List: {}{}\n", Colors::RED, (int)modRequestResult, Colors::RESET);
    }

    std::unordered_map<std::string, std::pair<ModRequest, ModDeclareContext::ModrinthProjectDownloadData>> downloadsList = context.getDownloadListFromRequests(requests);

    context.filterDownloadsListFromDownloadMetaFile(downloadsList);

    std::print("{}\n{}\n\n", Colors::ColorString(Colors::YELLOW, "Warnings: "+std::to_string(context.getWarnings())+" | Max Allowed: "+std::to_string(context.getMaxWarnings())),
                             Colors::ColorString(Colors::RED, "Errors: "+std::to_string(context.getErrors())+" | Max Allowed: "+std::to_string(context.getMaxErrors())));

    if (context.getWarnings() > context.getMaxWarnings()) {
        std::print("{}\n", Colors::ColorString(Colors::RED, "Exceeded Maximum Allowed Warnings; Exiting"));
        return -1;
    }
    if (context.getErrors() > context.getMaxErrors()) {
        std::print("{}\n", Colors::ColorString(Colors::RED, "Exceeded Maximum Allowed Errors; Exiting"));
        return -1;
    }

    context.downloadFromDownloadsList(downloadsList);

    return 0;
}
