#include "AudioException.hpp"
#include "AudioEngine.hpp"
#include <iostream>

cmdplay::audio::AudioEngine::AudioEngine(
	int32_t device, int32_t sampleRate, HWND windowHwnd)
{
	std::cout << "[INFO] Initializing Audio Engine (dev: " << device << "; freq: " << 
		sampleRate << ")" << std::endl;

	BASS_SetConfig(BASS_CONFIG_VISTA_TRUEPOS, 0);
	BASS_SetConfig(BASS_CONFIG_UPDATEPERIOD, 5);
	BASS_SetConfig(BASS_CONFIG_DEV_BUFFER, 10);
	BASS_SetConfig(BASS_CONFIG_BUFFER, 100);
	BASS_SetConfig(BASS_CONFIG_DEV_NONSTOP, 1);

	if (!BASS_Init(device, sampleRate, NULL, windowHwnd, nullptr))
		CheckAndThrowError("AudioEngine::AudioEngine");

	m_sampleRate = sampleRate;
	
	std::cout << "[INFO] Initialized Audio Engine successfully" << std::endl;
}

cmdplay::audio::AudioEngine::~AudioEngine()
{
	std::cout << "[INFO] Freeing Audio Engine" << std::endl;
	if (BASS_Free())
		std::cout << "[INFO] Freed Audio Engine successfully" << std::endl;
	else
		std::cout << "[ERROR] Failed to free Audio Engine" << std::endl;
}

int cmdplay::audio::AudioEngine::GetSampleRate()
{
	return m_sampleRate;
}

HSTREAM cmdplay::audio::AudioEngine::CreateBassStreamFromFile(
	const std::string& filename, DWORD flags)
{
	HSTREAM yield = BASS_StreamCreateFile(FALSE, filename.c_str(), 0, 0, flags);
	CheckAndThrowError("AudioEngine::CreateBassStreamFromFile");

	return yield;
}

HSTREAM cmdplay::audio::AudioEngine::CreateBassFxTempoStream(HSTREAM src,
	DWORD flags)
{
	HSTREAM yield = BASS_FX_TempoCreate(src, flags);
	CheckAndThrowError("AudioEngine::CreateBassFxTempoStream");
	return yield;
}

void cmdplay::audio::AudioEngine::DeleteBassStream(HSTREAM src)
{
	if (!BASS_StreamFree(src))
		CheckAndThrowError("AudioEngine::DeleteBassStream");
}

void cmdplay::audio::AudioEngine::PlayBassChannel(DWORD src)
{
	BASS_ChannelPlay(src, false);
	CheckAndThrowError("AudioEngine::PlayBassChannel");
}

void cmdplay::audio::AudioEngine::PauseBassChannel(DWORD src)
{
	BASS_ChannelPause(src);
	CheckAndThrowError("AudioEngine::PauseBassChannel");
}

void cmdplay::audio::AudioEngine::StopBassChannel(DWORD src)
{
	BASS_ChannelStop(src);
	CheckAndThrowError("AudioEngine::StopBassChannel");
}

void cmdplay::audio::AudioEngine::SetTempoChannel(DWORD src, float tempo)
{
	BASS_ChannelSetAttribute(src, BASS_ATTRIB_TEMPO_FREQ,
		GetSampleRate() * tempo);
	CheckAndThrowError("AudioEngine::SetTempoChannel");
}

float cmdplay::audio::AudioEngine::GetTempoChannel(DWORD src)
{
	float val;
	BASS_ChannelGetAttribute(src, BASS_ATTRIB_TEMPO_FREQ, &val);
	return val / GetSampleRate();
}

int32_t cmdplay::audio::AudioEngine::GetSampleRateStream(DWORD src)
{
	float val;
	BASS_ChannelGetAttribute(src, BASS_ATTRIB_FREQ, &val);
	CheckAndThrowError("AudioEngine::GetSampleRateStream");
	return static_cast<int32_t>(val);
}

void cmdplay::audio::AudioEngine::SetVolumeChannel(DWORD src, float volume)
{
	BASS_ChannelSetAttribute(src, BASS_ATTRIB_VOL, volume);
}

float cmdplay::audio::AudioEngine::GetVolumeChannel(DWORD src)
{
	float val;
	BASS_ChannelGetAttribute(src, BASS_ATTRIB_VOL, &val);
	CheckAndThrowError("AudioEngine::GetVolumeChannel");
	return val;
}

void cmdplay::audio::AudioEngine::SetPanChannel(DWORD src, float pan)
{
	BASS_ChannelSetAttribute(src, BASS_ATTRIB_PAN, pan);
	CheckAndThrowError("AudioEngine::SetPanChannel");
}

float cmdplay::audio::AudioEngine::GetPanChannel(DWORD src)
{
	float val;
	BASS_ChannelGetAttribute(src, BASS_ATTRIB_PAN, &val);
	CheckAndThrowError("AudioEngine::GetPanChannel");
	return val;
}

void cmdplay::audio::AudioEngine::SetPlaybackPositionChannel(DWORD src, float pos)
{
	double dPos = static_cast<double>(pos);
	QWORD nPos = BASS_ChannelSeconds2Bytes(src, dPos);
	BASS_ChannelSetPosition(src, nPos, BASS_POS_BYTE);
	CheckAndThrowError("AudioEngine::SetPlaybackPositionChannel");
}

float cmdplay::audio::AudioEngine::GetPlaybackPositionChannel(DWORD src)
{
	// Gets the playback position in bytes
	QWORD bp = BASS_ChannelGetPosition(src, BASS_POS_BYTE);

	// Converts the playback position from bytes to seconds
	double pos = BASS_ChannelBytes2Seconds(src, bp);
	return static_cast<float>(pos);
}

