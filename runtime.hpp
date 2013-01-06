#pragma once

#include <boost/variant.hpp>
#include <stdexcept>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <iostream>

namespace zlang {
    class gc_object {
    public:
        gc_object() = default;
        gc_object(gc_object const&) = delete;
        gc_object(gc_object&&) = default;
        gc_object& operator=(gc_object const&) = delete;
        gc_object& operator=(gc_object&&) = default;
        virtual ~gc_object() = 0;

        virtual void mark() = 0;
        virtual void unmark() = 0;

        bool marked;
    };
    inline gc_object::~gc_object() {}

    class object : public gc_object {
    public:
        virtual void mark() override {
            if (marked) return;
            marked = true;
            for (auto& pair : members) pair.second->mark();
        }

        virtual void unmark() override {
            if (!marked) return;
            marked = false;
            for (auto& pair : members) pair.second->unmark();
        }

        object* get(std::string const& name) const {
            return members.at(name);
        }

        void set(std::string const& name, object* obj) {
            members[name] = obj;
        }

    private:
        std::unordered_map<std::string, object*> members;
    };

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

    struct vm {
        enum opcode : std::size_t {
            NOP = 0,

            POP = 11,

            JUMP = 20,
            CALL = 21,
            RET = 22,

            EXIT = 30,
        };

        void operator()() {
            if (call_stack.empty()) {
                throw no_stack_frame_error{"missing initial stack frame"};
            }

            for (;;) {
                switch (at(pc)) {
                    case NOP: break;

                    case POP: {
                        if (data_stack.empty()) {
                            throw stack_underflow_error{"stack underflow"};
                        }
                        data_stack.pop();
                    } break;

                    case JUMP: pc = at(++pc); break;
                    case CALL: {
                        auto* func = data_stack.top();
                        data_stack.pop();

                        if (!dynamic_cast<function*>(func)) {
                            throw vm_error{"expected function object"};
                        }
                        auto& f = static_cast<function*>(func)->implementation;

                        try {
                            auto* ret_val =
                                boost::get<function::cxx_function>(f)(*this);
                            data_stack.push(ret_val);
                        } catch (boost::bad_get const&) {
                            auto pair =
                                boost::get<function::zoidlang_function>(f);

                            auto* frame = gc.alloc<stack_frame>(
                                pc + 1,
                                static_cast<function*>(func),
                                pair.first
                            );
                            call_stack.push(frame);
                            pc = pair.second;
                        }
                    } break;
                    case RET: {
                        auto* frame = call_stack.top();
                        call_stack.pop();
                        pc = frame->ret_to;
                    } break;

                    case EXIT: return;

                    default: throw bad_opcode{"bad opcode"};
                }
                ++pc;
            }
        }

        // TODO: Implement a real GC. :)
        class {
        public:
            // TODO: Enable iff subclass of gc_object.
            template<class T, class... Args>
            T* alloc(Args&&... args) {
                return new T(std::forward<Args>(args)...);
            }

            void add_root(gc_object* obj) {}
            void remove_root(gc_object* obj) {}
        } gc;

        std::size_t pc;
        std::stack<object*> data_stack;
        std::stack<stack_frame*> call_stack;

    private:
        std::size_t at(std::size_t index) {
            return call_stack.top()->code->code[index];
        }
    };
}

