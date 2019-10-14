#ifndef GRAPHICS_PIXMAP_VIEW_HPP_______
#define GRAPHICS_PIXMAP_VIEW_HPP_______

#include <cstddef>
#include <type_traits>
#include "base/custom_span.hpp"


namespace graphics
{


// TODO: Limit PixelData to uint8_t* and uint8_t const *


template < typename PixelData, typename Enable = void >
struct pixmap_view;

template < typename PixelData >
struct pixmap_view < PixelData, typename std::enable_if < std::is_const < PixelData > ::value > ::type >
{
	nonstd::span < PixelData > m_data;
	std::size_t m_width, m_height, m_hstride, m_num_channels;

	pixmap_view()
	{
	}

	pixmap_view(nonstd::span < PixelData > p_data, std::size_t p_width, std::size_t p_height, std::size_t p_hstride, std::size_t p_num_channels)
		: m_data(std::move(p_data))
		, m_width(p_width)
		, m_height(p_height)
		, m_hstride(p_hstride)
		, m_num_channels(p_num_channels)
	{
	}

	pixmap_view(pixmap_view < typename std::remove_const < PixelData > ::type > p_other)
	{
		m_data = std::move(p_other.m_data);
		m_width = p_other.m_width;
		m_height = p_other.m_height;
		m_hstride = p_other.m_hstride;
		m_num_channels = p_other.m_num_channels;
	}

	pixmap_view& operator = (pixmap_view < typename std::remove_const < PixelData > ::type > p_other)
	{
		m_data = std::move(p_other.m_data);
		m_width = p_other.m_width;
		m_height = p_other.m_height;
		m_hstride = p_other.m_hstride;
		m_num_channels = p_other.m_num_channels;

		return *this;
	}
};

template < typename PixelData >
struct pixmap_view < PixelData, typename std::enable_if < ! std::is_const < PixelData > ::value > ::type >
{
	nonstd::span < PixelData > m_data;
	std::size_t m_width, m_height, m_hstride, m_num_channels;
};

typedef pixmap_view < std::uint8_t const > const_pixmap_view_t;
typedef pixmap_view < std::uint8_t > nonconst_pixmap_view_t;


template < typename PixelData >
inline std::size_t width(pixmap_view < PixelData > const &p_pixmap_view)
{
	return p_pixmap_view.m_width;
}


template < typename PixelData >
inline std::size_t height(pixmap_view < PixelData > const &p_pixmap_view)
{
	return p_pixmap_view.m_height;
}


template < typename PixelData >
inline std::size_t hstride(pixmap_view < PixelData > const &p_pixmap_view)
{
	return p_pixmap_view.m_hstride;
}


template < typename PixelData >
inline std::size_t num_channels(pixmap_view < PixelData > const &p_pixmap_view)
{
	return p_pixmap_view.m_num_channels;
}


template < typename PixelData >
inline PixelData* at(pixmap_view < PixelData > const &p_pixmap_view, std::size_t p_x, std::size_t p_y)
{
	return p_pixmap_view.m_data.data() + p_x*p_pixmap_view.m_num_channels + p_y*p_pixmap_view.m_hstride;
}


template < typename PixelData >
inline pixmap_view < PixelData > make_pixmap_view(PixelData *p_data, std::size_t p_data_size, std::size_t p_width, std::size_t p_height, std::size_t p_hstride, std::size_t p_num_channels)
{
	return pixmap_view < PixelData > {
		nonstd::make_span(p_data, p_data_size),
		p_width, p_height,
		p_hstride,
		p_num_channels
	};
}


} // namespace graphics end


#endif // GRAPHICS_PIXMAP_VIEW_HPP_______
