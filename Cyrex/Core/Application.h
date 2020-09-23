#pragma once
#include "Platform/Windows/Window.h"
#include <optional>
#include <memory>

namespace Cyrex {
    class Application {
    public:
        Application();
        Application(const Application& rhs) = delete;
        Application& operator=(const Application& rhs) = delete;
        ~Application() = default;
    public:
        int Run();
    private:
        std::optional<int> MessagePump();
    private:
        std::unique_ptr<Window> window;
    };
}

