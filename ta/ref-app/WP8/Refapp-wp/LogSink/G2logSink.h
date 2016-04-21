#pragma once

#include "g2log/g2log.h"
#include <string>
#include <vector>

using namespace Platform;
using namespace Windows::Storage;

namespace LogSink
{
    public ref class G2LogSink sealed
	{
	public:
		G2LogSink();
		virtual ~G2LogSink();

		StorageFolder^ GetCurrentFolder();
		void SetFilePrefixName(Platform::String^ prefixName);
		void CloseFile();

		void Debug(String^ message);
		void Info(String^ message);
		void Warn(String^ message);
		void Error(String^ message);
		void Fatal(String^ message);
		void Log(int logLevel, String^ message);
		//void LogF(int logLevel, String ^message, Windows::Foundation::Collections::IVector<Platform::Object^>^ params);
		void StartLog(bool isStart);
		static property G2LogSink^ Instance
		{
			G2LogSink^ get();
		}
	private:
		void CreateFolder();
		std::vector<char> Platformtring2UTF8Buffer(Platform::String^ str);
		std::string Platformtring2UTF8String(Platform::String^ str);
		bool _isTimeStartLog;
		g2LogWorker *_g2LogWorker;
		StorageFolder ^_localFolder;
		static G2LogSink^ m_pInstance;
	};

}

