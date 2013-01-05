#include "runtime.hpp"
#include <iostream>

int main() {
    try {
        zlang::runtime runtime;
        auto* Object = static_cast<zlang::klass*>(runtime.find_global("Object"));
        auto* obj = zlang::call_method(Object, {Object});

        auto* to_s = obj->send_message("to_s", {});
        auto* zl_str = zlang::call_method(to_s, {obj});
        auto* str = static_cast<zlang::cxx_object*>(
                        zl_str->send_message("_string", {}));
        std::cout << boost::any_cast<std::string>(str->value) << '\n';

        runtime.gc();
    } catch (zlang::bad_selector const& e) {
        std::cerr << e.selector << '\n';
    }
}

