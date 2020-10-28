#include "Application.h"

int main() {
    Cyrex::Application::Create();
    return Cyrex::Application::Get().Run();
}