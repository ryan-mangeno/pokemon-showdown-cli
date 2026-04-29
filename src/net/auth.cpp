#include "auth.h"
#include "core/logger.h"

#include <curl/curl.h>
#include <nlohmann/json.hpp>

namespace pkm::net {

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
        size_t total_size = size * nmemb;
        userp->append((char*)contents, total_size);
        return total_size;
    }

    std::string request_assertion(const std::string& username, 
                                  const std::string& password, 
                                  const std::string& challstr) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            PK_ERROR("Failed to initialize CURL for authentication.");
            return "";
        }

        std::string response_buffer;
        std::string assertion = "";

        std::string post_fields = "act=login&name=" + username + 
                                  "&pass=" + password + 
                                  "&challstr=" + challstr;

        curl_easy_setopt(curl, CURLOPT_URL, "https://play.pokemonshowdown.com/action.php");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_fields.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_buffer);
        
        // follow redirects and handle HTTPS correctly
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        // make request
        CURLcode res = curl_easy_perform(curl);
        
        if (res == CURLE_OK) {
            // NOTE: Showdown prefixes successful JSON with a ']' character 
            // to prevent JSON hijacking. We must skip it
            if (!response_buffer.empty() && response_buffer[0] == ']') {
                try {
                    auto j = nlohmann::json::parse(response_buffer.substr(1));
                    
                    if (j.contains("assertion")) {
                        assertion = j["assertion"];
                        
                        // If the assertion starts with ';', it's an error message- ";invalid password"
                        if (!assertion.empty() && assertion[0] == ';') {
                            PK_ERROR("Login Server Error: {}", assertion.substr(1));
                            assertion = "";
                        }
                    } else if (j.contains("curuser") && j["curuser"]["loggedin"] == false) {
                        PK_ERROR("Login failed: Unauthorized");
                    }
                } catch (const std::exception& e) {
                    PK_ERROR("JSON Parse Error during Auth: {}", e.what());
                }
            }
        } else {
            PK_ERROR("HTTPS POST to login server failed: {}", curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
        return assertion;
    }

}