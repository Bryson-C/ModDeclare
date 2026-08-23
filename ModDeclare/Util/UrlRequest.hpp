//
// Created by Owner on 6/23/2026.
//

#ifndef COGITOPLATFORM_URLREQUEST_HPP
#define COGITOPLATFORM_URLREQUEST_HPP

#include <iostream>
#include <regex>

#include "cpr/cpr.h"
#include "nlohmann/json.hpp"

#include "File.hpp"
#include "ANSIColors.hpp"

namespace {
    size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
        size_t written = fwrite(ptr, size, nmemb, stream);
        return written;
    }
    int progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
        // It's here you will write the code for the progress message or bar
        // TODO: add progress bar
        return 0;
    }

    void download(const std::string& url, const std::string& path, const std::string& filename) {
        CURL *curl;
        FILE *fp;
        CURLcode res;
        std::filesystem::create_directories(path);
        curl = curl_easy_init();
        if (curl) {
            fp = fopen((path+"/"+filename).c_str(), "wb");
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
            // Internal CURL progressmeter must be disabled if we provide our own callback
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, FALSE);
            // Install the callback function
            curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
            res = curl_easy_perform(curl);
            /* always cleanup */
            curl_easy_cleanup(curl);
            fclose(fp);
        }
    }
}


inline nlohmann::json getJsonFromRequestUrl(const std::string& url) {
    try {
        cpr::Response r = cpr::Get(cpr::Url{url});
        if (r.status_code > 299) {
            std::print("{} API Error: {}{}\n", Colors::RED, r.reason, Colors::RESET);
        }
        try {
            auto json = nlohmann::json::parse(r.text);
            return json;
        } catch (nlohmann::json::parse_error& e) {
            std::print("Failed Parsing Json: {} {}\n", e.what(), e.byte);
        }

    } catch (std::exception& e) {
        std::print("CPR (cUrl) Error: {}\n", e.what());
    }
    return "";
}

inline void downloadFileFromUrl(const std::string& url, const std::string& folder, const std::string& filename) {
    download(url, folder, filename);
}

inline std::string cleanStringForUrl(const std::string& str) {
    std::string string = std::regex_replace(str, std::regex("\""), "%22");
    return std::regex_replace(string, std::regex(" "), "%20");
}

#endif //COGITOPLATFORM_URLREQUEST_HPP
