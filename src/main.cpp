#include "pkmpch.h"

#include "protocol/psclient.h"
#include "app/psapp.h"

int main() {

    pkm::Logger::init();
    pkm::PsApp app;
    app.init();
    app.run();

    return 0;
}
