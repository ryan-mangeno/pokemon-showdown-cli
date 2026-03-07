#pragma once

#include <tuple>

#include "core/defines.h"
#include "core/logger.h"
#include "util/util.h"

namespace pkm::net {

    static std::string net_config_path() {
        auto pth = std::filesystem::path(ROOT_DIR) / "config" / "net_config.json";
        if ( !std::filesystem::exists(pth) ) {
            PK_ERROR("Net Config path doesn't exist! {0}", pth.string());
            return {};
        }
        return pth.string();
    }


    struct NetConfig : Config<NetConfig> {
        std::string ps_server_port;
        std::string ps_server_url;
        std::string ps_websocket_path;

        auto as_tuple_impl() {
            return std::tie(ps_server_port,
                     ps_server_url,
                     ps_websocket_path
            );
        }
        
        NetConfig() = default;
        ~NetConfig() = default;
    };

}

const std::string NET_CONFIG_PATH = pkm::net::net_config_path();
