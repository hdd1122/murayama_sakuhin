#pragma once
#include <unordered_map>

struct Glyph
{
    int unicode;

    float u0, v0;
    float u1, v1;

    float pl, pb, pr, pt;
    float advance;
};

struct FontAtlas
{
    int atlasWidth;
    int atlasHeight;
    float lineHeight;
    float ascender;
    float descender;

    std::unordered_map<int, Glyph> glyphs;
};
