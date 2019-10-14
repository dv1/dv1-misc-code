#include <set>
#include <vector>
#include <algorithm>
#include "fmt/format.h"
#include "context.hpp"
#include "palettized_output.hpp"
#include "base/numeric.hpp"


namespace
{


std::size_t palette_size;


struct octree
{
	struct node
	{
		std::size_t m_num_references;
		graphics::color m_color;
		bool m_occupied;
		bool m_is_leaf;
		unsigned int m_level;

		node()
			: m_num_references(0)
			, m_color{0, 0, 0}
			, m_occupied(false)
			, m_is_leaf(false)
			, m_level(0)
		{
		}
	};

	typedef std::vector < node > nodes;
	nodes m_nodes;


	typedef std::set < std::size_t > node_array_indices;
	node_array_indices m_leaves;
	node_array_indices m_nonleaf_nodes;
};


void alloc_node(octree &p_octree, std::size_t p_array_index)
{
	if (p_array_index >= p_octree.m_nodes.size())
		p_octree.m_nodes.resize(p_array_index + 1);
}


// TODO: Handle edge cases where there are tree branches
// with lots of 1-child nodes (to avoid having to reduce
// these all the time). Perhaps add some sort of additional
// "shortcut" array index that is valid until a node is
// visited again.


void insert_color(octree &p_octree, std::size_t p_array_index, graphics::color const &p_color, std::size_t const p_color_weight, unsigned int p_level)
{
	alloc_node(p_octree, p_array_index);

	octree::node &node = p_octree.m_nodes[p_array_index];
	node.m_occupied = true;
	node.m_level = p_level;

	if (p_level == 8)
	{
		p_octree.m_leaves.insert(p_array_index);
		node.m_is_leaf = true;
		node.m_num_references = p_color_weight;
		node.m_color = p_color * p_color_weight;
		return;
	}

	node.m_num_references += p_color_weight;
	node.m_color += p_color * p_color_weight;

	p_octree.m_nonleaf_nodes.insert(p_array_index);

	std::size_t inv_level = 7 - p_level;

	std::size_t child_index = (((p_color[0] >> inv_level) & 0x1) << 2)
	                        | (((p_color[1] >> inv_level) & 0x1) << 1)
	                        | (((p_color[2] >> inv_level) & 0x1) << 0);

	std::size_t child_array_index = 8 * p_array_index + (1 + child_index);
	insert_color(p_octree, child_array_index, p_color, p_color_weight, p_level + 1);
}


void reduce_node(octree &p_octree, std::size_t p_array_index)
{
	octree::node &node = p_octree.m_nodes[p_array_index];
	assert(node.m_occupied);
	assert(!node.m_is_leaf);

	for (std::size_t child_index = 0; child_index < 8; ++child_index)
	{
		std::size_t child_array_index = 8 * p_array_index + (1 + child_index);
		octree::node &child_node = p_octree.m_nodes[child_array_index];

		if (!child_node.m_occupied || !child_node.m_is_leaf)
			continue;

		child_node.m_occupied = false;

		p_octree.m_leaves.erase(child_array_index);
	}

	node.m_is_leaf = true;
	p_octree.m_leaves.insert(p_array_index);
}


void reduce_tree(octree &p_octree)
{
	std::vector < std::size_t > node_indices(p_octree.m_nonleaf_nodes.size());
	std::copy(p_octree.m_nonleaf_nodes.begin(), p_octree.m_nonleaf_nodes.end(), node_indices.begin());

	std::sort(node_indices.begin(), node_indices.end(),
		[&p_octree](std::size_t p_first, std::size_t p_second) -> bool {
			octree::node &first_node = p_octree.m_nodes[p_first];
			octree::node &second_node = p_octree.m_nodes[p_second];

			if (first_node.m_level > second_node.m_level)
				return true;
			else if (first_node.m_level < second_node.m_level)
				return false;

			return (first_node.m_num_references < second_node.m_num_references);
		}
	);

	{
		auto progress_report = base::make_ostream_progress_report(std::cerr, "Reducing trivial nodes", std::chrono::milliseconds{50});

		std::size_t initial_num_nodes = node_indices.size();
		std::size_t num_reduced_trivial_nodes = 0;
		for (auto iter = node_indices.begin(); iter != node_indices.end();)
		{
			std::size_t array_index = *iter;
			octree::node &node = p_octree.m_nodes[array_index];

			if (!node.m_occupied || node.m_is_leaf || (node.m_num_references != 1) || (array_index == 0))
			{
				++iter;
				continue;
			}

			reduce_node(p_octree, array_index);
			iter = node_indices.erase(iter);

			++num_reduced_trivial_nodes;

			progress_report(initial_num_nodes - (node_indices.end() - iter), initial_num_nodes);
		}
		fmt::print(stderr, "{} trivial nodes reduced\n", num_reduced_trivial_nodes);
	}

	std::chrono::steady_clock::time_point last_report_time_point;
	while (p_octree.m_leaves.size() > palette_size)
	{
		reduce_node(p_octree, *(node_indices.begin()));
		node_indices.erase(node_indices.begin());

		auto now = std::chrono::steady_clock::now();
		auto time_since_last_report = now - last_report_time_point;

		if ((time_since_last_report >= std::chrono::milliseconds{50}))
		{
			fmt::print(stderr, "remaining non-leaf nodes: {} remaining leaves: {}\n", node_indices.size(), p_octree.m_leaves.size());
			last_report_time_point = now;
		}
	}

	fmt::print(stderr, "remaining non-leaf nodes: {} remaining leaves: {}\n", node_indices.size(), p_octree.m_leaves.size());
}


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
	octree color_octree;
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

		for (auto const &histogram_entry : temp_color_histogram)
			insert_color(color_octree, 0, histogram_entry.first, histogram_entry.second, 0);

		fmt::print(stderr, "\n");
		fmt::print(stderr, "{} source pixel entries\n", temp_color_histogram.size());
		fmt::print(stderr, "{} non-leaf octree nodes\n", color_octree.m_nonleaf_nodes.size());
		fmt::print(stderr, "{} octree leaves\n", color_octree.m_leaves.size());
	}


	reduce_tree(color_octree);


	std::size_t i = 0;
	for (std::size_t array_index : color_octree.m_leaves)
	{
		octree::node const &leaf = color_octree.m_nodes[array_index];

		if (leaf.m_num_references > 0)
		{
			p_context.m_palette[i] = leaf.m_color / leaf.m_num_references;
			++i;
		}
	}


	prev_progress_percent = -1;
	produce_palettized_output(
		p_context,
		base::make_ostream_progress_report(std::cerr, "Determining pixels of output image", std::chrono::milliseconds{50})
	);
	fmt::print(stderr, "\n");


	return true;
}
