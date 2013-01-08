#include "vm.hpp"

void zlang::vm::operator()() {
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
            case SWAP: {
                auto* a = data_stack.top();
                data_stack.pop();
                auto* b = data_stack.top();
                data_stack.pop();
                data_stack.push(a);
                data_stack.push(b);
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

            case OBJ_GET: {
                auto* obj = data_stack.top();
                data_stack.pop();
                auto* key = data_stack.top();
                data_stack.pop();
                // TODO: Get attribute of object and push it.
            } break;
            case OBJ_SET: {
                auto* obj = data_stack.top();
                data_stack.pop();
                auto* key = data_stack.top();
                data_stack.pop();
                auto* value = data_stack.top();
                data_stack.pop();
                // TODO: Set attribute of object.
            } break;
            case OBJ_UNSET: {
                auto* obj = data_stack.top();
                data_stack.pop();
                auto* key = data_stack.top();
                data_stack.pop();
                // TODO: Remove attribute from object.
            } break;

            case SET_EXCEPTION_HANDLER: {
                auto handler = at(++pc);
                call_stack.top()->exception_handler = handler;
            } break;
            case UNSET_EXCEPTION_HANDLER: {
                call_stack.top()->exception_handler = boost::none;
            } break;
            case THROW: {
                for (;;) {
                    if (call_stack.empty()) {
                        throw unhandled_exception{"unhandled exception"};
                    }

                    auto* frame = call_stack.top();
                    if (frame->exception_handler) {
                        pc = *frame->exception_handler;
                    } else {
                        call_stack.pop();
                    }
                }
            } break;

            case EXIT: return;

            default: throw bad_opcode{"bad opcode"};
        }
        ++pc;
    }
}

