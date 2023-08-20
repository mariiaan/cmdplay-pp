#include "AudioSource.hpp"
#include "AudioEngine.hpp"
#include "../Instance.hpp"
#include "AudioException.hpp"
#include <iostream>

cmdplay::audio::AudioSource::AudioSource(bool fxEnabled)
{
	m_audioEngine = cmdplay::Instance::AudioEngine;
	if (m_audioEngine == nullptr)
		throw AudioException("Audio Engine not initialized");
	m_fxEnabled = fxEnabled;
}

cmdplay::audio::AudioSource::AudioSource(const std::string& filename,
	bool fxEnabled)
{
	m_audioEngine = cmdplay::Instance::AudioEngine;
	if (m_audioEngine == nullptr)
		throw AudioException("Audio Engine not initialized");
	m_fxEnabled = fxEnabled;
	Load(filename);
}

cmdplay::audio::AudioSource::~AudioSource()
{
	try
	{
		if (m_streamHandle != -1)
			m_audioEngine->DeleteBassStream(m_streamHandle);
	}
	catch(AudioException&){}
	try
	{
		if (m_streamHelperHandle != -1)
			m_audioEngine->DeleteBassStream(m_streamHelperHandle);
	}
	catch (AudioException&) {}
	m_streamHandle = m_streamHelperHandle = -1;
}

void cmdplay::audio::AudioSource::Load(const std::string& filename)
{
	if (m_fxEnabled)
	{
		// When BASS FX is enabled, create a helper stream for actually loading
		// the audio file
		m_streamHelperHandle = m_audioEngine->CreateBassStreamFromFile(
			filename, BASS_STREAM_DECODE | BASS_SAMPLE_FLOAT);
		// and wrap a BASS FX stream around it
		m_streamHandle = m_audioEngine->CreateBassFxTempoStream(
			m_streamHelperHandle, BASS_FX_FREESOURCE);
		m_baseTempo = GetTempo();
		std::cout << "[INFO] Created audio source from " << filename << std::endl;
	}
	else
	{
		// otherwise, just create a stream for loading the sample
		m_streamHandle = m_audioEngine->CreateBassStreamFromFile(filename,
			BASS_SAMPLE_FLOAT);
		std::cout << "[INFO] Created audio source from " << filename << std::endl;
	}
}

bool cmdplay::audio::AudioSource::IsFxEnabled()
{
	return m_fxEnabled;
}

void cmdplay::audio::AudioSource::Play()
{
	m_audioEngine->PlayBassChannel(m_streamHandle);
}

void cmdplay::audio::AudioSource::Pause()
{
	m_audioEngine->PauseBassChannel(m_streamHandle);
}

void cmdplay::audio::AudioSource::Stop()
{
	m_audioEngine->StopBassChannel(m_streamHandle);
}

void cmdplay::audio::AudioSource::PlayPause()
{
	if (GetPlaying())
		Pause();
	else
		Play();
}

void cmdplay::audio::AudioSource::SetPlaybackPosition(float value)
{
	m_audioEngine->SetPlaybackPositionChannel(m_streamHandle, value);
}

float cmdplay::audio::AudioSource::GetPlaybackPosition()
{
	return m_audioEngine->GetPlaybackPositionChannel(m_streamHandle);
}

float cmdplay::audio::AudioSource::GetTotalLength()
{
	return m_audioEngine->GetLengthChannel(m_streamHandle);
}

void cmdplay::audio::AudioSource::SetPan(float value)
{
	m_audioEngine->SetPanChannel(m_streamHandle, value);
}

float cmdplay::audio::AudioSource::GetPan()
{
	return m_audioEngine->GetPanChannel(m_streamHandle);
}

void cmdplay::audio::AudioSource::SetVolume(float value)
{
	m_audioEngine->SetVolumeChannel(m_streamHandle, value);
}

float cmdplay::audio::AudioSource::GetVolume()
{
	return m_audioEngine->GetVolumeChannel(m_streamHandle);
}

void cmdplay::audio::AudioSource::SetPlaying(bool value)
{
	if (value)
	{
		if (!GetPlaying())
			Play();
	}
	else if (GetPlaying())
		Pause();
}

bool cmdplay::audio::AudioSource::GetPlaying()
{
	return m_audioEngine->IsPlayingChannel(m_streamHandle);
}

void cmdplay::audio::AudioSource::SetTempo(float value)
{
	if (!m_fxEnabled)
		throw AudioException("NoFx");
	m_audioEngine->SetTempoChannel(m_streamHandle, value * m_baseTempo);
}

float cmdplay::audio::AudioSource::GetTempo()
{
	return m_audioEngine->GetTempoChannel(m_streamHandle) / m_baseTempo;
}

HSTREAM cmdplay::audio::AudioSource::GetStreamHandle()
{
	return m_streamHandle;
}

HSTREAM cmdplay::audio::AudioSource::GetStreamHelperHandle()
{
	return m_streamHelperHandle;
}

void cmdplay::audio::AudioSource::GetSpectrum(float* targetData, 
	size_t targetDataSize, SpectrumElementCount fftSize)
{
	m_audioEngine->GetBassChannelSpectrum(m_streamHandle, 
		targetData, targetDataSize, fftSize);
}
