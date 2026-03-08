#include "pkmpch.h"

#include "protocol/psclient.h"

int main() {

    pkm::Logger::init();
    pkm::protocol::PsClient client;
    client.init();
    client.run();
    client.shutdown();

    return 0;
}
