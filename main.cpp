#include "lex.hpp"
#include "runtime.hpp"
#include <iostream>

int main() {
    std::string text{R"code(
import io

# This is a comment! :D

main = ->
    name = gets()
    if name == "Zoidberg"
        puts("Whoop whoop whoop!")
        return
    else
        puts("Hello, {0}!", name)
        return

    unreachable
    )code"};

    zlang::lexer lexer{text};

    try {
        for (;;) {
            auto tok = lexer.next();
            std::cout << tok.kind << ": ";
            if (tok.value) {
                std::cout << *tok.value;
            }
            std::cout << '\n';
        }
    } catch (zlang::end_of_text const&) {

    } catch (zlang::bad_escape_sequence const& e) {
        std::cerr << "Bad escape sequence at " << e.index << ".\n";
    } catch (zlang::bad_token const& e) {
        std::cerr << "Bad token at " << e.index << ".\n";
    }

    zlang::garbage_collector gc;
    gc.alloc<zlang::object>();
    gc.alloc<zlang::object>();
    gc.alloc<zlang::object>();
    gc();
    gc.alloc<zlang::object>();

    auto* method = gc.alloc<zlang::method_implementation>();
    method->function = [] (std::string const& sel,
                           std::vector<zlang::object*> const& args) {
        std::cout << "Hello, world!\n";
        return nullptr;
    };
    auto* obj = gc.alloc<zlang::object>();
    auto* obj2 = gc.alloc<zlang::object>();
    obj2->members["call"] = method;
    obj->members["call"] = obj2;
    zlang::call_method(obj, {});

    gc();

    auto* Object = gc.alloc<zlang::klass>();
    auto* ObjectMeta = gc.alloc<zlang::klass>();
    Object->isa = ObjectMeta;

    auto* to_s = gc.alloc<zlang::method_implementation>();
    to_s->function = [] (std::string const& sel,
                         std::vector<zlang::object*> const& args) {
        std::cout << "<Object at " << args[0] << ">" << '\n';
        return nullptr;
    };
    Object->members["to_s"] = to_s;

    auto* object = gc.alloc<zlang::object>();
    object->isa = Object;
    call_method(object->send_message("to_s", {}), {object});

    auto* String = gc.alloc<zlang::klass>();
    auto* StringMeta = gc.alloc<zlang::klass>();
    String->super = Object;
    String->isa = StringMeta;
    StringMeta->super = ObjectMeta;
    StringMeta->isa = ObjectMeta;

    auto* string_to_s = gc.alloc<zlang::method_implementation>();
    string_to_s->function = [] (std::string const& sel,
                         std::vector<zlang::object*> const& args) {
        std::cout << "<String at " << args[0] << ">" << '\n';
        return nullptr;
    };
    String->members["to_s"] = string_to_s;

    auto* my_string = gc.alloc<zlang::object>();
    my_string->isa = String;
    call_method(my_string->send_message("to_s", {}), {my_string});

    gc();
}

