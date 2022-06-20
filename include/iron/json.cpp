
#include "json.h"

namespace fe {

json_doc::~json_doc() { 
    if (root_) {
        root_->~json();
    }
    delete allocator_; 
}

json* json_doc::assign_root(json&& j) {
    if (root_) {
        root_->~json();
    }
    root_ = new(allocator_->alloc(sizeof(json))) json(std::move(j));
    return root_;
}

} // namespace fe
