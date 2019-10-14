#ifndef GRAPHICS_COLOR_HPP___________
#define GRAPHICS_COLOR_HPP___________

#include <cstddef>
#include <array>
#include <map>
#include "base/progress_report.hpp"
#include "pixmap_view.hpp"


namespace graphics
{


struct color
{
	std::array < int, 3 > m_rgb_values;

	color();
	explicit color(int const p_red, int const p_green, int const p_blue);

	color& operator += (color const &p_other);
	color& operator -= (color const &p_other);
	color& operator *= (int const p_value);
	color& operator /= (int const p_value);

	int operator [](std::size_t const p_index) const
	{
		return m_rgb_values[p_index];
	}

	int& operator [](std::size_t const p_index)
	{
		return m_rgb_values[p_index];
	}
};

color operator + (color const &p_first, color const &p_second);
color operator - (color const &p_first, color const &p_second);
color operator * (color const &p_color, int const p_value);
color operator / (color const &p_color, int const p_value);
bool operator < (color const &p_first, color const &p_second);
bool operator == (color const &p_first, color const &p_second);
bool operator != (color const &p_first, color const &p_second);

std::string to_string(color const &p_color);

long calculate_color_distance(color const &p_first, color const &p_second);


typedef std::map < color, std::size_t > color_histogram;

void compute_color_histogram(
	color_histogram &p_color_histogram,
	const_pixmap_view_t p_input_pixmap,
	base::progress_report_callback const &p_progress_report_callback = base::progress_report_callback()
);


} // namespace graphics end


#endif // GRAPHICS_COLOR_HPP___________
