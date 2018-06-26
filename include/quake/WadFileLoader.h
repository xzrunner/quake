#pragma once

#include <string>

namespace quake
{

class Palette;

class WadFileLoader
{
public:
	WadFileLoader(const Palette& palette);

	void Load(const std::string& wad_filepath);

private:
	static std::string LoadString(const char* data, int len);

private:
	const Palette& m_palette;

}; // WadFileLoader

}