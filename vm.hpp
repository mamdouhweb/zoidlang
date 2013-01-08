#pragma once

#include "gc.hpp"
#include "object.hpp"
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <stdexcept>
#include <memory>
#include <stack>
#include <string>
#include <utility>
#include <vector>

namespace zlang {
    class vm;

    class byte_code : public object {
    public:
        byte_code(std::vector<std::size_t> byte_code) : code{byte_code} {}

        std::vector<std::size_t> code;
    };

    class function : public object {
    public:
        using zoidlang_function = std::pair<byte_code*, std::size_t>;
        using cxx_function = std::function<object*(vm&)>;

        function(zoidlang_function f) : implementation{f} {}
        function(cxx_function f) : implementation{f} {}

        virtual void mark() override {
            if (marked) return;
            object::mark();
            try {
                boost::get<zoidlang_function>(implementation).first->mark();
            } catch (boost::bad_get const&) {}
        }

        virtual void unmark() override {
            if (!marked) return;
            object::unmark();
            try {
                boost::get<zoidlang_function>(implementation).first->unmark();
            } catch (boost::bad_get const&) {}
        }

        boost::variant<zoidlang_function, cxx_function> implementation;
    };

    class stack_frame : public object {
    public:
        stack_frame(std::size_t ret_to_, function* f, byte_code* c)
        : ret_to{ret_to_}
        , func(f)
        , code(c) {}

        virtual void mark() override {
            if (marked) return;
            object::mark();
            if (func) func->mark();
            code->mark();
        }

        virtual void unmark() override {
            if (!marked) return;
            object::unmark();
            if (func) func->unmark();
            code->unmark();
        }

        std::size_t ret_to;
        boost::optional<std::size_t> exception_handler;
        function* func;
        byte_code* code;
    };

#define VM_ERROR(name) \
    class name : public vm_error { \
    public: \
        name (std::string const& e) : vm_error{e} {} \
        name (char const* e) : vm_error{e} {} \
    }

    class vm_error : public std::runtime_error {
    public:
        vm_error(std::string const& e) : std::runtime_error{e} {}
        vm_error(char const* e) : std::runtime_error{e} {}
    };

    VM_ERROR(no_stack_frame_error);
    VM_ERROR(stack_underflow_error);
    VM_ERROR(bad_opcode);
    VM_ERROR(unhandled_exception);

    template<class T>
    class gc_stack {
    public:
        static_assert(std::is_base_of<gc_object, T>::value, "");

        gc_stack(garbage_collector& gc_) : gc{gc_} {}
        gc_stack(gc_stack const&) = delete;
        gc_stack(gc_stack&&) = default;
        gc_stack& operator=(gc_stack const&) = delete;
        gc_stack& operator=(gc_stack&&) = default;
        ~gc_stack() {
            while (!stack.empty()) {
                gc.remove_root(stack.top());
                stack.pop();
            }
        }

        void push(T* obj) {
            stack.push(obj);
            gc.add_root(obj);
        }

        T* top() const {
            return stack.top();
        }

        void pop() {
            gc.remove_root(stack.top());
            stack.pop();
        }

        bool empty() const {
            return stack.empty();
        }

    private:
        garbage_collector& gc;
        std::stack<T*> stack;
    };

    class vm {
    public:
        enum opcode : std::size_t {
            NOP = 0,

            POP = 10,

            JUMP = 20,
            CALL = 21,
            RET = 22,

            OBJ_GET = 30,
            OBJ_SET = 31,
            OBJ_UNSET = 32,

            SET_EXCEPTION_HANDLER = 40,
            UNSET_EXCEPTION_HANDLER = 41,
            THROW = 42,

            EXIT = 100,
        };

        vm() : data_stack{gc}, call_stack{gc} {};

        void operator()();

        garbage_collector gc;
        std::size_t pc;
        gc_stack<object> data_stack;
        gc_stack<stack_frame> call_stack;

    private:
        std::size_t at(std::size_t index) {
            return call_stack.top()->code->code[index];
        }
    };
}

