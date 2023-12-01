#include "hwloc_zs.h"
#include "utils.h"

namespace zerosum {

/* For some weird systems (Fugaku), the first core is logically 0
 * but the OS sees it as 12. So there are 48 cores with logical
 * indexes 0-47, but the OS reports them as 12-59. We need to map
 * them so that we monitor the right CPU HWT from /proc/stat. */

std::map<size_t, size_t>& ScopedHWLOC::getHWTMap() {
    static std::map<size_t, size_t> OStoLogicalMap;
    return OStoLogicalMap;
}

void ScopedHWLOC::traverse(hwloc_obj_t obj, size_t indent) {
    std::string typestr{hwloc_obj_type_string (obj->type)};
    char tmp[256] = {0};
    hwloc_obj_attr_snprintf(tmp, 255, obj, " ", 0);
    std::cout << std::string( obj->depth*2, ' ' ) << typestr << " L#" <<
    obj->logical_index << " P#" << obj->os_index << " " << tmp << std::endl;
    for (unsigned i=0; i < obj->arity; i++) {
        this->traverse(obj->children[i], indent+2);
    }
    static bool doMap{parseBool("ZS_MAP_CORES",false)};
    static bool doMap2{parseBool("ZS_MAP_PUS",false)};
    if (doMap && obj->type == HWLOC_OBJ_PU) {
        /* Map the logical map index from the OS index */
        getHWTMap().insert(std::pair<size_t,size_t>(obj->os_index, obj->logical_index));
    }
    if (doMap2 && obj->type == HWLOC_OBJ_CORE) {
        /* Map the logical map index from the OS index */
        getHWTMap().insert(std::pair<size_t,size_t>(obj->os_index, obj->logical_index));
    }
}

void ScopedHWLOC::validate_hwloc(size_t rank) {
    static ScopedHWLOC myloc;
    if (rank == 0) {
        myloc.traverse();
    }
}

}
