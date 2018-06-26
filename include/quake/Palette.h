#pragma once

#include <string>

namespace quake
{

class Palette
{
public:
	Palette();
	~Palette();

	void LoadFromFile(const std::string& filepath);

	void IndexedToRgb(const unsigned char* indexed, size_t size,
		unsigned char* rgb) const;

private:
	size_t m_size;
	unsigned char* m_data;

}; // Palette

}