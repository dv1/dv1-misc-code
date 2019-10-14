#include "fi_pixmap.hpp"


namespace graphics
{


fi_pixmap::fi_pixmap()
	: m_fibitmap(nullptr)
{
}


fi_pixmap::fi_pixmap(FIBITMAP *p_fibitmap)
	: m_fibitmap(p_fibitmap)
{
}


fi_pixmap::fi_pixmap(fi_pixmap const &p_other)
	: m_fibitmap(p_other.m_fibitmap)
{
}


fi_pixmap::fi_pixmap(fi_pixmap &&p_other)
	: m_fibitmap(p_other.m_fibitmap)
{
	p_other.m_fibitmap = nullptr;
}


fi_pixmap::~fi_pixmap()
{
	if (m_fibitmap != nullptr)
		FreeImage_Unload(m_fibitmap);
}


fi_pixmap& fi_pixmap::operator = (fi_pixmap const &p_other)
{
	m_fibitmap = p_other.m_fibitmap;
	return *this;
}


fi_pixmap& fi_pixmap::operator = (fi_pixmap &&p_other)
{
	m_fibitmap = p_other.m_fibitmap;
	p_other.m_fibitmap = nullptr;
	return *this;
}


static std::size_t get_num_channels(FIBITMAP *p_fibitmap)
{
	switch (FreeImage_GetImageType(p_fibitmap))
	{
		// TODO: more types
		case FIT_BITMAP: return std::max(FreeImage_GetBPP(p_fibitmap) / 8u, 1u);
		case FIT_RGBF: return 3;
		default: return 1;
	}
}


const_pixmap_view_t make_pixmap_view(fi_pixmap const &p_fi_pixmap)
{
	FIBITMAP *fibitmap = p_fi_pixmap.get_fibitmap();
	std::size_t width = FreeImage_GetWidth(fibitmap);
	std::size_t height = FreeImage_GetHeight(fibitmap);
	std::size_t hstride = FreeImage_GetPitch(fibitmap);

	return make_pixmap_view(
		reinterpret_cast < std::uint8_t const * > (FreeImage_GetBits(fibitmap)),
		hstride * height,
		width, height,
		hstride,
		get_num_channels(fibitmap)
	);
}


nonconst_pixmap_view_t make_pixmap_view(fi_pixmap &p_fi_pixmap)
{
	FIBITMAP *fibitmap = p_fi_pixmap.get_fibitmap();
	std::size_t width = FreeImage_GetWidth(fibitmap);
	std::size_t height = FreeImage_GetHeight(fibitmap);
	std::size_t hstride = FreeImage_GetPitch(fibitmap);

	return make_pixmap_view(
		reinterpret_cast < std::uint8_t* > (FreeImage_GetBits(fibitmap)),
		hstride * height,
		width, height,
		hstride,
		get_num_channels(fibitmap)
	);
}

	
} // namespace graphics end
