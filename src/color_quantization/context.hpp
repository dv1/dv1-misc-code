#ifndef COLOR_QUANTIZATION_CONTEXT_HPP______
#define COLOR_QUANTIZATION_CONTEXT_HPP______

#include <vector>
#include <cstddef>
#include <cstdint>
#include <boost/program_options.hpp>
#include "graphics/pixmap_view.hpp"
#include "graphics/palette.hpp"


struct context
{
	graphics::nonconst_pixmap_view_t m_input_image;
	graphics::nonconst_pixmap_view_t m_output_image;

	bool m_use_dithering;

	graphics::palette m_palette;
};


void add_program_options(boost::program_options::options_description &p_options_description);
bool setup_color_quantization(context &p_context);
void teardown_color_quantization(context &p_context);
bool apply_color_quantization(context &p_context);


#endif // COLOR_QUANTIZATION_CONTEXT_HPP______
