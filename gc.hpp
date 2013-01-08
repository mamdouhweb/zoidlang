#pragma once

#include "object.hpp"
#include <memory>
#include <set>
#include <type_traits>
#include <utility>

namespace zlang {
    class garbage_collector {
    public:
        template<class T, class... Args>
        typename std::enable_if<std::is_base_of<gc_object, T>::value, T*>::type
        alloc(Args&&... args) {
            auto pair = objects.insert(
                std::unique_ptr<T>{new T(std::forward<Args>(args)...)});
            return static_cast<T*>(pair.first->get());
        }

        void add_root(gc_object* obj);
        void remove_root(gc_object* obj);

        void operator()();

    private:
        std::set<std::unique_ptr<gc_object>> objects;
        std::set<gc_object*> roots;
    };
}

