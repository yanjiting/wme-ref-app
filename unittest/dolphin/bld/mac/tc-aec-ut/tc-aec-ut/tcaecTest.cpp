#include "tcaec.h"
#include "audtest.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Usage:
 - The default mode is for running a test with PASS / FAIL criteria.
 Example:
 cd wme

 xcodebuild -project unittest/dolphin/bld/mac/tc-aec-ut/tc-aec-ut.xcodeproj -configuration Release clean && xcodebuild -project unittest/dolphin/bld/mac/tc-aec-ut/tc-aec-ut.xcodeproj -configuration Release && (cd unittest/dolphin/bld/mac/tc-aec-ut/build/Release && ./tc-aec-ut)


 - The simulation mode, give file path arguments to the tc-aec-ut:
 tc-aec-ut <mic.raw> <ls.raw> <aec_out.raw>

 Example:
 xcodebuild -project unittest/dolphin/bld/mac/tc-aec-ut/tc-aec-ut.xcodeproj -configuration Release clean && xcodebuild -project unittest/dolphin/bld/mac/tc-aec-ut/tc-aec-ut.xcodeproj -configuration Release && (cd unittest/dolphin/bld/mac/tc-aec-ut/build/Release && ./tc-aec-ut mic.raw ls.raw out.raw)
 */


// Set the following to 1 when making new reference data:
static int writeoutput = 0;
const int framesize = 480;

struct TESTSETTINGS
{
    int numSamples;
    int numFrames;
    float epsilon;
    int maxlag;
};

static int readDefaultDataFromFiles(
    float ** micBuf,
    float ** lsBuf,
    float ** outBufReference,
    float ** outBuf,
    int numSamples)
{
    puts("Read default input files");

    *micBuf = new float[numSamples];
    *lsBuf = new float[numSamples];
    *outBufReference = new float[numSamples];
    *outBuf = new float[numSamples];

    FILE * fp = fopen("mic_mono.dat", "rb");
    if (!fp) {
        fprintf(stderr, "Could not open file with mic data\n");
        return 1;
    }
    size_t n = fread(*micBuf, sizeof(float), numSamples, fp);
    assert(n == numSamples);
    fclose(fp);

    fp = fopen("ls_mono.dat", "rb");
    if (!fp) {
        fprintf(stderr, "Could not open file with loudspeaker reference data\n");
        return 1;
    }
    n = fread(*lsBuf, sizeof(float), numSamples, fp);
    assert(n == numSamples);
    fclose(fp);

    fp = fopen("out_mono_wme.dat", "rb");
    if (!fp) {
        fprintf(stderr, "Could not open file with output reference data\n");
        return 1;
    }
    n = fread(*outBufReference, sizeof(float), numSamples, fp);
    assert(n == numSamples);
    fclose(fp);

    return 0;
}

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

static bool readSimulationFiles(char* argv[], float **micBuf, float **lsBuf, float **outBuf, int *numSamples)
{
    puts("Read the following input files for simulation: ");
    printf("mic: %s\n", argv[1]);
    printf("ls: %s\n", argv[2]);

    int micLength = 0, lsLength = 0;
    if (!(findLengthOfFile(argv[1], &micLength) && findLengthOfFile(argv[2], &lsLength)))
    	return false;

    *numSamples = micLength > lsLength ? lsLength : micLength;
    //printf("numSamples %d\n", *numSamples);

    *micBuf = new float[*numSamples];
    *lsBuf = new float[*numSamples];
    *outBuf = new float[*numSamples];

    FILE * fp = fopen(argv[1], "rb");
    if (!fp) {
    	fprintf(stderr, "Could not open file with mic data\n");
    	return 1;
    }
    size_t n = fread(*micBuf, sizeof(float), *numSamples, fp);
    fclose(fp);


    fp = fopen(argv[2], "rb");
    if (!fp) {
    	fprintf(stderr, "Could not open file with loudspeaker reference data\n");
    	return 1;
    }
    n = fread(*lsBuf, sizeof(float), *numSamples, fp);
    fclose(fp);

    return true;
}

static bool writeBufferToFile(const char * filename, float * buf, int len)
{
    FILE * fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Could not open file for writing: %s\n", filename);
        return false;
    }
    size_t n = fwrite(buf, sizeof(float), len, fp);
    fclose(fp);

    printf("%d samples written to %s\n", (int)n, filename);
    return true;
}

static bool writeIntBufferToFile(const char * filename, int * buf, int len)
{
    FILE * fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Could not open file for writing: %s\n", filename);
        return false;
    }
    size_t n = fwrite(buf, sizeof(int), len, fp);
    fclose(fp);

    printf("%d samples written to %s\n", (int)n, filename);
    return true;
}

