#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>
#include <mach-o/dyld.h>
#include <sstream>
#include "timer.h"

/*nQualityLevel
 1: low, 2: media, 3: High */
int SystemDownSample(const char *inputFile,int nWidth,int nHeight,int nQualityLevel, bool bOutPutFile,int nTimes)
{
    int nRet = -1;
    CGDataProviderRef dataProvider = NULL;
    CGImageRef image = NULL;
    CGColorSpaceRef colorSpace = NULL;
    UInt8 *pBmpContextData = NULL;
    CGContextRef bmpContext = NULL;

    std::string strInputPath = inputFile;
    if (strInputPath.find('/') == std::string::npos) {
        char szPath[MAXPATHLEN] = {0};
        uint32_t size = MAXPATHLEN;
        if (_NSGetExecutablePath(szPath, &size) == 0) {
            std::string strNewInputPath = szPath;
            size_t found = strNewInputPath.rfind('/');
            if (found != std::string::npos) {
                strNewInputPath.replace(found, std::string::npos, "/");
            }
            strNewInputPath += strInputPath;
            strInputPath = strNewInputPath;
        }
    }

    do {
        //load image from file
        dataProvider = CGDataProviderCreateWithFilename(strInputPath.c_str());
        if (dataProvider==NULL) { break; }

        image = CGImageCreateWithPNGDataProvider(dataProvider, NULL, NO, kCGRenderingIntentDefault);
        if (image==NULL) { break; }

        int nImageWidth = CGImageGetWidth(image);
        int nImageHeight= CGImageGetHeight(image);
        //init CG context
        colorSpace = CGColorSpaceCreateDeviceRGB();
        if (colorSpace==NULL) { break; }
        pBmpContextData = new UInt8[nWidth * nHeight * 4];
        if (pBmpContextData==NULL) { break; }
        bmpContext = CGBitmapContextCreate(pBmpContextData,
                                           nWidth, nHeight,
                                           8, nWidth*4, colorSpace, kCGImageAlphaNoneSkipLast);
        if (bmpContext==NULL) { break; }

        //downsample and get raw data
        //kCGInterpolationNone = 1, /* Never interpolate. */
        //kCGInterpolationLow = 2,  /* Low quality, fast interpolation. */
        //kCGInterpolationMedium = 4,   /* Medium quality, slower than kCGInterpolationLow. */
        //kCGInterpolationHigh = 3  /* Highest quality, slower than
        switch (nQualityLevel) {
            case 3:
                CGContextSetInterpolationQuality(bmpContext,kCGInterpolationHigh);
                break;
            case 2:
                CGContextSetInterpolationQuality(bmpContext,kCGInterpolationMedium);
                break;
            case 1:
                CGContextSetInterpolationQuality(bmpContext,kCGInterpolationLow);
                break;
            default:
                CGContextSetInterpolationQuality(bmpContext,kCGInterpolationNone);
        }

        int nStart = ticker::now();
        for (int i=0; i<nTimes; i++) {

            CGContextSetRGBFillColor(bmpContext,0,0.62,0.62,1);
            CGRect fillRect = CGRectMake(0,0,nWidth,nHeight);
            CGContextFillRect(bmpContext,fillRect);

            CGRect imgRect = CGRectMake(0,0,nWidth,nHeight);


            CGContextDrawImage(bmpContext,imgRect,image);

        }
        nRet = ticker::now() - nStart;

        //
        if (bOutPutFile) {
            std::stringstream streamPngFilePath;
            streamPngFilePath << strInputPath << "_" << nWidth << "_" << nHeight << "_" << nQualityLevel << ".png";
            CGImageRef imgOutput = CGBitmapContextCreateImage(bmpContext);
            if (imgOutput==NULL) {
                break;
            }

            NSBitmapImageRep *bitmap = [[NSBitmapImageRep alloc] initWithCGImage:imgOutput];
            if (bitmap) {
                NSData *pngData = [bitmap representationUsingType:NSPNGFileType properties:Nil];
                [bitmap release];

                if (pngData) {
                    NSString *path = [NSString stringWithUTF8String:streamPngFilePath.str().c_str()];
                    [pngData writeToFile:path atomically:YES];
                }
            }
            CGImageRelease(imgOutput);


        }

    } while (0);

    //clean
    if (colorSpace!=NULL) {
        CGColorSpaceRelease(colorSpace);
    }
    if (bmpContext!=NULL) {
        CGContextRelease(bmpContext);
    }
    if (pBmpContextData!=NULL) {
        delete [] pBmpContextData;
    }


    if (image!=NULL) {
        CGImageRelease(image);
    }
    if (dataProvider!=NULL) {
        CGDataProviderRelease(dataProvider);
    }

    return nRet;
}
int SystemDownSamplePerformanceTest(const char *inputFile, int nWidth, int nHeight, int nQualityLevel, int nTimes)
{
    return SystemDownSample(inputFile,nWidth,nHeight,nQualityLevel,false,nTimes);
}

int SystemDownSampleQualityTest(const char *inputFile, int nWidth, int nHeight, int nQualityLevel)
{
    return SystemDownSample(inputFile, nWidth, nHeight, nQualityLevel, true, 1);
}

void LaunchOneApp() {
#ifdef MACOS
    if(![[NSWorkspace sharedWorkspace] launchApplication:@"/Applications/Safari.app"])
        NSLog(@"Path Finder failed to launch");
#endif
}