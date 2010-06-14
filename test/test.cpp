#include <signal.h>
#include <iostream>
#include "../src/cpulsepulse.h"

using namespace std;

PulseTracker *tracker;
Pulser *pulser;

void end(int signal) {

    cout << "done" << endl;
    delete pulser;
    delete tracker;

}

int main() {

    tracker = new PulseTracker();
    pulser = new Pulser(tracker);

    signal(SIGINT, end);

    while (1) {
        pulser->pulse();
        cout << tracker->isPeak << (tracker->isIncreasing ? " +" : "") << endl;
    }

    return 0;

}
