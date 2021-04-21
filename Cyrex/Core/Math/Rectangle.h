#pragma once
#include <limits>

//I fucking hate macros
#ifdef max
#undef max
#endif

namespace Cyrex::Math {
    class Rectangle {
    public:
        Rectangle() noexcept
        :
            Left(0),
            Top(0),
            Right(std::numeric_limits<long>::max()),
            Bottom(std::numeric_limits<long>::max())
        {}

        Rectangle(const long left, const long top, const long right, const long bottom) noexcept
            :
            Left(left),
            Top(top),
            Right(right),
            Bottom(bottom)
        {}

        Rectangle(const Rectangle& rhs) noexcept
            :
            Left(rhs.Left),
            Top(rhs.Top),
            Right(rhs.Right),
            Bottom(rhs.Bottom)
        {}

        ~Rectangle() = default;

        [[nodiscrad]] bool operator==(const Rectangle& rhs) const noexcept { 
            return Left == rhs.Left && Top == rhs.Top && Right == rhs.Right && Bottom == rhs.Bottom; 
        }

        [[nodiscrad]] bool operator!=(const Rectangle& rhs) const noexcept {
            return !(*this == rhs);
        }

        long Left;
        long Top;
        long Right;
        long Bottom;
    };
}
