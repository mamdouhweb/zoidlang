#pragma once

#include <unordered_map>
#include <string>

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
}

