#ifndef GRAPHICS_FI_PIXMAP_HPP_______
#define GRAPHICS_FI_PIXMAP_HPP_______

#include <cstdint>
#include <cstddef>
#include <FreeImage.h>
#include "pixmap_view.hpp"


namespace graphics
{


class fi_pixmap
{
public:
	fi_pixmap();
	fi_pixmap(FIBITMAP *p_fibitmap);
	fi_pixmap(fi_pixmap const &p_other);
	fi_pixmap(fi_pixmap &&p_other);
	~fi_pixmap();

	fi_pixmap& operator = (fi_pixmap const &p_other);
	fi_pixmap& operator = (fi_pixmap &&p_other);

	// NOTE: This is const only because all FreeImage
	// functions expect a non-const FIBITMAP pointer.
	FIBITMAP* get_fibitmap() const
	{
		return m_fibitmap;
	}

private:
	FIBITMAP *m_fibitmap;
};


const_pixmap_view_t make_pixmap_view(fi_pixmap const &p_fi_pixmap);
nonconst_pixmap_view_t make_pixmap_view(fi_pixmap &p_fi_pixmap);


} // namespace graphics end


#endif // GRAPHICS_FI_PIXMAP_HPP_______
