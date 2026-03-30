#include <eventengine/error.hpp>
#include <uv.h>

namespace eventengine {

std::string Error::message() const {
    return uv_strerror(code_);
}

std::string Error::name() const {
    return uv_err_name(code_);
}

void Error::throwIfError() const {
    if (code_ < 0) {
        throw Exception(*this);
    }
}

Exception::Exception(Error err)
    : std::runtime_error(err.message()), error_(err) {}

} // namespace eventengine
