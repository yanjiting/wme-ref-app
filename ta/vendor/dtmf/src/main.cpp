/** Author:       Plyashkevich Viatcheslav <plyashkevich@yandex.ru> 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License 
 * All rights reserved. 
 */

#include <stdio.h>
#include <string>
#include "DtmfDetector.hpp"
#include "DtmfGenerator.hpp"
#include <assert.h>

char* getCmdOption(char ** begin, char ** end, const std::string & option) {
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end) {
        return *itr;
    }
    return NULL;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option) {
    return std::find(begin, end, option) != end;
}

int main(int argc, char** argv)
{
    const char *dialButtons = getCmdOption(argv, argv + argc, "-g");
    const char *file_name = getCmdOption(argv, argv + argc, "-f");
    FILE *fp = NULL;
    const unsigned int FRAME_SIZE = 160;

    if (file_name == NULL) {
        printf("Usage: dtmf -f file_name.pcm");
        return -2;
    }
    if (dialButtons != NULL) {
        fp = fopen(file_name, "wb+");
        assert(fp != NULL);
        DtmfGenerator dtmfGenerator(FRAME_SIZE, 500, 50);
        dtmfGenerator.dtmfGeneratorReset();
        dtmfGenerator.transmitNewDialButtonsArray((char*)dialButtons, strlen(dialButtons));
        INT16 samples[FRAME_SIZE];
        while (!dtmfGenerator.getReadyFlag())
        {
            dtmfGenerator.dtmfGenerating(samples);
            fwrite(samples, sizeof(INT16), FRAME_SIZE, fp);
        }
    }
    else {
        fp = fopen(file_name, "rb");
        if (fp == NULL) {
            printf("file not exist: %s", file_name);
            return 2;
        }
        INT16 *samples = new INT16[1024 * 1024];
        memset(samples, 0, sizeof(INT16) * 1024 * 1024);
        size_t nRead = fread(samples, sizeof(INT16), 1024 * 1024, fp);
        DtmfDetector dtmfDetector(nRead);
        dtmfDetector.zerosIndexDialButton();
        dtmfDetector.dtmfDetecting(samples); // Detecting from 16 bit's little-endian's pcm samples array

        //printf("Number of detecting buttons %d\nThere are: ", dtmfDetector.getIndexDialButtons());
        for (int ii = 0; ii < dtmfDetector.getIndexDialButtons(); ++ii)
        {
            printf("%c", dtmfDetector.getDialButtonsArray()[ii]);
        }
        //printf("\n");
        delete[] samples;
        //getchar();
    }
    if (fp != NULL)
        fclose(fp);

    return 0;
}
