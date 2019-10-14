#include "color.hpp"


namespace graphics
{


color::color()
{
}


color::color(int const p_red, int const p_green, int const p_blue)
	: m_rgb_values {{ p_red, p_green, p_blue }}
{
}


color& color::operator += (color const &p_other)
{
	m_rgb_values[0] += p_other.m_rgb_values[0];
	m_rgb_values[1] += p_other.m_rgb_values[1];
	m_rgb_values[2] += p_other.m_rgb_values[2];

	return *this;
}


color& color::operator -= (color const &p_other)
{
	m_rgb_values[0] -= p_other.m_rgb_values[0];
	m_rgb_values[1] -= p_other.m_rgb_values[1];
	m_rgb_values[2] -= p_other.m_rgb_values[2];

	return *this;
}


color& color::operator *= (int const p_value)
{
	m_rgb_values[0] *= p_value;
	m_rgb_values[1] *= p_value;
	m_rgb_values[2] *= p_value;

	return *this;
}


color& color::operator /= (int const p_value)
{
	m_rgb_values[0] /= p_value;
	m_rgb_values[1] /= p_value;
	m_rgb_values[2] /= p_value;

	return *this;
}


color operator + (color const &p_first, color const &p_second)
{
	color result = p_first;
	result += p_second;
	return result;
}


color operator - (color const &p_first, color const &p_second)
{
	color result = p_first;
	result -= p_second;
	return result;
}


color operator * (color const &p_color, int const p_value)
{
	color result = p_color;
	result *= p_value;
	return result;
}


color operator / (color const &p_color, int const p_value)
{
	color result = p_color;
	result /= p_value;
	return result;
}


bool operator < (color const &p_first, color const &p_second)
{
	return p_first.m_rgb_values < p_second.m_rgb_values;
}


bool operator == (color const &p_first, color const &p_second)
{
	return p_first.m_rgb_values == p_second.m_rgb_values;
}


bool operator != (color const &p_first, color const &p_second)
{
	return p_first.m_rgb_values != p_second.m_rgb_values;
}


std::string to_string(color const &p_color)
{
	return       std::to_string(p_color.m_rgb_values[0])
	     + "," + std::to_string(p_color.m_rgb_values[1])
	     + "," + std::to_string(p_color.m_rgb_values[2]);
}


long calculate_color_distance(color const &p_first, color const &p_second)
{
	// Adapted from https://www.compuphase.com/cmetric.htm,
	// "A low-cost approximation". sqrt() omitted, since we need
	// the distance only for comparisons and for range searches.

	long r1 = p_first.m_rgb_values[0];
	long g1 = p_first.m_rgb_values[1];
	long b1 = p_first.m_rgb_values[2];
	long r2 = p_second.m_rgb_values[0];
	long g2 = p_second.m_rgb_values[1];
	long b2 = p_second.m_rgb_values[2];

	long diff_r = r1 - r2;
	long diff_g = g1 - g2;
	long diff_b = b1 - b2;

	long r_mean = (r1 + r2) / 2;

	return (((512 + r_mean) * diff_r*diff_r) >> 8) + 4 * diff_g*diff_g + (((512 + 255 - r_mean) * diff_b*diff_b) >> 8);
}




void compute_color_histogram(
	color_histogram &p_color_histogram,
	const_pixmap_view_t p_input_pixmap,
	base::progress_report_callback const &p_progress_report_callback
)
{
	unsigned long num_pixels_processed = 0;
	unsigned long total_num_pixels = p_input_pixmap.m_width * p_input_pixmap.m_height;

	for (std::size_t y = 0; y < p_input_pixmap.m_height; ++y)
	{
		for (std::size_t x = 0; x < p_input_pixmap.m_width; ++x)
		{
			std::uint8_t const *pixel_data = at(p_input_pixmap, x, y);

			color pixel_color(pixel_data[2], pixel_data[1], pixel_data[0]);

			auto iter = p_color_histogram.find(pixel_color);
			if (iter == p_color_histogram.end())
				p_color_histogram.emplace(std::move(pixel_color), 1ul);
			else
				iter->second++;

			++num_pixels_processed;
			if (p_progress_report_callback)
				p_progress_report_callback(num_pixels_processed, total_num_pixels);
		}
	}
}


} // namespace graphics end
