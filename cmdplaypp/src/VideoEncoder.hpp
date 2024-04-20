#pragma once
#include <string>
#include "video/EncodeParams.hpp"

namespace cmdplay
{
	class VideoEncoder
	{
	public:
		VideoEncoder(const std::string& filepath, const std::string& brightnessLevels);
		
	private:
		std::string m_inputFilename;
		std::string m_brightnessLevels;

	private:
		/// <summary>
		/// Renders the given bitmap (RGB) onto another bitmap (RGB)
		/// </summary>
		/// <param name="targetBuf">Where should it be rendered to</param>
		/// <param name="targetWidth">Target buffer px width</param>
		/// <param name="targetHeight">Target buffer px height</param>
		/// <param name="toRender">The bitmap which should be rendered</param>
		/// <param name="toRenderWidth">The bitmaps x res</param>
		/// <param name="toRenderHeight">The bitmaps y res</param>
		/// <param name="x">Destination x</param>
		/// <param name="y">Destination y</param>
		void Render(unsigned char* targetBuf, int32_t targetWidth, int32_t targetHeight, unsigned char* toRender,
			int32_t toRenderWidth, int32_t toRenderHeight, int32_t x, int32_t y);

		/// <summary>
		/// Scales a given bitmap (RGB). Used for pre-scaling the font character bitmaps before any encoding
		/// happens.
		/// </summary>
		/// <param name="what">The bitmap to be scaled</param>
		/// <param name="inWidth">The original width</param>
		/// <param name="inHeight">The original height</param>
		/// <param name="targetWidth">The new width</param>
		/// <param name="targetHeight">The new height</param>
		/// <param name="destination">Where should the new data be written to. Must be preallocated</param>
		void Scale(unsigned char* what, int32_t inWidth, int32_t inHeight, int32_t targetWidth, int32_t targetHeight,
			unsigned char* destination);

	public:
		/// <summary>
		/// Opens an interactive menu where the user can set up some options
		/// </summary>
		/// <param name="outputFilename">The chosen output filename</param>
		/// <param name="params">The chosen EncodeParams</param>
		/// <returns>Whether the user confirmed the settings</returns>
		static bool OpenSettings(std::string& outputFilename, EncodeParams& params);

		/// <summary>
		/// Begins decoding the original, asciifying it and then reencoding it
		/// </summary>
		/// <param name="outputFilename">Where should the new video be written to</param>
		/// <param name="params"></param>
		/// <param name="output">Whether to output debug messages</param>
		void BeginEncoding(const std::string& outputFilename, EncodeParams params, bool output = true);
	};
}