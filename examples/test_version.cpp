#include <eventengine/version.hpp>
#include <eventengine/export.hpp>
#include <uv.h>
#include <iostream>

int main() {
    std::cout << "EventEngine v" << EVENTENGINE_VERSION_STRING << "\n";
    std::cout << "  major: " << EVENTENGINE_VERSION_MAJOR << "\n";
    std::cout << "  minor: " << EVENTENGINE_VERSION_MINOR << "\n";
    std::cout << "  patch: " << EVENTENGINE_VERSION_PATCH << "\n";
    std::cout << "libuv      v" << uv_version_string() << "\n";
    return 0;
}
