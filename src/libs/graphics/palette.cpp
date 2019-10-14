#include <limits>
#include "palette.hpp"


namespace graphics
{


palette::palette()
{
}


palette::palette(std::size_t const p_palette_size, color const &p_template)
	: m_colors(p_palette_size, p_template)
{
}


std::size_t find_nearest_color(palette const &p_palette, color const &p_color)
{
	long cur_min_distance = std::numeric_limits < long > ::max();
	std::size_t cur_best_palette_index = 0;

	for (std::size_t palette_index = 0; palette_index < p_palette.size(); ++palette_index)
	{
		color const & palette_entry = p_palette[palette_index];
		long distance = calculate_color_distance(palette_entry, p_color);

		if ((palette_index == 0) || (distance < cur_min_distance))
		{
			cur_min_distance = distance;
			cur_best_palette_index = palette_index;
		}
	}

	return cur_best_palette_index;
}


}
