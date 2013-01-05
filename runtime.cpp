#include "runtime.hpp"
#include <algorithm>
#include <stdexcept>

#include <iostream>
#include <typeinfo>

zlang::object* zlang::object::send_message(std::string const& selector,
                                           std::vector<object*> const& args) {
    auto it = members.find(selector);
    if (it != members.end()) {
        return it->second;
    } else if (isa) {
        return isa->send_message(selector, args);
    } else {
        throw bad_selector{selector};
    }
}

zlang::object* zlang::method_implementation::send_message(
    std::string const& selector,
    std::vector<object*> const& args
) {
    if (selector == "call") {
        // TODO: Implement.
        return nullptr;
    } else {
        return object::send_message(selector, args);
    }
}

zlang::garbage_collector::~garbage_collector() {
    for (auto* obj : objects) {
        delete obj;
    }
}

void zlang::garbage_collector::add_root(object* obj) {
    roots.insert(obj);
}

void zlang::garbage_collector::remove_root(object* obj) {
    roots.erase(obj);
}

void zlang::garbage_collector::operator()() {
    std::for_each(roots.begin(), roots.end(), &mark);
    for (auto it = objects.begin(); it != objects.end(); ++it) {
        if ((*it)->gc_marked) {
            delete *it;
            objects.erase(it);
        }
    }
    std::for_each(roots.begin(), roots.end(), &unmark);
}

std::size_t zlang::garbage_collector::objects_count() const {
    return objects.size();
}

std::size_t zlang::garbage_collector::roots_count() const {
    return roots.size();
}

void zlang::garbage_collector::mark(object* obj) {
    if (obj->gc_marked) return;
    obj->gc_marked = true;
    for (auto& pair : obj->members) {
        mark(pair.second);
    }
}

void zlang::garbage_collector::unmark(object* obj) {
    if (!obj->gc_marked) return;
    obj->gc_marked = false;
    for (auto& pair : obj->members) {
        unmark(pair.second);
    }
}

zlang::object* zlang::call_method(object* receiver,
                                  std::vector<object*> const& args) {
    object* obj = receiver;
    while (!dynamic_cast<method_implementation*>(obj)) {
        obj = obj->send_message("call", args);
    }
    return obj->send_message("call", args);
}

