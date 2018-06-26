#include "quake/WadFileLoader.h"
#include "quake/TextureManager.h"
#include "quake/Palette.h"

#include <bs/ImportStream.h>
#include <unirender/Blackboard.h>
#include <unirender/RenderContext.h>

#include <fstream>
#include <vector>

#include <assert.h>

namespace
{

static const int NAME_LEN = 16;

struct WadHeader
{
	char    magic[4];				// "WAD2", Name of the new WAD format
	int32_t numentries;             // Number of entries
	int32_t diroffset;              // Position of WAD directory in file
};

struct WadEntry
{
	int32_t offset;                 // Position of the entry in WAD
	int32_t dsize;                  // Size of the entry in WAD file
	int32_t size;                   // Size of the entry in memory
	char    type;                   // type of entry
	char    cmprs;                  // Compression. 0 if none.
	int16_t dummy;                  // Not used
	char    name[NAME_LEN];         // 1 to 16 characters, '\0'-padded
};

namespace WadEntryType
{
	static const char PALETTE   = '@';	// Color Palette
	static const char STATUS    = 'B';	// Pictures for status bar
	static const char MIP		= 'D';	// Used to be Mip Texture
	static const char CONSOLE   = 'E';	// Console picture (flat)
}

static const int MIP_LEVEL = 4;

}

namespace quake
{

WadFileLoader::WadFileLoader(const Palette& palette)
	: m_palette(palette)
{
}

void WadFileLoader::Load(const std::string& wad_filepath)
{
	std::ifstream fin(wad_filepath, std::ios::binary);
	if (fin.fail()) {
		return;
	}

	WadHeader header;
	fin.read(reinterpret_cast<char*>(&header), sizeof(header));
	if (strncmp(header.magic, "WAD2", 4) != 0) {
		fin.close();
		return;
	}

	fin.seekg(header.diroffset);
	WadEntry* entries = new WadEntry[header.numentries];
	fin.read(reinterpret_cast<char*>(entries), sizeof(WadEntry) * header.numentries);

	char* buf = new char[header.diroffset];

	auto tex_mgr = TextureManager::Instance()->Instance();
	for (int i = 0; i < header.numentries; ++i)
	{
		auto& entry = entries[i];
		assert(entry.size == entry.dsize && entry.type == 'D' && entry.cmprs == 0);

		fin.seekg(entry.offset);
		fin.read(buf, entry.dsize);

		bs::ImportStream is(buf, header.diroffset);
		std::string name = is.String(NAME_LEN);
		uint32_t width = is.UInt32();
		uint32_t height = is.UInt32();
		size_t offset[MIP_LEVEL];
		for (int i = 0; i < MIP_LEVEL; ++i) {
			offset[i] = is.UInt32();
		}

		uint32_t mip_w = width;
		uint32_t mip_h = height;

		auto& rc = ur::Blackboard::Instance()->GetRenderContext();
		int tex_id = rc.CreateTextureID(width, height, ur::TEXTURE_RGB, MIP_LEVEL != 1);
		for (int i = 0; i < MIP_LEVEL; ++i)
		{
			size_t pixel_sz = mip_w * mip_h;
			size_t rgb_sz = pixel_sz * 3;
			unsigned char* rgb = new unsigned char[rgb_sz];
			m_palette.IndexedToRgb((unsigned char*)buf + offset[i], pixel_sz, rgb);

			rc.UpdateTexture(tex_id, rgb, mip_w, mip_h, 0, i);
			delete[] rgb;

			mip_w /= 2;
			mip_h /= 2;
		}

		auto tex = std::make_unique<pt2::Texture>(width, height, tex_id, ur::TEXTURE_RGB);
		TextureManager::Instance()->Add(name, tex);
	}

	delete[] buf;
	delete[] entries;

	fin.close();
}

std::string WadFileLoader::LoadString(const char* data, int len)
{
	std::vector<char> buffer;
	buffer.resize(len + 1);
	buffer[len] = 0;
	std::memcpy(buffer.data(), data, len);
	return std::string(buffer.data());
}

}