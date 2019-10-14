#ifndef GRAPHICS_PALETTE_HPP___________
#define GRAPHICS_PALETTE_HPP___________

#include <cstddef>
#include <vector>
#include "color.hpp"


namespace graphics
{


struct palette
{
	typedef std::vector < color > colors;
	colors m_colors;

	typedef colors::const_iterator const_iterator;
	typedef colors::iterator iterator;

	palette();
	explicit palette(std::size_t const p_palette_size, color const &p_template);

	color& operator[](std::size_t const p_index)
	{
		return m_colors[p_index];
	}

	color const & operator[](std::size_t const p_index) const
	{
		return m_colors[p_index];
	}

	std::size_t size() const
	{
		return m_colors.size();
	}
};

inline palette::colors::const_iterator cbegin(palette const &p_palette)
{
	return p_palette.m_colors.cbegin();
}

inline palette::colors::const_iterator cend(palette const &p_palette)
{
	return p_palette.m_colors.cend();
}

inline palette::colors::iterator begin(palette &p_palette)
{
	return p_palette.m_colors.begin();
}

inline palette::colors::iterator end(palette &p_palette)
{
	return p_palette.m_colors.end();
}

std::size_t find_nearest_color(palette const &p_palette, color const &p_color);

	
} // namespace graphics end


#endif // GRAPHICS_PALETTE_HPP___________
