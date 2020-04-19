#pragma once

#include <string>

namespace ur2 { class Device; }

namespace quake
{

class Palette;

class WadFileLoader
{
public:
	WadFileLoader(const Palette& palette);

	void Load(const ur2::Device& dev,
        const std::string& wad_filepath);

private:
	static std::string LoadString(const char* data, int len);

private:
	const Palette& m_palette;

}; // WadFileLoader

}