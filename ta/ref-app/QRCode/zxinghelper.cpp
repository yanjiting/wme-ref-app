// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
/*
 *  Copyright 2010-2011 ZXing authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "zxinghelper.h"
#include "ImageReaderSource.h"
#include <zxing/common/Counted.h>
#include <zxing/Binarizer.h>
#include <zxing/MultiFormatReader.h>
#include <zxing/Result.h>
#include <zxing/ReaderException.h>
#include <zxing/common/GlobalHistogramBinarizer.h>
#include <zxing/common/HybridBinarizer.h>
#include <exception>
#include <zxing/Exception.h>
#include <zxing/common/IllegalArgumentException.h>
#include <zxing/BinaryBitmap.h>
//#include <zxing/DecodeHints.h>

#include <zxing/qrcode/QRCodeReader.h>
#include <zxing/multi/qrcode/QRCodeMultiReader.h>
#include <zxing/multi/ByQuadrantReader.h>
#include <zxing/multi/MultipleBarcodeReader.h>
#include <zxing/multi/GenericMultipleBarcodeReader.h>

using namespace std;
using namespace zxing;
using namespace zxing::multi;
using namespace zxing::qrcode;


bool more = false;
bool try_harder = false;
bool search_multi = false;
bool use_hybrid = false;
bool use_global = false;
bool verbose = false;



vector<Ref<Result> > zxinghelp::decode(Ref<BinaryBitmap> image, DecodeHints hints) {
	Ref<Reader> reader(new MultiFormatReader);
	return vector<Ref<Result> >(1, reader->decode(image, hints));
}

vector<Ref<Result> > zxinghelp::decode_multi(Ref<BinaryBitmap> image, DecodeHints hints) {
	MultiFormatReader delegate;
	GenericMultipleBarcodeReader reader(delegate);
	return reader.decodeMultiple(image, hints);
}

int zxinghelp::read_image(Ref<LuminanceSource> source, bool hybrid, string &barcode) {
	vector<Ref<Result> > results;
	string cell_result;
	int res = -1;

	try {
		Ref<Binarizer> binarizer;
		if (hybrid) {
			binarizer = new HybridBinarizer(source);
		} else {
			binarizer = new GlobalHistogramBinarizer(source);
		}
		DecodeHints hints(DecodeHints::DEFAULT_HINT);
		hints.setTryHarder(try_harder);
		Ref<BinaryBitmap> binary(new BinaryBitmap(binarizer));
		if (search_multi) {
			results = decode_multi(binary, hints);
		} else {
			results = decode(binary, hints);
		}
		res = 0;
	} catch (const ReaderException& e) {
		cell_result = "zxing::ReaderException: " + string(e.what());
		res = -2;
	} catch (const zxing::IllegalArgumentException& e) {
		cell_result = "zxing::IllegalArgumentException: " + string(e.what());
		res = -3;
	} catch (const zxing::Exception& e) {
		cell_result = "zxing::Exception: " + string(e.what());
		res = -4;
	} catch (const std::exception& e) {
		cell_result = "std::exception: " + string(e.what());
		res = -5;
	}




	if(results.size() == 0)
		barcode = "error";
	else
		barcode =results[0]->getText()->getText();


	return res;
}


std::string zxinghelp::GetBarCodeFromImg(const char* strFile) {
    
    if(strFile == NULL) return "";
    
	int total = 0;
	int gonly = 0;
	int honly = 0;
	int both = 0;
	int neither = 0;
    
	string filename ;
    
	string barcode ;
	filename.empty();
	barcode.empty();
	filename = strFile;
    
    
	if (filename.length() > 3 &&
		(filename.substr(filename.length() - 3, 3).compare("txt") == 0 ||
         filename.substr(filename.length() - 3, 3).compare("bin") == 0)) {
			return "";
        }

    
	if (!use_global && !use_hybrid) {
		use_global = use_hybrid = true;
	}
    
	Ref<LuminanceSource> source;
	try {
		source = ImageReaderSource::create(filename);
	} catch (const zxing::IllegalArgumentException &e) {
		return  "";
	}
    
    
	int gresult = 1;
	int hresult = 1;
    
	if (use_hybrid) {
		hresult = read_image(source, true, barcode);
	}
	if (use_global && (verbose || hresult != 0)) {
		gresult = read_image(source, false, barcode);
		if (!verbose && gresult != 0) {
			return "";
		}
	}
    
	if(barcode != "error")
	{
		return barcode;
	}
    
	return "";
}




