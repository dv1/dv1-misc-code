#ifndef COLOR_QUANTIZATION_PALETTIZED_OUTPUT_HPP
#define COLOR_QUANTIZATION_PALETTIZED_OUTPUT_HPP

#include "graphics/palette.hpp"
#include "graphics/pixmap_view.hpp"
#include "context.hpp"


void produce_palettized_output(
	context &p_context,
	base::progress_report_callback const &p_progress_report_callback = base::progress_report_callback(),
	std::function < std::size_t(graphics::color const &p_color) > p_find_nearest_color_callback = std::function < std::size_t(graphics::color const &p_color) > ()
);


#endif // COLOR_QUANTIZATION_PALETTIZED_OUTPUT_HPP
