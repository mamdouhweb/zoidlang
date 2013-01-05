#pragma once

#include <set>
#include <string>
#include <unordered_map>

namespace zlang {
    struct object {
        object() : isa{nullptr}, gc_marked{false} {}
        object(object const&) = delete;
        object(object&&) = default;
        object& operator=(object const&) = delete;
        object& operator=(object&&) = default;

        object* isa;
        std::unordered_map<std::string, object*> members;
        bool gc_marked;
    };

    class garbage_collector {
    public:
        garbage_collector() = default;
        garbage_collector(garbage_collector const&) = delete;
        garbage_collector(garbage_collector&&) = default;
        garbage_collector& operator=(garbage_collector const&) = delete;
        garbage_collector& operator=(garbage_collector&&) = default;
        ~garbage_collector();

        object* alloc();

        void add_root(object* obj);
        void remove_root(object* obj);

        void operator()();

        std::size_t objects_count() const;
        std::size_t roots_count() const;

    private:
        std::set<object*> objects;
        std::set<object*> roots;

        static void mark(object* obj);
        static void unmark(object* obj);
    };
}

