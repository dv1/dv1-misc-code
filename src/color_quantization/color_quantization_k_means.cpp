#include "fmt/format.h"
#include "context.hpp"
#include "palettized_output.hpp"


// This implementation of k-means based color quantization implements
// optimizations described in the paper "Improving the performance of
// k-means for color quantization" by M. Emre Celebi. Link:
// https://doi.org/10.1016/j.imavis.2010.10.002


namespace
{


std::size_t palette_size;


} // unnamed namespace end


void add_program_options(boost::program_options::options_description &p_options_description)
{
	p_options_description.add_options()
		("palette-size,p", boost::program_options::value < std::size_t > (&palette_size)->default_value(256), "Palette size (valid range: 2-256)")
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

	fmt::print(stderr, "Palette size: {} colors\n", palette_size);

	p_context.m_palette = graphics::palette{palette_size, graphics::color{0, 0, 0}};

	return true;
}


void teardown_color_quantization(context &)
{
}


bool apply_color_quantization(context &p_context)
{
	std::vector < graphics::color > unique_input_colors;
	std::vector < double > color_weights;
	std::vector < std::size_t > unique_input_colors_nearest_palette_indices;
	int prev_progress_percent;


	// Initialize the unique_input_colors and
	// unique_input_colors_nearest_palette_indices vectors.

	{
		graphics::color_histogram temp_color_histogram;
		prev_progress_percent = -1;

		compute_color_histogram(
			temp_color_histogram,
			p_context.m_input_image,
			base::make_ostream_progress_report(std::cerr, "Scanning image pixels", std::chrono::milliseconds{50})
		);
		if (prev_progress_percent != -1)
			fmt::print(stderr, "\n");

		unique_input_colors.resize(temp_color_histogram.size());
		color_weights.resize(temp_color_histogram.size());
		unique_input_colors_nearest_palette_indices.resize(temp_color_histogram.size());

		unsigned long total_num_pixels = graphics::width(p_context.m_input_image) * graphics::height(p_context.m_input_image);

		std::size_t i = 0;
		for (auto iter = temp_color_histogram.begin(); iter != temp_color_histogram.end(); ++i, ++iter)
		{
			unique_input_colors[i] = iter->first;
			color_weights[i] = double(iter->second) / total_num_pixels;
		}

		fmt::print(stderr, "\n");
		fmt::print(stderr, "{} source pixel entries\n", unique_input_colors.size());
	}


	// Set up an initial palette.

	prev_progress_percent = -1;
	for (std::size_t i = 0; i < palette_size; ++i)
	{
		int progress_percent = ((i + 1) * 100) / palette_size;
		if (progress_percent != prev_progress_percent)
		{
			fmt::print(stderr, "Setting up initial palette: {}%\r", progress_percent);
			prev_progress_percent = progress_percent;
		}

		auto iter = unique_input_colors.begin() + i * unique_input_colors.size() / palette_size;

		p_context.m_palette[i] = *iter;
	}
	fmt::print(stderr, "\n");

	for (std::size_t i = 0; i < unique_input_colors.size(); ++i)
	{
		std::size_t nearest_palette_index = find_nearest_color(p_context.m_palette, unique_input_colors[i]);
		unique_input_colors_nearest_palette_indices[i] = nearest_palette_index;
	}


	fmt::print(stderr, "Beginning color quantization iterations\n");

	long min_max_distance = -1;
	std::vector < long > distance_matrix(palette_size * palette_size);
	std::vector < std::size_t > permutation_matrix(palette_size * palette_size);
	std::vector < double > sum_palette(palette_size*3, 0.0);
	std::vector < double > sum_weights(palette_size, 0.0);
	graphics::palette new_palette{palette_size, graphics::color{0, 0, 0}};

	for (unsigned int iteration = 0; iteration < 100; ++iteration)
	{
		graphics::palette &cur_palette = p_context.m_palette;
		long max_distance = -1.0f;

		for (unsigned int i = 0; i < palette_size; ++i)
		{
			distance_matrix[i + i*palette_size] = 0;
			for (unsigned int j = i + 1; j < palette_size; ++j)
			{
				distance_matrix[i + j*palette_size] = distance_matrix[j + i*palette_size] = calculate_color_distance(cur_palette[i], cur_palette[j]);
			}
		}

		for (unsigned int i = 0; i < palette_size; ++i)
		{
			for (unsigned int j = 0; j < palette_size; ++j)
				permutation_matrix[j + i*palette_size] = j;

			std::sort(
				&(permutation_matrix[0 + i*palette_size]), &(permutation_matrix[palette_size + i*palette_size]),
				[&](std::size_t p_first, std::size_t p_second) {
					return distance_matrix[p_first + i*palette_size] < distance_matrix[p_second + i*palette_size];
				}
			);
		}

		for (std::size_t i = 0; i < unique_input_colors.size(); ++i)
		{
			std::size_t palette_index = unique_input_colors_nearest_palette_indices[i];

			long min_distance, prev_distance;
			min_distance = prev_distance = calculate_color_distance(unique_input_colors[i], cur_palette[palette_index]);

			for (std::size_t j = 1; j < palette_size; ++j)
			{
				std::size_t t = permutation_matrix[j + palette_index*palette_size];
				if (distance_matrix[t + palette_index*palette_size] >= (4 * prev_distance))
					break;

				long distance = calculate_color_distance(unique_input_colors[i], cur_palette[t]);

				if (distance <= min_distance)
				{
					min_distance = distance;
					unique_input_colors_nearest_palette_indices[i] = t;
				}
			}

			if (max_distance < 0)
				max_distance = min_distance;
			else
				max_distance = std::max(max_distance, min_distance);
		}

		std::fill(begin(sum_palette), end(sum_palette), 0.0);
		std::fill(begin(sum_weights), end(sum_weights), 0.0);
		std::fill(begin(new_palette), end(new_palette), graphics::color{0, 0, 0});


		{
			std::size_t palette_index = unique_input_colors_nearest_palette_indices[0];
			for (int c = 0; c < 3; ++c)
				sum_palette[palette_index*3 + c] = unique_input_colors[0][c] * color_weights[0];
			sum_weights[palette_index] = color_weights[0];
		}

		for (std::size_t i = 1; i < unique_input_colors.size(); ++i)
		{
			std::size_t palette_index = unique_input_colors_nearest_palette_indices[i];
			for (int c = 0; c < 3; ++c)
				sum_palette[palette_index*3 + c] += unique_input_colors[i][c] * color_weights[i];
			sum_weights[palette_index] += color_weights[i];
		}

		for (unsigned int k = 0; k < palette_size; ++k)
		{
			for (int c = 0; c < 3; ++c)
				new_palette[k][c] = int(sum_palette[k*3 + c] / sum_weights[k]);
		}

		fmt::print(stderr, "Iteration #{}: max distance {}\n", iteration, max_distance);

		if (min_max_distance >= 0)
		{
			if (iteration > 30)
			{
				if (max_distance > min_max_distance)
					break;
				else if ((min_max_distance - max_distance) < 5)
					break;
			}

			min_max_distance = max_distance;
		}
		else
			min_max_distance = max_distance;

		cur_palette = new_palette;
	}


	prev_progress_percent = -1;
	produce_palettized_output(
		p_context,
		base::make_ostream_progress_report(std::cerr, "Determining pixels of output image", std::chrono::milliseconds{50})
	);
	fmt::print(stderr, "\n");


	return true;
}
