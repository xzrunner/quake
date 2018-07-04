#include "quake/Lightmaps.h"

#include <unirender/Texture.h>
#include <unirender/Blackboard.h>
#include <model/TextureLoader.h>

#include <string.h>

namespace quake
{

CU_SINGLETON_DEFINITION(Lightmaps);

Lightmaps::Lightmaps()
{
	Clear();
}

int Lightmaps::AllocBlock(int w, int h, int* x, int* y)
{
	// ericw -- rather than searching starting at lightmap 0 every time,
	// start at the last lightmap we allocated a surface in.
	// This makes AllocBlock much faster on large levels (can shave off 3+ seconds
	// of load time on a level with 180 lightmaps), at a cost of not quite packing
	// lightmaps as tightly vs. not doing this (uses ~5% more lightmaps)
	for (int texnum = m_last_lightmap_allocated; texnum < MAX_LIGHTMAPS; ++texnum, ++m_last_lightmap_allocated)
	{
		int best = BLOCK_HEIGHT;

		for (int i = 0; i < BLOCK_WIDTH - w; ++i)
		{
			int best2 = 0;

			int j = 0;
			for ( ; j < w; ++j)
			{
				if (m_allocated[texnum][i + j] >= best) {
					break;
				}
				if (m_allocated[texnum][i + j] > best2) {
					best2 = m_allocated[texnum][i + j];
				}
			}
			if (j == w)
			{	// this is a valid spot
				*x = i;
				*y = best = best2;
			}
		}

		if (best + h > BLOCK_HEIGHT)
			continue;

		for (int i = 0; i < w; ++i) {
			m_allocated[texnum][*x + i] = best + h;
		}

		return texnum;
	}

	return 0; //johnfitz -- shut up compiler
}

uint8_t* Lightmaps::Query(int tex_idx, int x, int y)
{
	uint8_t* base = m_lightmaps + tex_idx * BPP * BLOCK_WIDTH * BLOCK_HEIGHT;
	base += (y * BLOCK_WIDTH + x) * BPP;
	return base;
}

void Lightmaps::CreatetTextures()
{
	auto& rc = ur::Blackboard::Instance()->GetRenderContext();
	for (int i = 0; i < MAX_LIGHTMAPS; ++i)
	{
		if (!m_allocated[i][0]) {
			break;
		}

		auto data = m_lightmaps + i * BLOCK_WIDTH * BLOCK_HEIGHT * BPP;
		m_textures[i] = model::TextureLoader::LoadFromMemory(data, BLOCK_WIDTH, BLOCK_HEIGHT, BPP);
	}
}

unsigned int Lightmaps::GetTexID(int idx) const
{
	if (idx >= 0 && idx < MAX_LIGHTMAPS && m_textures[idx]) {
		return m_textures[idx]->TexID();
	} else {
		return 0;
	}
}

void Lightmaps::Clear()
{
	memset(m_allocated, 0, sizeof(m_allocated));
	m_last_lightmap_allocated = 0;

	memset(m_lightmaps, 0xff, sizeof(m_lightmaps));

	for (int i = 0; i < MAX_LIGHTMAPS; ++i) {
		m_textures[i].reset();
	}
}

}