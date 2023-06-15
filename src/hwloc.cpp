#include "hwloc_zs.h"

namespace zerosum {

void ScopedHWLOC::traverse(hwloc_obj_t obj, size_t indent) {
    std::string typestr{hwloc_obj_type_string (obj->type)};
    char tmp[256] = {0};
    hwloc_obj_attr_snprintf(tmp, 255, obj, " ", 0);
    std::cout << std::string( obj->depth*2, ' ' ) << typestr << " L#" <<
    obj->logical_index << " P#" << obj->os_index << " " << tmp << std::endl;
    for (unsigned i=0; i < obj->arity; i++) {
        this->traverse(obj->children[i], indent+2);
    }
}

void ZeroSum::validate_hwloc(void) {
    static ScopedHWLOC myloc;
    myloc.traverse();
}

}
