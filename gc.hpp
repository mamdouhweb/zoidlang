#pragma once

#include "object.hpp"
#include <set>
#include <type_traits>
#include <utility>

namespace zlang {
    class garbage_collector {
    public:
        garbage_collector() = default;
        garbage_collector(garbage_collector const&) = delete;
        garbage_collector(garbage_collector&&) = default;
        garbage_collector& operator=(garbage_collector const&) = delete;
        garbage_collector& operator=(garbage_collector&&) = default;
        ~garbage_collector();

        template<class T, class... Args>
        typename std::enable_if<std::is_base_of<gc_object, T>::value, T*>::type
        alloc(Args&&... args) {
            auto* obj = new T(std::forward<Args>(args)...);
            try {
                objects.insert(obj);
            } catch (...) {
                delete obj;
                throw;
            }
            return obj;
        }

        void add_root(gc_object* obj);
        void remove_root(gc_object* obj);

        void operator()();

    private:
        std::set<gc_object*> objects;
        std::set<gc_object*> roots;
    };
}

