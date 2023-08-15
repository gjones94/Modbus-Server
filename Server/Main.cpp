#include "BaseServer.h"

int main() {
    BaseServer<char*> server;
    server.Start();
    return 1;
}
