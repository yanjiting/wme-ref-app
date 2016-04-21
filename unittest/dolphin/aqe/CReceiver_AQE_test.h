#ifndef CAPTURE_RECEIVER_H
#define CAPTURE_RECEIVER_H


class CaptureReceiver : public ICaptureReceiver
{
public:
	virtual int OnCaptureData (CWbxAeMediaBlock* pDataBlock)
	{
		return 0;
	}
protected:
private:
};



class PlaybackReceiver: public IPlaybackReceiver
{
public:
	virtual int OnPlaybackData (CWbxAeMediaBlock* pDataBlock)
	{
		return 0;
	}
protected:
private:
};

#endif