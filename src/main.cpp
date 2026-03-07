#include "pkmpch.h"

#include <util/json_loader.h>
#include <core/logger.h>
#include <net/netconfig.h>
#include <net/wsclient.h>

int main() {

    pkm::Logger::init();
    pkm::net::NetConfig ncfg;
    pkm::JsonLoader::load(ncfg, NET_CONFIG_PATH.c_str());

    pkm::net::WsClient client(ncfg);
    if (client.connect()) {
        PK_INFO("Recieved:\n{0}", client.receive());
    }

    client.close();

    return 0;
}
