#ifndef _ZXING_HELPER_H_
#define _ZXING_HELPER_H_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <zxing/common/Counted.h>
#include <zxing/Result.h>
#include <zxing/BinaryBitmap.h>
#include <zxing/DecodeHints.h>


using namespace std;
//using namespace zxing;

class  zxinghelp {
public:
	zxinghelp()
	{

	}
	virtual ~zxinghelp()
	{

	}
	std::string GetBarCodeFromImg(const char* strFile);
private:
	vector<zxing::Ref<zxing::Result> > decode(zxing::Ref<zxing::BinaryBitmap> image, zxing::DecodeHints hints);
	vector<zxing::Ref<zxing::Result> > decode_multi(zxing::Ref<zxing::BinaryBitmap> image, zxing::DecodeHints hints);
	int read_image(zxing::Ref<zxing::LuminanceSource> source, bool hybrid, string &barcode);
};

#endif // _ZXING_HELPER_H_

