#pragma once

#include <sushi/sushi.hpp>

#ifdef INFINITE
#undef INFINITE
#include <msdfgen-ext.h>
#include <msdfgen.h>
#define INFINITE 0xffffffff
#else
#include <msdfgen-ext.h>
#include <msdfgen.h>
#endif

#include <string>
#include <unordered_map>
#include <memory>

namespace ember {

struct FontDeleter {
    void operator()(msdfgen::FontHandle* ptr) {
        msdfgen::destroyFont(ptr);
    }
};

class msdf_font {
public:
    struct glyph {
        sushi::mesh_group mesh;
        sushi::texture_2d texture;
        float advance;
    };

    msdf_font() = default;
    msdf_font(const std::string& filename);

    const glyph& get_glyph(int unicode) const;

private:
    std::unique_ptr<msdfgen::FontHandle, FontDeleter> font;
    mutable std::unordered_map<int, glyph> glyphs;
};

} // namespace ember
