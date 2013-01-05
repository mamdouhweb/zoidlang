#include "runtime.hpp"
#include <algorithm>
#include <stdexcept>
#include <sstream>

zlang::object* zlang::object::send_message(std::string const& selector,
                                           std::vector<object*> const& args) {
    auto it = members.find(selector);
    if (it != members.end()) {
        return it->second;
    }
    if (isa) {
        return isa->send_message(selector, args);
    }
    throw bad_selector{selector};
}

void zlang::object::gc_mark() {
    if (gc_marked) return;
    gc_marked = true;
    for (auto& member : members) member.second->gc_mark();
}

void zlang::object::gc_unmark() {
    if (!gc_marked) return;
    gc_marked = false;
    for (auto& member : members) member.second->gc_unmark();
}

zlang::object* zlang::klass::send_message(
    std::string const& selector,
    std::vector<object*> const& args
) {
    auto it = members.find(selector);
    if (it != members.end()) {
        return it->second;
    }
    if (isa) {
        it = isa->members.find(selector);
        if (it != isa->members.end()) {
            return it->second;
        }
    }
    if (super) {
        return super->send_message(selector, args);
    }
    throw bad_selector{selector};
}

void zlang::klass::gc_mark() {
    if (gc_marked) return;
    gc_marked = true;
    if (isa) isa->gc_mark();
    if (super) super->gc_mark();
    for (auto& member : members) member.second->gc_mark();
}

void zlang::klass::gc_unmark() {
    if (!gc_marked) return;
    gc_marked = false;
    if (isa) isa->gc_unmark();
    if (super) super->gc_unmark();
    for (auto& member : members) member.second->gc_unmark();
}


zlang::object* zlang::method_implementation::send_message(
    std::string const& selector,
    std::vector<object*> const& args
) {
    if (selector == "call") {
        return function(args);
    } else {
        return object::send_message(selector, args);
    }
}

zlang::object* zlang::cxx_object::send_message(
    std::string const& selector,
    std::vector<object*> const& args
) {
    throw bad_selector{selector};
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
    for (auto* root : roots) root->gc_mark();
    for (auto it = objects.begin(); it != objects.end(); ++it) {
        if (!(*it)->gc_marked) {
            delete *it;
            objects.erase(it);
        }
    }
    for (auto* root : roots) root->gc_unmark();
}

std::size_t zlang::garbage_collector::objects_count() const {
    return objects.size();
}

std::size_t zlang::garbage_collector::roots_count() const {
    return roots.size();
}

zlang::runtime::runtime() {
    global = gc.alloc<object>();
    gc.add_root(global);

    // TODO: Move these class definitions to separate files.

    klass* Object = make_class(gc, "Object", nullptr, nullptr);
    klass* Class = make_class(gc, "Class", Object, nullptr);
    Object->isa = Class;
    Class->isa = Class;

    auto* Object_init_imp = gc.alloc<method_implementation>();
    Object_init_imp->function =
        [] (std::vector<object*> const& args) {
            if (args.size() != 1) {
                // TODO: Throw Zoidlang exception.
            }
            return args[0];
        };
    Object->members["init"] = Object_init_imp;

    auto* Object_to_s_imp = gc.alloc<method_implementation>();
    Object_to_s_imp->function =
        [this] (std::vector<object*> const& args) {
            if (args.size() != 1) {
                // TODO: Throw Zoidlang exception.
            }

            std::ostringstream stream;
            stream << "<"
                   << args[0]->isa->name
                   << " at "
                   << static_cast<void*>(args[0])
                   << ">";
            auto* String = find_global("String");
            auto* string = gc.alloc<cxx_object>();
            string->value = stream.str();
            return call_method(String, {String, string});
        };
    Object->members["to_s"] = Object_to_s_imp;

    auto* Class_call_imp = gc.alloc<method_implementation>();
    Class_call_imp->function =
        [this] (std::vector<object*> args) {
            if (args.size() != 2) {
                // TODO: Throw Zoidlang exception.
            }

            auto* name_object = args[1];

            std::string name;
            try {
                auto* _string = args[1]->send_message("_string", {});
                auto* object = &dynamic_cast<cxx_object&>(*_string);
                name = boost::any_cast<std::string>(object->value);
            } catch (bad_selector const&) {
                // TODO: Throw Zoidlang exception.
                throw;
            } catch (std::bad_cast const&) {
                // TODO: Throw Zoidlang exception.
                throw;
            } catch (boost::bad_any_cast const&) {
                // TODO: Throw Zoidlang exception.
                throw;
            }

            klass* super;
            try {
                super = &dynamic_cast<klass&>(*args[2]);
            } catch (std::bad_cast const&) {
                // TODO: Throw Zoidlang exception.
                throw;
            }
            return make_class(gc, name, super,
                              static_cast<klass*>(find_global("Class")));
        };
    Class->members["call"] = Class_call_imp;

    klass* String = make_class(gc, "String", Object, Class);

    auto* String_init_imp = gc.alloc<method_implementation>();
    String_init_imp->function =
        [this] (std::vector<object*> const& args) {
            if (args.size() == 0 || args.size() > 2) {
                // TODO: Throw Zoidlang exception.
            }

            cxx_object* string;
            if (args.size() == 2 && dynamic_cast<cxx_object*>(args[1])) {
                string = static_cast<cxx_object*>(args[1]);
            } else {
                string = gc.alloc<cxx_object>();
                string->value = std::string("");
            }
            args[0]->members["_string"] = string;
            return args[0];
        };
    String->members["init"] = String_init_imp;

    auto* String_to_s_imp = gc.alloc<method_implementation>();
    String_to_s_imp->function =
        [] (std::vector<object*> const& args) {
            return args[0];
        };
    String->members["to_s"] = String_to_s_imp;

    global->members["Object"] = Object;
    global->members["Class"] = Class;
    global->members["String"] = String;
}

zlang::object* zlang::runtime::find_global(std::string const& name) {
    return global->members.at(name);
}

zlang::object* zlang::call_method(object* receiver,
                                  std::vector<object*> const& args) {
    object* obj = receiver;
    while (!dynamic_cast<method_implementation*>(obj)) {
        obj = obj->send_message("call", args);
    }
    return obj->send_message("call", args);
}

zlang::klass* zlang::make_class(garbage_collector& gc, std::string const& name,
                                klass* super, klass* class_class) {
    klass* k = gc.alloc<klass>();
    k->isa = class_class;
    k->super = super;
    k->name = name;
    auto* call = gc.alloc<method_implementation>();
    call->function =
        [=, &gc] (std::vector<object*> const& args) {
            object* obj = gc.alloc<object>();
            obj->isa = static_cast<klass*>(args[0]);
            auto* init_method = obj->send_message("init", {obj});
            std::vector<object*> new_args(args.begin() + 1, args.end());
            new_args.insert(new_args.begin(), obj);
            return call_method(init_method, new_args);
        };
    k->members["call"] = call;
    return k;
}

