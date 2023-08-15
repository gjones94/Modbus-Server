#include "ModbusSlave.h"

int main() {
    ModbusSlave slave(502);
    slave.Start();
    return 1;
}
