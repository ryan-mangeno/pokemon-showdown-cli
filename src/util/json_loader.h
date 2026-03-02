#include <nlohmann/json.hpp>
#include <fstream>
#include <string>
#include <iostream>
#include <stdexcept>
#include <string>

#include "util.h"

namespace pkm {

// this updates if changes are made to config/config.json in repo root
    struct Config {
        std::string ps_server_port;
        std::string ps_server_url;
        std::string ps_websocket_path; 
        
        Config() = default;
    };

    class ConfigLoader {
    public:
        static Config load(const std::string& path = ROOT_DIR) {
            const std::filesystem::path cfg_path = config_path(); 

            std::ifstream file(cfg_path);
            if (!file.is_open()) {
                // TODO: Log
                return Config();
            }
    
            nlohmann::json j;
            file >> j;

            Config cfg;
            cfg.ps_server_port    = j.at("ps_server_port").get<std::string>();
            cfg.ps_server_url     = j.at("ps_server_url").get<std::string>();
            cfg.ps_websocket_path = j.at("ps_websocket_path").get<std::string>();
            
            return cfg;
        }
    };

}
