#include "gtest/gtest.h"
#include "share_util.h"
//#define ENABLE_SHARE_DOWNSAMPLE_UNITTEST
#if defined(ENABLE_SHARE_DOWNSAMPLE_UNITTEST)
/*nQualityLevel
    1: low, 2: media, 3: High */
const int times = 10;

struct PngFileItem {
    string strFile;
    WBXSize sizeFileResolution;
};
//be able to get *.png from https://cisco.box.com/s/buoybyp83t89yendqo9nagcgnldf28p0
//#define TEST_ALL_SOURCE
#if defined(TEST_ALL_SOURCE)
std::vector<PngFileItem> vFiles = {
    {"text-2880-1800.png",{2880, 1800}},
    {"image-2880-1800.png",{2880, 1800}},
    {"image-5120-2880.png",{5120, 2800}}
};
#else
std::vector<PngFileItem> vFiles = {
    {"text-2880-1800.png",{2880, 1800}},
    {"image-2880-1800.png",{2880, 1800}}
};
#endif

std::vector<int> vSizes = { 360, 720, 1080 };
#ifdef WIN32
std::vector<int> vQualities = { 1, 2 }; //windows just has 2 level
#else
std::vector<int> vQualities = { 1, 2, 3 };
#endif

extern int SystemDownSamplePerformanceTest(const char *inputFile, int nWidth, int nHeight, int nQualityLevel, int nTimes);
extern int SystemDownSampleQualityTest(const char *inputFile, int nWidth, int nHeight, int nQualityLevel);

void outputPerformanceReport(const char *inputFile,int nSourceWidth, int nSourceHeight, int nDestWidth, int nDestHeight, int nUsageTime, int nQualityLevel)
{
    std::stringstream stream;
    stream << "downsample usageTime=" << nUsageTime
           << " " <<inputFile
           << " from " << nSourceWidth << "*" << nSourceHeight
           << " to " << nDestWidth << "*" << nDestHeight
           << " Quality=" << nQualityLevel << std::endl;
    std::cout << stream.str();
}
/*
 Mac System CG downsample performance
 =======================================================================================================
 downsample usageTime=19000 text-2880-1800.png from 2880*1800 to 2880*1800 Quality=1
 downsample usageTime=37000 text-2880-1800.png from 2880*1800 to 1728*1080 Quality=1
 downsample usageTime=27000 text-2880-1800.png from 2880*1800 to 1152*720 Quality=1
 downsample usageTime=15000 text-2880-1800.png from 2880*1800 to 576*360 Quality=1
 downsample usageTime=18000 text-2880-1800.png from 2880*1800 to 2880*1800 Quality=2
 downsample usageTime=19000 text-2880-1800.png from 2880*1800 to 1728*1080 Quality=2
 downsample usageTime=16000 text-2880-1800.png from 2880*1800 to 1152*720 Quality=2
 downsample usageTime=13000 text-2880-1800.png from 2880*1800 to 576*360 Quality=2
 downsample usageTime=18000 text-2880-1800.png from 2880*1800 to 2880*1800 Quality=3
 downsample usageTime=23000 text-2880-1800.png from 2880*1800 to 1728*1080 Quality=3
 downsample usageTime=19000 text-2880-1800.png from 2880*1800 to 1152*720 Quality=3
 downsample usageTime=16000 text-2880-1800.png from 2880*1800 to 576*360 Quality=3
 downsample usageTime=24000 image-2880-1800.png from 2880*1800 to 2880*1800 Quality=1
 downsample usageTime=43000 image-2880-1800.png from 2880*1800 to 1728*1080 Quality=1
 downsample usageTime=32000 image-2880-1800.png from 2880*1800 to 1152*720 Quality=1
 downsample usageTime=21000 image-2880-1800.png from 2880*1800 to 576*360 Quality=1
 downsample usageTime=24000 image-2880-1800.png from 2880*1800 to 2880*1800 Quality=2
 downsample usageTime=23000 image-2880-1800.png from 2880*1800 to 1728*1080 Quality=2
 downsample usageTime=20000 image-2880-1800.png from 2880*1800 to 1152*720 Quality=2
 downsample usageTime=17000 image-2880-1800.png from 2880*1800 to 576*360 Quality=2
 downsample usageTime=25000 image-2880-1800.png from 2880*1800 to 2880*1800 Quality=3
 downsample usageTime=27000 image-2880-1800.png from 2880*1800 to 1728*1080 Quality=3
 downsample usageTime=22000 image-2880-1800.png from 2880*1800 to 1152*720 Quality=3
 downsample usageTime=20000 image-2880-1800.png from 2880*1800 to 576*360 Quality=3


 Windows System GDI downsample performance
 =======================================================================================================
 downsample usageTime=3200 text-2880-1800.png from 2880*1800 to 2880*1800 Quality=1
 downsample usageTime=6200 text-2880-1800.png from 2880*1800 to 1728*1080 Quality=1
 downsample usageTime=6300 text-2880-1800.png from 2880*1800 to 1152*720 Quality=1
 downsample usageTime=6300 text-2880-1800.png from 2880*1800 to 576*360 Quality=1
 downsample usageTime=25000 text-2880-1800.png from 2880*1800 to 2880*1800 Quality=2
 downsample usageTime=42200 text-2880-1800.png from 2880*1800 to 1728*1080 Quality=2
 downsample usageTime=31200 text-2880-1800.png from 2880*1800 to 1152*720 Quality=2
 downsample usageTime=20400 text-2880-1800.png from 2880*1800 to 576*360 Quality=2
 downsample usageTime=4700 image-2880-1800.png from 2880*1800 to 2880*1800 Quality=1
 downsample usageTime=7800 image-2880-1800.png from 2880*1800 to 1728*1080 Quality=1
 downsample usageTime=7900 image-2880-1800.png from 2880*1800 to 1152*720 Quality=1
 downsample usageTime=7800 image-2880-1800.png from 2880*1800 to 576*360 Quality=1
 downsample usageTime=26500 image-2880-1800.png from 2880*1800 to 2880*1800 Quality=2
 downsample usageTime=37500 image-2880-1800.png from 2880*1800 to 1728*1080 Quality=2
 downsample usageTime=26600 image-2880-1800.png from 2880*1800 to 1152*720 Quality=2
 downsample usageTime=18700 image-2880-1800.png from 2880*1800 to 576*360 Quality=2
 */
