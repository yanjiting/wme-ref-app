extern "C" {
#include "loudmax.h"
}
#include "lslimiter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Usage:
 - The simulation mode, give file path arguments to the tc-dagc-playback-ut:
 tc-dagc-playback-ut <input.raw> <output.raw> mode gain
 
 mode (optional): 
    0 - fixedgain (default)
    1 - loudmax

 gain (optional): default value is 1.0f


 Example:
 cd wme
 xcodebuild -project unittest/dolphin/bld/mac/tc-dagc-playback-ut/tc-dagc-playback-ut.xcodeproj -configuration Release clean && xcodebuild -project unittest/dolphin/bld/mac/tc-dagc-playback-ut/tc-dagc-playback-ut.xcodeproj -configuration Release && (cd unittest/dolphin/bld/mac/tc-dagc-playback-ut/build/Release && ./tc-dagc-playback-ut input.raw output.raw 0 8)
 */

const int framesize = 480;

enum mode
{
    FIXEDGAIN = 1,
    LOUDMAX = 2,
};

static bool findLengthOfFile(const char *filename, int * length)
{
    FILE * fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Could not open file %s\n", filename);
        return false;
    }

    *length = 0;
    float tmpBuf;
    while (fread(&tmpBuf, sizeof(float), 1, fp)) {
        (*length)++;
    }

    return true;
}

static bool readSimulationFile(char* argv[], float **input, int *numSamples)
{
    puts(" Read the following input file for simulation: ");
    printf(" Input: %s\n", argv[1]);

    if (!(findLengthOfFile(argv[1], numSamples)))
    	return false;

    *input = new float[*numSamples];

    FILE * fp = fopen(argv[1], "rb");
    if (!fp) {
        fprintf(stderr, " Could not open file with input data\n");
        return false;
    }
    size_t n = fread(*input, sizeof(float), *numSamples, fp);
    (void) n;
    fclose(fp);

    return true;
}

static bool writeBufferToFile(const char * filename, float * buf, int len)
{
    FILE * fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, " Could not open file for writing: %s\n", filename);
        return false;
    }
    size_t n = fwrite(buf, sizeof(float), len, fp);
    fclose(fp);

    printf(" %d samples written to %s\n", (int)n, filename);
    return true;
}

static void adjustlevel(float * buf, float gain)
{
    for (int i = 0; i < framesize; ++i)
        buf[i] *= gain;
}

static void run_test_dagc_playback(float * input, float * output, int numFrames, enum mode mode, float gain)
{
    const float clip_headroom = 0.86f;
    float threshold = 0.5f;
    float soft_threshold = 1 / (clip_headroom * threshold);

    struct loudmax * m_loudmax = loudmax_create(gain, framesize);

    struct LSLIMITER * m_lslimiter = lslimiter_create();
    lslimiter_init(m_lslimiter);
    
    for (int i = 0; i < numFrames; ++i) {
        if (mode == LOUDMAX)
            loudmax_process(m_loudmax, input);
        else
            adjustlevel(input, gain);

        lslimiter_process(m_lslimiter, &input, &output, &threshold, 1, soft_threshold);

        input += framesize;
        output += framesize;
    }
    loudmax_destroy(m_loudmax);
    lslimiter_destroy(m_lslimiter);
}

int main(int argc, char* argv[])
{
    float *input = nullptr;
    int numFrames = 0;
    int numSamples = 0;
    enum mode mode = FIXEDGAIN;
    float gain = 1.0f;

    puts("TC DAGC playback unittest");

    if (argc < 3 || argc > 5) {
        puts(" Wrong number of arguments - quit test");
        return 0;
    }

    if (strcmp(argv[3], "loudmax") == 0)
        mode = LOUDMAX;

    if (argc == 5)
        gain = (float)atof(argv[4]);

    if (!readSimulationFile(argv, &input, &numSamples))
        return 0;

    numFrames = numSamples / framesize;
    numSamples = numFrames * framesize;  //floor numSamples to closest framesize

    float *output = new float[numSamples];

    printf(" Run test with %d frames, mode: %s, gain: %3.1f\n",
           numFrames, (mode == LOUDMAX) ? "loudmax" : "fixedgain", gain);

    run_test_dagc_playback(input, output, numFrames, mode, gain);

    writeBufferToFile(argv[2], output, numSamples);

    delete input;
    delete output;
}
