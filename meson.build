project('color-quantization-experiments', ['c', 'cpp'], default_options: ['c_std=c11', 'cpp_std=c++17'])

cpp = meson.get_compiler('cpp')

boost_dep = dependency('boost', modules : ['program_options'])
freeimage_dep = cpp.find_library('freeimage', required: false)
sndfile_dep = dependency('sndfile', required: false)

common_incdirs = include_directories(['src/libs', 'external/fmtlib/include', 'external/span-lite/include'])

base_lib = static_library(
	'base_lib',
	[
		'src/libs/base/progress_report.cpp',
		'external/fmtlib/src/format.cc',
		'external/fmtlib/src/posix.cc'
	],
	include_directories: common_incdirs
)

if freeimage_dep.found()
	graphics_lib = static_library(
		'graphics_lib',
		[
			'src/libs/graphics/color.cpp',
			'src/libs/graphics/fi_pixmap.cpp',
			'src/libs/graphics/palette.cpp'
		],
		include_directories: common_incdirs,
		dependencies: [freeimage_dep]
	)

	color_quantization_common_lib = static_library(
		'color_quantization_common',
		[
			'src/color_quantization/main.cpp',
			'src/color_quantization/palettized_output.cpp'
		],
		dependencies: [boost_dep, freeimage_dep],
		link_with: [base_lib, graphics_lib],
		include_directories: common_incdirs
	)
	executable(
		'color_quantization_k_means',
		'src/color_quantization/color_quantization_k_means.cpp',
		dependencies: [boost_dep, freeimage_dep],
		link_with: [color_quantization_common_lib],
		include_directories: common_incdirs
	)
	executable(
		'color_quantization_median_cut',
		'src/color_quantization/color_quantization_median_cut.cpp',
		dependencies: [boost_dep, freeimage_dep],
		link_with: [color_quantization_common_lib],
		include_directories: common_incdirs
	)
	executable(
		'color_quantization_octree',
		'src/color_quantization/color_quantization_octree.cpp',
		dependencies: [boost_dep, freeimage_dep],
		link_with: [color_quantization_common_lib],
		include_directories: common_incdirs
	)
endif
