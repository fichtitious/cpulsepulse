#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <pulse/error.h>
#include "cpulsepulse.h"

using namespace std;

const unsigned int NUM_AUDIO_FRAMES = 1024;
const uint8_t NUM_CHANNELS = 2;
const uint32_t SAMPLE_RATE = 44100;
const int HISTORY_LENGTH = 32;

Pulser::Pulser() {

    // find the running pulseaudio sink
    string runningSink = getRunningSink();
    const char *audioDevice = runningSink.c_str();

    // initialize the sample buffer
    _pulseAudioSamples = new PULSEAUDIO_SAMPLE_TYPE[NUM_AUDIO_FRAMES];
    _sampleSize = sizeof(PULSEAUDIO_SAMPLE_TYPE) * NUM_AUDIO_FRAMES;

    // make a pulseaudio connection
    cout << "cpulsepulse is connecting to pulseaudio sink " << audioDevice << "...";
    const pa_sample_spec paSampleSpec = { PULSEAUDIO_SAMPLE_TYPE_CODE, SAMPLE_RATE, NUM_CHANNELS };
    if (!(_pulseAudioClient = pa_simple_new( NULL, "cpulsepulse", PA_STREAM_RECORD, audioDevice, "cpulsepulse",
                                             &paSampleSpec, NULL, NULL, &_pulseAudioError ))) {
        cout << "\ncpulsepulse couldn't connect to pulseaudio: " << pa_strerror(_pulseAudioError) << endl;
        exit(1);
    } else {
        cout << "connected." << endl;
    }

    // set up peak tracking
    isPeak = false;
    isIncreasing = false;
    _history = new PULSEAUDIO_SAMPLE_TYPE[HISTORY_LENGTH];
    _ringIdx = 0;
    _peakness = 0;
    int i;
    for (i = 0; i < HISTORY_LENGTH; i++) {
        _history[i] = 0;
    }

}

Pulser::~Pulser() {

    pa_simple_free(_pulseAudioClient);
    delete _pulseAudioSamples;
    delete _history;
    cout << "cpulsepulse successfully closed its pulseaudio connection." << endl;

}

void Pulser::pulse() {

    // read the latest data from pulseaudio into _pulseAudioSamples
    if (pa_simple_read( _pulseAudioClient, _pulseAudioSamples, _sampleSize, &_pulseAudioError ) < 0) {
        cerr << "cpulsepulse error reading from pulseaudio: " << pa_strerror(_pulseAudioError) << endl;
        return;
    }

    // sum up _pulseAudioSamples
    PULSEAUDIO_SAMPLE_TYPE latest = 0;
    int i;
    for (i = 0; i < NUM_AUDIO_FRAMES; i++) {
        latest += _pulseAudioSamples[i];
    }

    // mean squares
    PULSEAUDIO_SAMPLE_TYPE localAverage = 0;
    for (i = 0; i < HISTORY_LENGTH; i++) {
        localAverage += pow(_history[i], 2);
    }
    localAverage /= HISTORY_LENGTH;

    // mean squared differences from the local average
    PULSEAUDIO_SAMPLE_TYPE localVariance = 0;
    for (i = 0; i < HISTORY_LENGTH; i++) {
        localVariance += pow(localAverage - _history[i], 2);
    }
    localVariance /= HISTORY_LENGTH;

    // temp save last peakness
    PULSEAUDIO_SAMPLE_TYPE lastPeakness = _peakness;

    // calculate new peakness
    PULSEAUDIO_SAMPLE_TYPE valueToBeat = localAverage * (-0.00008 * localVariance + 32.0);
    _peakness = latest - valueToBeat;

    // save values for the derived state variables
    isPeak = _peakness > 0;
    isIncreasing = _peakness - lastPeakness > 312.0;

    // push the latest value into the ring buffer
    // and cycle the ring pointer if necessary
    _history[_ringIdx] = latest;
    if ( ++_ringIdx == HISTORY_LENGTH ) {
        _ringIdx = 0;
    }

}

string Pulser::getRunningSink() {

    FILE *pipe = popen( "pacmd list-sinks | grep state", "r" );

    char buffer[128];
    short i = 0;
    short deviceIdx = -1;
    string line;
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != NULL) {
            line = buffer;
            if (line.find("RUNNING") != string::npos) {
                deviceIdx = i;
                break;
            } else {
                i++;
            }
        }
    }

    pclose(pipe);

    if (deviceIdx == -1) {
        cerr << "cpulsepulse could not find a running pulseaudio sink device (ran pacmd list-sinks)" << endl;
        exit(1);
    } else {
        stringstream stream;
        stream << deviceIdx;
        return stream.str();
    }

}
