#include <assert.h>
#include <FreeImage.h>
#include <boost/program_options.hpp>
#include "fmt/format.h"
#include "fmt/ostream.h"
#include "base/scope_guard.hpp"
#include "graphics/fi_pixmap.hpp"
#include "context.hpp"


void display_help(boost::program_options::options_description const &p_allowed_progopts)
{
	fmt::print(stderr, "Usage: options_description [options]\n");
	fmt::print(stderr, "{}", p_allowed_progopts);
}


int main(int argc, char *argv[])
{
	context ctx;


	// Setup program options.

	bool help = false;
	bool use_dithering = false;
	std::string input_filename;
	std::string output_filename;

	boost::program_options::options_description allowed_progopts("Options");
	allowed_progopts.add_options()
		("help,h", boost::program_options::bool_switch(&help), "produce help message")
		("input,i", boost::program_options::value < std::string > (&input_filename), "input image file to color-quantize")
		("output,o", boost::program_options::value < std::string > (&output_filename), "color-quantized output image file")
		("use-dithering,d", boost::program_options::bool_switch(&use_dithering), "use dithering when quantizing the image")
		;

	add_program_options(allowed_progopts);


	// Read options from the command line
	try
	{
		boost::program_options::variables_map progopts_varmap;
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, allowed_progopts), progopts_varmap);
		boost::program_options::notify(progopts_varmap);
	}
	catch (boost::program_options::invalid_command_line_syntax const &p_invalid_command_line_syntax)
	{
		fmt::print(stderr, "{}\n", p_invalid_command_line_syntax.what());
		display_help(allowed_progopts);
		return -1;
	}
	catch (boost::program_options::unknown_option const &p_unknown_option)
	{
		fmt::print(stderr, "{}\n", p_unknown_option.what());
		display_help(allowed_progopts);
		return -1;
	}
	catch (boost::program_options::error const &p_error)
	{
		fmt::print(stderr, "{}\n", p_error.what());
		return -1;
	}

	// If the help switch is present, display help text, then exit.
	if (help)
	{
		display_help(allowed_progopts);
		return -1;
	}

	// Do some sanity checks on the command line arguments.
	if (input_filename.empty())
	{
		fmt::print(stderr, "Need an input filename\n");
		return -1;
	}

	if (output_filename.empty())
	{
		fmt::print(stderr, "Need an output filename\n");
		return -1;
	}


	try
	{
		// Set up color quantization.
		if (!setup_color_quantization(ctx))
			return -1;


		// Setup FreeImage.

		auto freeimage_guard = base::make_scope_guard([]() {
			FreeImage_DeInitialise();
		});

		FreeImage_Initialise();


		// Get input image.

		FREE_IMAGE_FORMAT input_format = FreeImage_GetFileType(input_filename.c_str());
		if (input_format == FIF_UNKNOWN)
		{
			input_format = FreeImage_GetFIFFromFilename(input_filename.c_str());
			if (input_format == FIF_UNKNOWN)
			{
				fmt::print(stderr, "Input image cannot be found, cannot be read, or has unknown file format\n");
				return -1;
			}
		}

		graphics::fi_pixmap original_input_image = FreeImage_Load(input_format, input_filename.c_str(), 0);
		if (original_input_image.get_fibitmap() == nullptr)
		{
			fmt::print(stderr, "Could not load input image file \"{}\"\n", input_filename);
			return -1;
		}

		// Convert the image to 24-bit RGB.

		graphics::fi_pixmap converted_input_image = FreeImage_ConvertTo24Bits(original_input_image.get_fibitmap());
		if (converted_input_image.get_fibitmap() == nullptr)
		{
			fmt::print(stderr, "Could not convert input image file \"{}\" to 24 bit\n", input_filename);
			return -1;
		}

		// We no longer need the original input image, so get rid of it to save some resouces.
		original_input_image = graphics::fi_pixmap();

		ctx.m_input_image = make_pixmap_view(converted_input_image);


		graphics::fi_pixmap output_image = FreeImage_Allocate(graphics::width(ctx.m_input_image), graphics::height(ctx.m_input_image), 8);
		if (output_image.get_fibitmap() == nullptr)
		{
			fmt::print(stderr, "Could not allocate output image\n");
			return -1;
		}

		ctx.m_output_image = make_pixmap_view(output_image);

		ctx.m_use_dithering = use_dithering;


		fmt::print(stderr, "Input image: \"{}\"\n", input_filename);
		fmt::print(stderr, "Output image: \"{}\"\n", output_filename);
		fmt::print(stderr, "Image size: {} x {}\n", graphics::width(ctx.m_input_image), graphics::height(ctx.m_input_image));
		fmt::print(stderr, "Dithering: {}\n", use_dithering ? "yes" : "no");


		if (!apply_color_quantization(ctx))
			return -1;


		{
			RGBQUAD *fb_palette = FreeImage_GetPalette(output_image.get_fibitmap());

			for (std::size_t i = 0; i < ctx.m_palette.size(); ++i)
			{
				graphics::color const & palette_entry = ctx.m_palette[i];
				fmt::print(stderr, "Palette index # {}: {}\n", i, to_string(palette_entry));
				fb_palette[i].rgbRed   = std::min(std::max(int(palette_entry[0]), 0), 255);
				fb_palette[i].rgbGreen = std::min(std::max(int(palette_entry[1]), 0), 255);
				fb_palette[i].rgbBlue  = std::min(std::max(int(palette_entry[2]), 0), 255);
			}
		}


		if (!FreeImage_Save(FIF_GIF, output_image.get_fibitmap(), output_filename.c_str(), 0))
		{
			fmt::print(stderr, "Could not save output image to \"{}\"\n", output_filename);
			return -1;
		}


		// Cleanup.
		teardown_color_quantization(ctx);
	} 
	catch (std::exception const &p_exception)
	{
		fmt::print(stderr, "Exception caught: {}\n", p_exception.what());
		return -1;
	}


	return 0;
}
