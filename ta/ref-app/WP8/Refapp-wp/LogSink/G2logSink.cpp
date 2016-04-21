#include "G2logSink.h"
#include "g2log/g2logworker.h"
#include "g2log/g2log.h"
#include "g2log/g2time.h"
#include <ppltasks.h>

using namespace Platform;
using namespace Windows::Storage;
using namespace concurrency;
using namespace Windows::Foundation;

namespace LogSink
{
	String^ folderName = L"Logs";
	static const std::string date_formatted = "%Y/%m/%d";
	static const std::string time_formatted = "%H:%M:%S";

	G2LogSink^ G2LogSink::m_pInstance = nullptr;

	G2LogSink^ G2LogSink::Instance::get()
	{
		if (m_pInstance == nullptr)
		{
			m_pInstance = ref new G2LogSink();
		}

		return m_pInstance;
	}

	void G2LogSink::SetFilePrefixName(Platform::String^ prefixName)
	{

		if (_g2LogWorker == nullptr)
		{ 
			_g2LogWorker = new g2LogWorker(Platformtring2UTF8String(prefixName), Platformtring2UTF8String(_localFolder->Path));
			g2::initializeLogging(_g2LogWorker);
			Log(INFO, L"create log file");
		}
	}

    G2LogSink::G2LogSink()
    {	
		CreateFolder();
		_isTimeStartLog = true;
    }

    G2LogSink::~G2LogSink()
    {
        if (_g2LogWorker != nullptr)
        {
            free(_g2LogWorker);
            _g2LogWorker = nullptr;
        }
    }

	void G2LogSink::CloseFile()
	{
		_g2LogWorker->close();
	}

	StorageFolder^ G2LogSink::GetCurrentFolder()
	{
		return _localFolder;
	}

	void G2LogSink::StartLog(bool isStart)
	{
		_isTimeStartLog = isStart;
		_g2LogWorker->startLog(isStart);
	}

    void G2LogSink::Info(String ^message)
    {

        LOG(INFO) << Platformtring2UTF8String(message);
    }



    void G2LogSink::Warn(String ^message)
    {
        LOG(WARNING) << Platformtring2UTF8String(message);
    }



    void G2LogSink::Error(String ^message)
    {
        LOG(ERR) << Platformtring2UTF8String(message);
    }



    void G2LogSink::Fatal(String ^message)
    {
        LOG(FATAL) << Platformtring2UTF8String(message);

    }


    void G2LogSink::Log(int logLevel, String ^message)
    {
		if (!_isTimeStartLog)
		{
			return;
		}

        switch (logLevel)
        {
        case INFO:
            LOG(INFO) << Platformtring2UTF8String(message);
            break;
		case WARNING:
			LOG(WARNING) << Platformtring2UTF8String(message);
			break;
		case DEBUG:
            LOG(DEBUG) << Platformtring2UTF8String(message);
            break;

        case FATAL:
            LOG(FATAL) << Platformtring2UTF8String(message);
            break;

		case ERR:
            LOG(ERR) << Platformtring2UTF8String(message);
            break;

        default:
            break;
        }

    }

	//void G2LogSink::LogF(int logLevel, String ^message, Windows::Foundation::Collections::IVector<Platform::Object^>^ params)
	//{
	//	if (!_isTimeStartLog)
	//	{
	//		return;
	//	}

	//	switch (logLevel)
	//	{
	//	case INFO:
	//		LOGF(INFO, message, params);
	//		break;
	//	case WARNING:
	//		LOG(WARNING) << Platformtring2UTF8String(message);
	//		break;
	//	case DEBUG:
	//		LOG(DEBUG) << Platformtring2UTF8String(message);
	//		break;

	//	case FATAL:
	//		LOG(FATAL) << Platformtring2UTF8String(message);
	//		break;

	//	case ERR:
	//		LOG(ERR) << Platformtring2UTF8String(message);
	//		break;

	//	default:
	//		break;
	//	}

	//}

    void G2LogSink::Debug(String ^message)
    {
        LOG(DEBUG) << Platformtring2UTF8String(message);
    }

	void G2LogSink::CreateFolder()
	{

		StorageFolder ^localFolder = ApplicationData::Current->LocalFolder;
		// synchronously create/open the log folder, otherwise it will miss some log when the file isn't created/opened.
		HANDLE handle = CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);
		IAsyncOperation<StorageFolder ^> ^op = localFolder->CreateFolderAsync(folderName, CreationCollisionOption::OpenIfExists);
		op->Completed = ref new AsyncOperationCompletedHandler<StorageFolder ^>([&](IAsyncOperation<StorageFolder ^> ^asyncInfo, AsyncStatus asyncStatus)
		{
			try
			{
				//For C++/CX, IAsyncOperation::Close is automatically called in the destructor.
				_localFolder = asyncInfo->GetResults();
			}
			catch (Exception ^e)
			{
				//LogMessage2Console(L"create log file failed. exception:[result=%d,Message=%s]", e->HResult, e->Message);
			}
			SetEvent(handle);
		});
		WaitForSingleObjectEx(handle, INFINITE, FALSE);
		CloseHandle(handle);

	}

	std::vector<char>  G2LogSink::Platformtring2UTF8Buffer(Platform::String^ str)
	{
		// A null value cannot be marshalled for Platform::String^, so they should never be null
		if (str->IsEmpty())
		{
			// Return an "empty" string
			return std::vector<char>(1);
		}

		// Get the size of the utf-8 string
		int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, str->Data(), str->Length(), nullptr, 0, nullptr, nullptr);
		if (size == 0)
		{
			// Not much we can do here; just return an empty string
			return std::vector<char>(1);
		}

		// Allocate the buffer and do the conversion
		std::vector<char> buffer(size + 1 /* null */);
		if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, str->Data(), str->Length(), buffer.data(), size, nullptr, nullptr) == 0)
		{
			// Not much we can do here; just return an empty string
			return std::vector<char>(1);
		}

		return buffer;
	}

	std::string G2LogSink::Platformtring2UTF8String(Platform::String^ str)
	{
		std::vector<char> v = Platformtring2UTF8Buffer(str);
		std::string utf8Str(v.begin(), v.end());
		return utf8Str;
	}
}