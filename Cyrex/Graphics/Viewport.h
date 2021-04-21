#pragma once

namespace Cyrex {
    class Viewport {
    public:
        [[nodiscard]] const float AspectRatio() const noexcept { return Width / Height; }

        float TopLeftX;
        float TopLeftY;
        float Width;
        float Height;
        float MinDepth;
        float MaxDepth;
    };
}
