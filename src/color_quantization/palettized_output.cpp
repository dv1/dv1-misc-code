#include "base/kd_tree.hpp"
#include "palettized_output.hpp"


void produce_palettized_output(
	context &p_context,
	base::progress_report_callback const &p_progress_report_callback,
	std::function < std::size_t(graphics::color const &p_color) > p_find_nearest_color_callback
)
{
	graphics::palette &output_palette = p_context.m_palette;

	std::vector < std::size_t > palette_indices(output_palette.size());
	for (int i = 0; i < int(palette_indices.size()); ++i)
		palette_indices[i] = i;

	base::kd_tree < std::size_t > palette_kd_tree;
	fill(
		palette_kd_tree,
		palette_indices.begin(), palette_indices.end(),
		[&output_palette](std::size_t p_first, std::size_t p_second, unsigned int p_level) -> bool {
			unsigned int dimension = p_level % 3;
			return output_palette[p_first][dimension] < output_palette[p_second][dimension];
		}
	);

	if (!p_find_nearest_color_callback)
	{
		p_find_nearest_color_callback = [&output_palette, &palette_kd_tree](graphics::color const &p_color) -> std::size_t {
			auto nearest_iter = find_nearest(
				palette_kd_tree,
				p_color,
				[&output_palette](std::size_t output_palette_index, graphics::color const &p_color) -> long {
					graphics::color const &palette_color = output_palette[output_palette_index];
					return calculate_color_distance(palette_color, p_color);
				},
				[&output_palette](std::size_t output_palette_index, graphics::color const &p_color, unsigned int p_level) -> long {
					graphics::color const &palette_color = output_palette[output_palette_index];
					unsigned int dimension = p_level % 3;
					graphics::color color1 { 0, 0, 0 };
					graphics::color color2 { 0, 0, 0 };
					color1[dimension] = palette_color[dimension];
					color2[dimension] = p_color[dimension];

					long sign = (color2[dimension] >= color1[dimension]) ? 1.0f : -1.0f;
					return calculate_color_distance(color1, color2) * sign;
				}
			);
			return nearest_iter->m_value;
		};
	}

	std::size_t width = graphics::width(p_context.m_input_image);
	std::size_t height = graphics::height(p_context.m_input_image);
	unsigned long num_pixels_processed = 0;
	unsigned long total_num_pixels = width * height;

	for (unsigned long y = 0; y < height; ++y)
	{
		for (unsigned long x = 0; x < width; ++x)
		{
			std::uint8_t const *pixel_data = graphics::at(p_context.m_input_image, x, y);

			graphics::color pixel_color(pixel_data[2], pixel_data[1], pixel_data[0]);

			std::size_t nearest_palette_index = p_find_nearest_color_callback(pixel_color);
			graphics::color const & nearest_palette_color = output_palette[nearest_palette_index];
			graphics::at < std::uint8_t > (p_context.m_output_image, x, y)[0] = nearest_palette_index;

			if (p_context.m_use_dithering)
			{
				graphics::color quantization_error = pixel_color - nearest_palette_color;

				int const chroma_weights[3] = { 299, 587, 114 };

				long const floyd_steinberg_x_offset[4] = { +1, -1,  0, +1 };
				long const floyd_steinberg_y_offset[4] = {  0, +1, +1, +1 };
				int const floyd_steinberg_weight[4] = { 7, 3, 5, 1 };
				int const floyd_steinberg_total_weight = 16;

				for (int idx = 0; idx < 4; ++idx)
				{
					if ((floyd_steinberg_x_offset[idx] < 0) && (x == 0))
						continue;
					if ((floyd_steinberg_x_offset[idx] > 0) && (x == (width - 1)))
						continue;
					if ((floyd_steinberg_y_offset[idx] < 0) && (y == 0))
						continue;
					if ((floyd_steinberg_y_offset[idx] > 0) && (y == (height - 1)))
						continue;

					unsigned long x_with_offset = x + floyd_steinberg_x_offset[idx];
					unsigned long y_with_offset = y + floyd_steinberg_y_offset[idx];

					for (int rgb_idx = 0; rgb_idx < 3; ++rgb_idx)
					{
						int rgb_value = graphics::at(p_context.m_input_image, x_with_offset, y_with_offset)[rgb_idx];
						rgb_value += quantization_error[rgb_idx] * floyd_steinberg_weight[idx] * chroma_weights[rgb_idx] / floyd_steinberg_total_weight	 / 1000;
						rgb_value = std::max(std::min(rgb_value, 255), 0);
						graphics::at(p_context.m_input_image, x_with_offset, y_with_offset)[rgb_idx] = rgb_value;
					}
				}
			}

			++num_pixels_processed;
			if (p_progress_report_callback)
				p_progress_report_callback(num_pixels_processed, total_num_pixels);
		}
	}
}
