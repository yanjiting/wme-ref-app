#include "DummyBackdoorAgent.h"
#include "json/json.h"
#include "DummyDbg.h"
//=====================================================================================================
//
CDummyBackdoorAgent::CDummyBackdoorAgent(){

	m_mapBackdoors.insert(std::make_pair("backdoorShowQRCodeView", &CDummyBackdoorAgent::backdoorShowQRCodeView));
	m_mapBackdoors.insert(std::make_pair("backdoorHideQRCodeView", &CDummyBackdoorAgent::backdoorHideQRCodeView));
	m_mapBackdoors.insert(std::make_pair("backdoorStopDummyApp", &CDummyBackdoorAgent::backdoorStopDummyApp));
	
}
CDummyBackdoorAgent::~CDummyBackdoorAgent(){

}
/*
	{
		"view_x": 0,
		"view_y": 0,
		"view_w": 500,
		"view_h": 400,
		"qrcode_grid_row" : 2,
		"qrcode_grid_col" : 2,
		"qrcode_contents" : ["1.1","1.2","2.1","2.2"],
	}
*/
std::string CDummyBackdoorAgent::backdoorShowQRCodeView(std::string arg){
	//pass arg
	Json::Reader r;
	Json::Value input;
	r.parse(arg, input, true);

	//spcific logic
	int x = input["view_x"].asInt();
	int y = input["view_y"].asInt();
	int width = input["view_w"].asInt();
	int height = input["view_h"].asInt();
	int row = input["qrcode_grid_row"].asInt();
	int col = input["qrcode_grid_col"].asInt();
	std::vector<std::string> v_qr_contents;
	if (input["qrcode_contents"].isArray()){
		Json::Value::iterator it;
		int nRow = 0;
		int nCol = 0;
		for (it = input["qrcode_contents"].begin(); it != input["qrcode_contents"].end();it++){
			//(nRow,nCol)
			std::string strQRCodeContent = (*it).asString();
			v_qr_contents.push_back(strQRCodeContent);

			//Update row/column
			nCol++;
			if (nCol == col){
				nCol = 0; 
				nRow++;
			}
			if (nRow == row)
				break;
		}
	}
	ShowQRCodeView(x, y, width, height, row, col, v_qr_contents);
	//result
	Json::Value ret(Json::objectValue);
	ret["ret"] = "true";

	Json::FastWriter w;
	std::string sRet = w.write(ret);
	CM_INFO_TRACE("backdoorShowQRCodeView, result=" << sRet);
	return sRet;
}

std::string CDummyBackdoorAgent::backdoorHideQRCodeView(std::string arg){
	//pass arg
	Json::Reader r;
	Json::Value input;
	r.parse(arg, input, true);

	//spcific logic
	HideQRCodeView();

	//result
	Json::Value ret(Json::objectValue);
	ret["ret"] = "true";

	Json::FastWriter w;
	std::string sRet = w.write(ret);
	CM_INFO_TRACE("backdoorShowQRCodeView, result=" << sRet);
	return sRet;
}
std::string CDummyBackdoorAgent::backdoorStopDummyApp(std::string arg){
	//pass arg
	Json::Reader r;
	Json::Value input;
	r.parse(arg, input, true);

	//spcific logic
	//nothing

	//result
	Json::Value ret(Json::objectValue);
	ret["ret"] = "true";

	Json::FastWriter w;
	std::string sRet = w.write(ret);
	CM_INFO_TRACE("backdoorShowQRCodeView, result=" << sRet);
	return sRet;
}

//=====================================================================================================
//
CDummyBackdoorAgent *g_DummyBackdoorAgent = NULL;
DummyConfig DummyConfig::dummyConfig;

char* getCmdOption(char ** begin, char ** end, const std::string & option) {
	char ** itr = std::find(begin, end, option);
	if (itr != end && ++itr != end) {
		return *itr;
	}
	return NULL;
}
void InitAgentByInputParams(int argc, const char ** argv){

	int port = 0;
	const char* szTAPort = getCmdOption((char **)argv, (char **)(argv + argc), "--ta");
	if (szTAPort)
		port = atoi(szTAPort);
	else
		DUMMY_ERROR_TRACE("need command param '--ta port' ");
	// --show-qrcodes-json "{'view_x': 0,'view_y': 0,'view_w' : 500,'view_h' : 400,'qrcode_grid_row' : 2,'qrcode_grid_col' : 2,'qrcode_contents' : ['1.1', '1.2', '2.1', '2.2'],}"
	const char* szQRCodesJson = getCmdOption((char **)argv, (char **)(argv + argc), "--show-qrcodes-json");
	if (szQRCodesJson){
		DummyConfig::Instance().m_strShowQRCode = szQRCodesJson;
		std::replace(DummyConfig::Instance().m_strShowQRCode.begin(), DummyConfig::Instance().m_strShowQRCode.end(), '\'', '\"');
	}

	g_DummyBackdoorAgent = new CDummyBackdoorAgent();
	if (!(port > 0
		&& g_DummyBackdoorAgent->Init(port))){
		DUMMY_ERROR_TRACE("CDummyBackdoorAgent::Init fail");
	}

	return;
}

void CleanupAgent()
{
	if (g_DummyBackdoorAgent){
		g_DummyBackdoorAgent->Finit();
		delete g_DummyBackdoorAgent;
	}
	g_DummyBackdoorAgent = NULL;
}
