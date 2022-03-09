#include "Http/HttpServer.hpp"
#include "Http/HttpHandle.hpp"
#include "service/timer/timer.hpp"
#include <unistd.h>

int main(int argc, char const* argv[]) {
    HttpServer *httpServer = new HttpServer();
    httpHandleInit();
    while (1) {
        // usleep(LIBEVENT_LOOP_INTERVAL);
        process_timer_events();
        httpServer->loop();
    }

    return 0;
}
