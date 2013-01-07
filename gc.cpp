#include "gc.hpp"

zlang::garbage_collector::~garbage_collector() {
    for (auto* obj : objects) delete obj;
}

void zlang::garbage_collector::add_root(gc_object* obj) {
    roots.insert(obj);
}

void zlang::garbage_collector::remove_root(gc_object* obj) {
    roots.erase(obj);
}

void zlang::garbage_collector::operator()() {
    for (auto* obj : roots) obj->mark();
    for (auto* obj : objects) {
        if (!obj->marked) {
            delete obj;
            objects.erase(obj);
        }
    }
    for (auto* obj : roots) obj->unmark();
}

