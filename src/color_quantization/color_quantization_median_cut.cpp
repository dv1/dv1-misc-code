#include <vector>
#include <algorithm>
#include "fmt/format.h"
#include "context.hpp"
#include "palettized_output.hpp"
#include "base/numeric.hpp"


namespace
{


std::size_t palette_size;
bool use_median_cut_for_nearest_color = false;
unsigned int num_levels;

struct median_cut_entry
{
	graphics::color m_color;
	int m_rgb_component_index;
	int m_rgb_component_value;
	int m_palette_index;
};

typedef std::vector < median_cut_entry > median_cut_vector;


int find_largest_rgb_component_index(median_cut_vector::iterator p_begin, median_cut_vector::iterator p_end)
{
	graphics::color min_rgb, max_rgb;
	for (auto iter = p_begin; iter != p_end; ++iter)
	{
		if (iter != p_begin)
		{
			for (int i = 0; i < 3; ++i)
			{
				min_rgb[i] = std::min(min_rgb[i], iter->m_color[i]);
				max_rgb[i] = std::max(max_rgb[i], iter->m_color[i]);
			}
		}
		else
			min_rgb = max_rgb = iter->m_color;
	}

	int largest_range = -1;
	int largest_rgb_component_idx = 0;
	for (int i = 0; i < 3; ++i)
	{
		int range = max_rgb[i] - min_rgb[i];
		if (range > largest_range)
		{
			largest_range = range;
			largest_rgb_component_idx = i;
		}
	}

	return largest_rgb_component_idx;
}


void perform_median_cut(context &p_context, graphics::palette::iterator &p_palette_iter, median_cut_vector::iterator p_begin, median_cut_vector::iterator p_end, unsigned int p_level)
{
	if (p_level == num_levels)
	{
		assert(p_palette_iter != end(p_context.m_palette));

		std::size_t palette_index = std::distance(begin(p_context.m_palette), p_palette_iter);

		graphics::color accumulated_colors { 0, 0, 0 };
		for (auto iter = p_begin; iter != p_end; ++iter)
		{
			accumulated_colors += iter->m_color;
			iter->m_palette_index = palette_index;
		}
		accumulated_colors /= std::distance(p_begin, p_end);

		*p_palette_iter = std::move(accumulated_colors);
		p_palette_iter++;
	}
	else
	{
		int largest_rgb_component_idx = find_largest_rgb_component_index(p_begin, p_end);

		std::sort(
			p_begin, p_end,
			[largest_rgb_component_idx](median_cut_entry const &p_first, median_cut_entry const &p_second) -> bool {
				return p_first.m_color[largest_rgb_component_idx] < p_second.m_color[largest_rgb_component_idx];
			}
		);

		std::size_t num_values = p_end - p_begin;
		auto median_value_iter = (p_begin + num_values / 2);
		int rgb_component_value = median_value_iter->m_color[largest_rgb_component_idx];

		perform_median_cut(p_context, p_palette_iter, p_begin, median_value_iter, p_level + 1);
		perform_median_cut(p_context, p_palette_iter, median_value_iter, p_end, p_level + 1);

		median_value_iter->m_rgb_component_index = largest_rgb_component_idx;
		median_value_iter->m_rgb_component_value = rgb_component_value;
	}
}


void perform_median_cut(context &p_context, median_cut_vector::iterator p_begin, median_cut_vector::iterator p_end)
{
	graphics::palette::iterator palette_iter = begin(p_context.m_palette);
	perform_median_cut(p_context, palette_iter, std::move(p_begin), std::move(p_end), 0);
}


median_cut_vector::iterator find_nearest_color(median_cut_vector::iterator p_begin, median_cut_vector::iterator p_end, graphics::color const &p_color, unsigned int p_level = 0)
{
	if (p_level == num_levels)
	{
		return p_begin;
	}
	else
	{
		std::size_t num_values = p_end - p_begin;
		auto median_value_iter = (p_begin + num_values / 2);

		if (p_color[median_value_iter->m_rgb_component_index] < median_value_iter->m_rgb_component_value)
			return find_nearest_color(p_begin, median_value_iter, p_color, p_level + 1);
		else
			return find_nearest_color(median_value_iter, p_end, p_color, p_level + 1);
	}
}


} // unnamed namespace end


void add_program_options(boost::program_options::options_description &p_options_description)
{
	p_options_description.add_options()
		("palette-size,p", boost::program_options::value < std::size_t > (&palette_size)->default_value(256), "Palette size (valid range: 2-256)")
		("use-median-cut-for-nearest-color,m", boost::program_options::bool_switch(&use_median_cut_for_nearest_color), "Reuse median-cut partitioning for determining nearest color (faster, but less accurate color matching than default method)")
		;
}


std::size_t get_palette_size(context const &)
{
	return palette_size;
}


bool setup_color_quantization(context &p_context)
{
	if ((palette_size < 2) || (palette_size > 256))
	{
		fmt::print(stderr, "Invalid palette size {}; valid range is 2-256\n", palette_size);
		return false;
	}

	if ((palette_size & (palette_size - 1)) != 0)
	{
		fmt::print(stderr, "Invalid palette size {}; must be a power-of-two\n", palette_size);
		return false;
	}

	fmt::print(stderr, "Palette size: {} colors\n", palette_size);
	fmt::print(stderr, "Reusing median-cut partitioning for faster (but less accurate) color matching: {}\n", use_median_cut_for_nearest_color ? "yes" : "no");

	num_levels = base::calculate_num_significant_bits(palette_size) - 1;

	p_context.m_palette = graphics::palette{palette_size, graphics::color{0, 0, 0}};

	return true;
}


void teardown_color_quantization(context &)
{
}


bool apply_color_quantization(context &p_context)
{
	median_cut_vector unique_input_colors;
	int prev_progress_percent;

	{
		graphics::color_histogram temp_color_histogram;
		prev_progress_percent = -1;

		compute_color_histogram(
			temp_color_histogram,
			p_context.m_input_image,
			base::make_ostream_progress_report(std::cerr, "Scanning image pixels", std::chrono::milliseconds{50})
		);
		if (prev_progress_percent != -1)
			fmt::print("\n");

		unique_input_colors.resize(temp_color_histogram.size());

		std::transform(
			temp_color_histogram.begin(), temp_color_histogram.end(),
			unique_input_colors.begin(),
			[](graphics::color_histogram::value_type const &p_histogram_value) -> median_cut_entry { return median_cut_entry { p_histogram_value.first, 0, 0, 0 }; }
		);
	}

	perform_median_cut(p_context, unique_input_colors.begin(), unique_input_colors.end());


	std::function < std::size_t(graphics::color const &p_color) > find_nearest_color_func;
	if (use_median_cut_for_nearest_color)
	{
		find_nearest_color_func = [&p_context, &unique_input_colors](graphics::color const &p_color) -> std::size_t {
			auto iter = find_nearest_color(unique_input_colors.begin(), unique_input_colors.end(), p_color);
			return iter->m_palette_index;
		};
	}


	prev_progress_percent = -1;
	produce_palettized_output(
		p_context,
		base::make_ostream_progress_report(std::cerr, "Determining pixels of output image", std::chrono::milliseconds{50}),
		std::move(find_nearest_color_func)
	);
	fmt::print(stderr, "\n");


	return true;
}