static void set_testsettings(
    TESTSETTINGS *testsettings,
    int numSamples,
    int numFrames,
    float epsilon,
    int maxlag)
{
    testsettings->numSamples = numSamples;
    testsettings->numFrames = numFrames;
    testsettings->epsilon = epsilon;
    testsettings->maxlag = maxlag;
}

static void downscale(float * buf, size_t buflen)
{
    for (size_t i = 0; i < buflen; ++i)
        buf[i] /= INT16_MAX;
}

static void upscale(float * buf, size_t buflen)
{
    for (size_t i = 0; i < buflen; ++i)
        buf[i] *= INT16_MAX;
}

static int run_test_ec(
    float * micBuf,
    float * lsBuf,
    const float * outRefBuf,
    float * outBuf,
    const TESTSETTINGS * testsettings)
{
    bool differ = false;
    int delayEstimate = 0;
    float * erleBuf = nullptr;
    int * delayBuf = nullptr;
    float * aerlInverse = nullptr;

    TCAEC aec(testsettings->maxlag);

    aec.SetNlpSpeed(0.25f);
    aec.SetNlpHigh(false);

    if (writeoutput) {
        erleBuf = new float[testsettings->numFrames];
        delayBuf = new int[testsettings->numFrames];
        aerlInverse = new float[testsettings->numFrames];
        aec.ComputeErle(true);
        aec.SetBetaErle(0.0f);
    }

    for (int i = 0; i < testsettings->numFrames; ++i)
    {
        upscale(micBuf, framesize);
        upscale(lsBuf, framesize);

        aec.SetPlaybackVolume(1.0f);
        aec.process(micBuf, lsBuf, outBuf, framesize, framesize, 0);

        downscale(outBuf, framesize);

        if (aec.GetDelayEstimate() != delayEstimate) {
            printf("New echo path delay estimated to be %d frames in frame %d\n", aec.GetDelayEstimate(), i);
            delayEstimate = aec.GetDelayEstimate();
        }

        if (outRefBuf) {
            differ |= !audtest_floatsAbsEq(outBuf, outRefBuf, framesize, testsettings->epsilon);
            outRefBuf += framesize;
        }

        if (differ && !writeoutput)
            break;

        if (writeoutput) {
            erleBuf[i] = aec.GetErle();
            delayBuf[i] = aec.GetInstantaneousDelayEstimate();
            aerlInverse[i] = aec.GetAerlInverse();
        }

        micBuf += framesize;
        lsBuf += framesize;
        outBuf += framesize;
    }

    if (differ)
        fprintf(stderr, "Computed network output differs from reference data.\n");

    if (writeoutput) {
        writeBufferToFile("erle.dat", erleBuf, testsettings->numFrames);
        writeIntBufferToFile("delay.dat", delayBuf, testsettings->numFrames);
        writeBufferToFile("aerlInverse.dat", aerlInverse, testsettings->numFrames);
        delete erleBuf;
        delete delayBuf;
        delete aerlInverse;
    }

    return differ;
}

int main(int argc, char* argv[])
{
    const int maxlag = 50;
    TESTSETTINGS testsettings;
    int ret;

    const float epsilon = 5e-4f;
    float *micBuf = nullptr;
    float *lsBuf = nullptr;
    float *outBufReference = nullptr;
    float *outBuf = nullptr;
    int numFrames = 0;
    int numSamples = 0;

    puts("TC AEC unittest");

    if (argc > 1 && argc != 4) {
        puts("Wrong number of arguments - quit test");
        return 0;
    } else if (argc == 4) {
        if (!readSimulationFiles(argv, &micBuf, &lsBuf, &outBuf, &numSamples))
            return 0;

        numFrames = numSamples / framesize;
        numSamples = numFrames * framesize;  //floor numSamples to closest framesize
        //printf("Set numFrames %d, numSamples %d\n", numFrames, numSamples);
        writeoutput = 1;
        set_testsettings(&testsettings, numSamples, numFrames, epsilon, maxlag);
    } else {
        numFrames  = 1145;
        numSamples = numFrames * framesize;
        set_testsettings(&testsettings, numSamples, numFrames, epsilon, maxlag);
        ret = readDefaultDataFromFiles(&micBuf, &lsBuf, &outBufReference, &outBuf, numSamples);
        assert(ret == 0);
    }

    ret = run_test_ec(micBuf, lsBuf, outBufReference, outBuf, &testsettings);

    if (writeoutput) {
        if (argc == 4)
            writeBufferToFile(argv[3], outBuf, numSamples);
        else
            writeBufferToFile("out_mono_new.dat", outBuf, numSamples);
    } else {
        printf("TC AEC unittest: %s\n", ret == 0 ? "PASSED" : "FAILED");
        delete outBufReference;
    }

    delete micBuf;
    delete lsBuf;
    delete outBuf;
    return ret;
}
