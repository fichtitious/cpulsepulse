#include <pulse/simple.h>

using namespace std;

#define PULSEAUDIO_SAMPLE_TYPE float
#define PULSEAUDIO_SAMPLE_TYPE_CODE PA_SAMPLE_FLOAT32LE

class Pulser {

  private:

    // pulseaudio sampling
    pa_simple *_pulseAudioClient;
    int _pulseAudioError;
    int _sampleSize;
    PULSEAUDIO_SAMPLE_TYPE *_pulseAudioSamples;

    // peak tracking
    PULSEAUDIO_SAMPLE_TYPE *_history;
    int _ringIdx;
    PULSEAUDIO_SAMPLE_TYPE _peakness;

    string getRunningSink();

  public:

    Pulser();
    ~Pulser();
    void pulse();

    bool isPeak;
    bool isIncreasing;

};
