#include "vm.hpp"

int main() {
    std::vector<std::size_t> byte_code{
        zlang::vm::CALL,
        zlang::vm::POP,
        zlang::vm::CALL,
        zlang::vm::POP,
        zlang::vm::CALL,
        zlang::vm::POP,
        zlang::vm::EXIT,
    };

    zlang::vm vm;

    auto* f = vm.gc.alloc<zlang::function>([] (zlang::vm&) {
        std::cout << "Hello, world!\n";
        return nullptr;
    });

    vm.data_stack.push(f);
    vm.data_stack.push(f);
    vm.data_stack.push(f);

    auto* code_obj = vm.gc.alloc<zlang::byte_code>(byte_code);
    auto* initial_frame = vm.gc.alloc<zlang::stack_frame>(0, nullptr, code_obj);
    vm.call_stack.push(initial_frame);
    vm.pc = 0;

    vm();
}

