#pragma once

class IBackdoorSink{
public:
	virtual bool OnRequest(std::string selector, std::string arg, std::string &result) = 0;
};