TEST(SystemCaptureDownSample, Performance)
{
    int nWidth = 0, nHeight = 0;
    for (const PngFileItem &onePngFileItem : vFiles) {
        string file = onePngFileItem.strFile;
        WBXSize vResolution = onePngFileItem.sizeFileResolution;
        float fRation = 1.0 * vResolution.cx / vResolution.cy;

        for (const int &quality : vQualities) {
            vSizes.push_back(vResolution.cy);
            for (const int &height : vSizes) {
                nHeight = height;
                nWidth = fRation*nHeight;
                int nTimeUsage = SystemDownSamplePerformanceTest(file.c_str(), nWidth, nHeight, quality,times)/1000;
                outputPerformanceReport(file.c_str(),vResolution.cx, vResolution.cy, nWidth, nHeight, nTimeUsage, quality);
            }
            vSizes.erase(vSizes.end()-1);
        }
    }
}


TEST(SystemCaptureDownSample, Quality)
{
    int nWidth = 0, nHeight = 0;
    for (const PngFileItem &onePngFileItem : vFiles) {
        string file = onePngFileItem.strFile;
        WBXSize vResolution = onePngFileItem.sizeFileResolution;
        float fRation = 1.0 * vResolution.cx / vResolution.cy;

        for (const int &quality : vQualities) {
            vSizes.push_back(vResolution.cy);
            for (const int &height : vSizes) {
                nHeight = height;
                nWidth = fRation*nHeight;
                int nTimeUsage = SystemDownSampleQualityTest(file.c_str(), nWidth, nHeight,quality);
            }
            vSizes.erase(vSizes.end()-1);
        }
    }
}

TEST(OpenH264CaptureDownSample, Performance)
{

}

TEST(OpenH264CaptureDownSample, Quality)
{

}

#endif ENABLE_SHARE_DOWNSAMPLE_UNITTEST