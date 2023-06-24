#pragma once

namespace graphics
{
    struct Colour
    {
        float r;
        float g;
        float b;
        float a;

        static constexpr Colour red() { return { 1.f, 0.f, 0.f, 1.f }; }
        static constexpr Colour green() { return { 0.f, 1.f, 0.f, 1.f }; }
        static constexpr Colour blue() { return { 0.f, 0.f, 1.f, 1.f }; }
    };
}