QWORD cmdplay::audio::AudioEngine::GetBytePlaybackPositionChannel(DWORD src)
{
	return BASS_ChannelGetPosition(src, BASS_POS_BYTE);
}

float cmdplay::audio::AudioEngine::GetLengthChannel(DWORD src)
{
	// Gets the playback length in bytes
	QWORD len = BASS_ChannelGetLength(src, BASS_POS_BYTE);

	// Converts the playback length from bytes to seconds
	double pos = BASS_ChannelBytes2Seconds(src, len);
	return static_cast<float>(pos);
}

QWORD cmdplay::audio::AudioEngine::GetByteLengthChannel(DWORD src)
{
	return BASS_ChannelGetLength(src, BASS_POS_BYTE);
}

bool cmdplay::audio::AudioEngine::IsPlayingChannel(DWORD src)
{
	return BASS_ChannelIsActive(src) == BASS_ACTIVE_PLAYING;
}

HSAMPLE cmdplay::audio::AudioEngine::CreateBassSampleFromFile(
	const std::string& filename, int32_t maxPlaybacks, DWORD flags)
{
	HSAMPLE yield;
	yield = BASS_SampleLoad(false, filename.c_str(), 0, 0, 
		static_cast<DWORD>(maxPlaybacks), flags);
	CheckAndThrowError("AudioEngine::CreateBassSampleFromFile");
	return yield;
}

void cmdplay::audio::AudioEngine::DeleteBassSample(HSAMPLE src)
{
	if (!BASS_SampleFree(src))
		CheckAndThrowError("AudioEngine::DeleteBassSample");
}

HCHANNEL cmdplay::audio::AudioEngine::CreateBassSampleChannel(HSAMPLE src)
{
	return BASS_SampleGetChannel(src, 0);
}

void cmdplay::audio::AudioEngine::CheckAndThrowError(const std::string& source)
{
	std::string errorSource;
	if (source.empty())
		errorSource = "AudioEngine::CheckAndThrowError";
	else
		errorSource = std::string(source);

	int32_t error = BASS_ErrorGetCode();
	switch (error)
	{
	case 0:
		return;
	case 1:
		throw AudioException("MemoryError"); break;
	case 2:
		throw AudioException("FileOpenError"); break;
	case 3:
		throw AudioException("DriverError"); break;
	case 4:
		throw AudioException("SampleBufferLost"); break;
	case 5:
		throw AudioException("InvalidHandle"); break;
	case 6:
		throw AudioException("UnknownError"); break;
	case 7:
		throw AudioException("InvalidPlaybackPosition"); break;
	case 8:
		throw AudioException("NotInitialized"); break;
	case 9:
		throw AudioException("NotStarted"); break;
	case 12:
		throw AudioException("NoCDInDrive"); break;
	case 13:
		throw AudioException("InvalidTrackNumber"); break;
	case 14:
		throw AudioException("AlreadyInitPausePlay"); break;
	case 20:
		throw AudioException("IllegalParameter"); break;
	case 23:
		throw AudioException("IllegalDeviceNumber"); break;
	case 41:
		throw AudioException("UnsupportedFileFormat"); break;
	case 5000:
		throw AudioException("WasapiError"); break;
	case -1:
		throw AudioException("UnknownError"); break;
	default:
		throw AudioException("GenericError"); break;
	}
}

void cmdplay::audio::AudioEngine::Update()
{
	BASS_Update(0);
}

void cmdplay::audio::AudioEngine::SetVolume(float value)
{
	BASS_SetVolume(value);
}

float cmdplay::audio::AudioEngine::GetVolume()
{
	return BASS_GetVolume();
}

void cmdplay::audio::AudioEngine::GetBassChannelSpectrum(HCHANNEL channel,
	float* target, size_t targetSize, SpectrumElementCount size)
{
	// The bass fft size is always double of the actual fft size returned for some reason
	switch (size)
	{
	case SpectrumElementCount::FFT_128:
	{
		if (targetSize < 128)
			throw AudioException("BufferTooSmallException"); 
		BASS_ChannelGetData(channel, target, BASS_DATA_FFT256);
		break;
	}
	case SpectrumElementCount::FFT_256:
	{
		if (targetSize < 256)
			throw AudioException("BufferTooSmallException");
		BASS_ChannelGetData(channel, target, BASS_DATA_FFT512);
		break;
	}
	case SpectrumElementCount::FFT_512:
	{
		if (targetSize < 512)
			throw AudioException("BufferTooSmallException");
		BASS_ChannelGetData(channel, target, BASS_DATA_FFT1024);
		break;
	}
	case SpectrumElementCount::FFT_1024:
	{
		if (targetSize < 1024)
			throw AudioException("BufferTooSmallException");
		BASS_ChannelGetData(channel, target, BASS_DATA_FFT2048);
		break;
	}
	case SpectrumElementCount::FFT_2048:
	{
		if (targetSize < 2048)
			throw AudioException("BufferTooSmallException");
		BASS_ChannelGetData(channel, target, BASS_DATA_FFT4096);
		break;
	}
	case SpectrumElementCount::FFT_4096:
	{
		if (targetSize < 4096)
			throw AudioException("BufferTooSmallException");
		BASS_ChannelGetData(channel, target, BASS_DATA_FFT8192);
		break;
	}
	default:
		throw 1; // todo: exception machen
		break; 
	}
	CheckAndThrowError("AudioEngine::GetBassChannelSpectrum");
}