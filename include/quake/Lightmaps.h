#pragma once

#include <cu/cu_macro.h>
#include <unirender/typedef.h>

#include <cstdint>
#include <memory>

namespace ur { class Device; }

namespace quake
{

class Lightmaps
{
public:
	int AllocBlock(int w, int h, int* x, int* y);

	uint8_t* Query(int tex_idx, int x, int y);

	void CreatetTextures(const ur::Device& dev);

	unsigned int GetTexID(int idx) const;

	void Clear();

public:
	static const int BLOCK_WIDTH  = 128;
	static const int BLOCK_HEIGHT = 128;;

	static const int MAX_LIGHTMAPS = 512;

	static const int BPP = 4;

private:
	int	m_allocated[MAX_LIGHTMAPS][BLOCK_WIDTH];
	int m_last_lightmap_allocated;

	uint8_t m_lightmaps[MAX_LIGHTMAPS * BPP * BLOCK_WIDTH * BLOCK_HEIGHT];

	ur::TexturePtr m_textures[MAX_LIGHTMAPS];

	CU_SINGLETON_DECLARATION(Lightmaps);

}; // Lightmaps

}