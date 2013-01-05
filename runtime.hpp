#pragma once

#include <boost/any.hpp>
#include <stdexcept>
#include <functional>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace zlang {
    class garbage_collector;
    struct klass;

    struct bad_selector : public std::runtime_error {
    public:
        bad_selector(std::string sel)
        : selector{std::move(sel)}
        , std::runtime_error{"bad selector"} {}

        std::string selector;
    };

    struct object {
        object(garbage_collector& gc_)
        : gc{gc_}
        , isa{nullptr}
        , gc_marked{false} {}
        object(object const&) = delete;
        object(object&&) = default;
        object& operator=(object const&) = delete;
        object& operator=(object&&) = default;
        virtual ~object() = default;

        virtual object* send_message(std::string const& selector,
                                     std::vector<object*> const& args);

        virtual void gc_mark();
        virtual void gc_unmark();

        klass* isa;
        std::unordered_map<std::string, object*> members;
        garbage_collector& gc;
        bool gc_marked;
    };

    struct klass : object {
        klass(garbage_collector& gc_) : object{gc_} {}

        virtual object* send_message(std::string const& selector,
                                     std::vector<object*> const& args) override;

        virtual void gc_mark() override;
        virtual void gc_unmark() override;

        std::string name;
        klass* super;
    };

    struct method_implementation : object {
        method_implementation(garbage_collector& gc_) : object{gc_} {}

        virtual object* send_message(std::string const& selector,
                                     std::vector<object*> const& args) override;

        std::function<object*(std::vector<object*> const&)> function;
    };

    struct cxx_object : object {
        cxx_object(garbage_collector& gc_) : object{gc_} {}

        virtual object* send_message(std::string const& selector,
                                     std::vector<object*> const& args) override;

        boost::any value;
    };

    class garbage_collector {
    public:
        garbage_collector() = default;
        garbage_collector(garbage_collector const&) = delete;
        garbage_collector(garbage_collector&&) = default;
        garbage_collector& operator=(garbage_collector const&) = delete;
        garbage_collector& operator=(garbage_collector&&) = default;
        ~garbage_collector();

        template<class T>
        typename std::enable_if<std::is_base_of<object, T>::value, T>::type*
        alloc() {
            auto* obj = new T{*this};
            try {
                objects.insert(obj);
            } catch (std::bad_alloc const&) {
                delete obj;
                throw;
            }
            return obj;
        }

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

    class runtime {
    public:
        runtime();

        object* find_global(std::string const& name);

        garbage_collector gc;

    private:
        object* global;
    };

    object* call_method(object* receiver, std::vector<object*> const& args);
    klass* make_class(garbage_collector& gc, std::string const& name,
                      klass* super, klass* object_class);
}

