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
    gc.alloc();
    gc.alloc();
    gc.alloc();
    gc();
    gc.alloc();
}

