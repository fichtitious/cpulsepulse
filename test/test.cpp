#include <signal.h>
#include <iostream>
#include "../src/cpulsepulse.h"

using namespace std;

Pulser *pulser;

void end(int signal) {

    cout << "done" << endl;
    delete pulser;

}

int main() {

    pulser = new Pulser();

    signal(SIGINT, end);

    while (1) {
        pulser->pulse();
        cout << pulser->isPeak << (pulser->isIncreasing ? " +" : "") << endl;
    }

    return 0;

}
