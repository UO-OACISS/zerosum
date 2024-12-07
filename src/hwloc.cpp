#include "hwloc_zs.h"
#include "zerosum.h"
// This is an hwloc internal function that isn't defined in hwloc.h
#if !defined(hwloc_pci_class_string)
extern "C" const char * hwloc_pci_class_string(unsigned short class_id);
#endif

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
    //std::cout << std::string( obj->depth*2, ' ' ) << typestr << " L#" <<
    //obj->logical_index << " P#" << obj->os_index << " " << tmp << std::endl;
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

std::string safe_obj_get_info(hwloc_obj_t obj) {
    std::string buffer;
    for(unsigned i=0; i<obj->infos_count; i++) {
        struct hwloc_info_s *info = &obj->infos[i];
        buffer += ", " + std::string(info->name) + ": " + std::string(info->value);
    }
    return buffer;
}

std::string getDetailName(hwloc_obj_t obj) {
    std::string buffer{""};
    char tmpstr[1024] = {0};
    constexpr double gb = 1.0 / (1024.0 * 1024.0 * 1024.0);
    //constexpr double mb = 1.0 / (1024.0 * 1024.0);
    constexpr double kb = 1.0 / (1024.0);
    switch (obj->type) {
        case HWLOC_OBJ_NUMANODE:
            buffer += "size: " + std::to_string(obj->attr->numanode.local_memory * gb) + "GB";
            break;
        case HWLOC_OBJ_L1CACHE:
        case HWLOC_OBJ_L2CACHE:
        case HWLOC_OBJ_L3CACHE:
        case HWLOC_OBJ_L4CACHE:
        case HWLOC_OBJ_L5CACHE:
        case HWLOC_OBJ_L1ICACHE:
        case HWLOC_OBJ_L2ICACHE:
        case HWLOC_OBJ_L3ICACHE:
        case HWLOC_OBJ_MEMCACHE:
            buffer += "depth: " + std::to_string(obj->attr->cache.depth);
            switch (obj->attr->cache.type) {
                case HWLOC_OBJ_CACHE_UNIFIED:
                    buffer += ", type: Unified";
                    break;
                case HWLOC_OBJ_CACHE_DATA:
                    buffer += ", type: Data";
                    break;
                case HWLOC_OBJ_CACHE_INSTRUCTION:
                    buffer += ", type: Instruction";
                    break;
            }
            buffer += ", size: " + std::to_string(obj->attr->cache.size * kb) + "kB";
            buffer += ", line size: " + std::to_string(obj->attr->cache.linesize);
            if (obj->attr->cache.associativity == -1)
                buffer += ", Fully-associative";
            else if (obj->attr->cache.associativity != 0)
                buffer += ", ways: " + std::to_string(obj->attr->cache.associativity);
            break;
        case HWLOC_OBJ_GROUP:
            buffer += "depth: " + std::to_string(obj->attr->group.depth);
            break;
        case HWLOC_OBJ_BRIDGE:
            switch (obj->attr->bridge.upstream_type) {
                case HWLOC_OBJ_BRIDGE_HOST:
                    buffer += "bridge upstream type: Host";
                    break;
                case HWLOC_OBJ_BRIDGE_PCI:
                    buffer += "bridge upstream type: PCI";
                    snprintf(tmpstr, 1024, ", bus id: %04x:%02x:%02x.%01x",
                            obj->attr->pcidev.domain, obj->attr->pcidev.bus, obj->attr->pcidev.dev, obj->attr->pcidev.func);
                    buffer += tmpstr;
                    snprintf(tmpstr, 1024, ", class: %s",
                            hwloc_pci_class_string(obj->attr->pcidev.class_id));
                    buffer += tmpstr;
                    snprintf(tmpstr, 1024, ", id: %04x:%04x",
                            obj->attr->pcidev.vendor_id, obj->attr->pcidev.device_id);
                    buffer += tmpstr;
                    if (obj->attr->pcidev.linkspeed) {
                        snprintf(tmpstr, 1024, ", linkspeed: %f GB/s", obj->attr->pcidev.linkspeed);
                        buffer += tmpstr;
                    }
                    break;
            }
            switch (obj->attr->bridge.downstream_type) {
                case HWLOC_OBJ_BRIDGE_HOST:
                    assert(0);
                case HWLOC_OBJ_BRIDGE_PCI:
                    buffer += ", bridge downstream type: PCI";
                    snprintf(tmpstr, 1024, ", secondary bus: %02x",
                            obj->attr->bridge.downstream.pci.secondary_bus);
                    buffer += tmpstr;
                    snprintf(tmpstr, 1024, ", subordinate bus: %02x",
                            obj->attr->bridge.downstream.pci.subordinate_bus);
                    buffer += tmpstr;
                    break;
            }
            break;
        case HWLOC_OBJ_PCI_DEVICE:
            snprintf(tmpstr, 1024, "bus id: %04x:%02x:%02x.%01x",
                    obj->attr->pcidev.domain, obj->attr->pcidev.bus, obj->attr->pcidev.dev, obj->attr->pcidev.func);
            buffer += tmpstr;
            snprintf(tmpstr, 1024, ", class: %s",
                    hwloc_pci_class_string(obj->attr->pcidev.class_id));
            buffer += tmpstr;
            snprintf(tmpstr, 1024, ", id: %04x:%04x",
                    obj->attr->pcidev.vendor_id, obj->attr->pcidev.device_id);
            buffer += tmpstr;
            if (obj->attr->pcidev.linkspeed) {
                snprintf(tmpstr, 1024, ", linkspeed: %f GB/s", obj->attr->pcidev.linkspeed);
                buffer += tmpstr;
            }
            break;
        case HWLOC_OBJ_OS_DEVICE:
            switch (obj->attr->osdev.type) {
                case HWLOC_OBJ_OSDEV_BLOCK:
                /**< \brief Operating system block device, or non-volatile memory device.
                  * For instance "sda" or "dax2.0" on Linux. */
                    snprintf(tmpstr, 1024, "type block");
                    break;
                case HWLOC_OBJ_OSDEV_GPU:
                /**< \brief Operating system GPU device.
                  * For instance ":0.0" for a GL display,
                  * "card0" for a Linux DRM device. */
                    snprintf(tmpstr, 1024, "type GPU");
                    break;
                case HWLOC_OBJ_OSDEV_NETWORK:
                /**< \brief Operating system network device.
                  * For instance the "eth0" interface on Linux. */
                    snprintf(tmpstr, 1024, "type Network");
                    break;
                case HWLOC_OBJ_OSDEV_OPENFABRICS:
                /**< \brief Operating system openfabrics device.
                  * For instance the "mlx4_0" InfiniBand HCA,
                  * or "hfi1_0" Omni-Path interface on Linux. */
                    snprintf(tmpstr, 1024, "type OpenFabric");
                    break;
                case HWLOC_OBJ_OSDEV_DMA:
                /**< \brief Operating system dma engine device.
                  * For instance the "dma0chan0" DMA channel on Linux. */
                    snprintf(tmpstr, 1024, "type DMA");
                    break;
                case HWLOC_OBJ_OSDEV_COPROC:
                /**< \brief Operating system co-processor device.
                  * For instance "opencl0d0" for a OpenCL device,
                  * "cuda0" for a CUDA device. */
                    snprintf(tmpstr, 1024, "type Co-processor");
                    break;
                default:
                    snprintf(tmpstr, 1024, "type unknown");
                    break;
            }
            buffer += tmpstr;
            break;
        case HWLOC_OBJ_PACKAGE:
            break;
        case HWLOC_OBJ_MACHINE:
            buffer += ZeroSum::getInstance().getHostname();
            break;
        case HWLOC_OBJ_CORE:
        case HWLOC_OBJ_PU:
            buffer += "Logical index: " + std::to_string(obj->logical_index);
            buffer += ", OS index: " + std::to_string(obj->os_index);
            break;
        default:
            /* nothing to show */
            break;
    }
    buffer += safe_obj_get_info(obj);
    return buffer;
}

std::pair<std::string, uint32_t> ScopedHWLOC::buildJSON(hwloc_obj_t obj, int32_t depth) {
    // depth-first, iterate over the children
    uint32_t childrenSize = 0;
    // keep track of the nodes we have seen already, starting with this one.
    static std::set<hwloc_obj_t> processed;
    /*
    if (processed.count(obj) > 0) {
        std::pair<std::string, uint32_t> data(std::string(""), 0);
        return data;
    }
    */
    processed.insert(obj);
    std::string json;
    std::string typestr{hwloc_obj_type_string (obj->type)};
    std::string name = typestr;
    // first! if this is a cache and it isn't shared, skip it.
    if (obj->arity == 1 && obj->memory_arity == 0 &&
        obj->io_arity == 0 && obj->misc_arity == 0) {
        switch (obj->type) {
            case HWLOC_OBJ_L1CACHE:
            case HWLOC_OBJ_L2CACHE:
            case HWLOC_OBJ_L3CACHE:
            case HWLOC_OBJ_L4CACHE:
            case HWLOC_OBJ_L5CACHE:
            case HWLOC_OBJ_L1ICACHE:
            case HWLOC_OBJ_L2ICACHE:
            case HWLOC_OBJ_L3ICACHE:
                return buildJSON(obj->first_child, depth);
            default:
                break;
        }
    }
    depth += 1;
    auto detailName = getDetailName(obj);
    if (obj->subtype != nullptr) { name += " " + std::string(obj->subtype); }
    if (obj->name != nullptr) { name += " " + std::string(obj->name); }
    if (obj->type == HWLOC_OBJ_CORE || obj->type == HWLOC_OBJ_PU) {
        name += " L#" + std::to_string(obj->logical_index);
        name += " P#" + std::to_string(obj->os_index);
    }
    json = "\n{\"name\":\"" + name + "\",\"utilization\":0,\"rank\":-1,\"detail_name\":\"" + detailName + "\",";
    // special handing for numa nodes.
    /*
    if (obj->type == HWLOC_OBJ_NUMANODE) {
        // NUMA nodes have a special depth ::HWLOC_TYPE_DEPTH_NUMANODE
        json += "\"depth\":" + std::to_string(depth);
        hwloc_obj_t child_obj = obj->parent->first_child;
        if (processed.count(child_obj) == 0) {
            json += ",\"children\":[";
            while(child_obj != nullptr) {
                auto child = buildJSON(child_obj, depth);
                json = json + child.first + ",";
                childrenSize += child.second;
                child_obj = child_obj->next_sibling;
            }
            hwloc_obj_t io_obj = obj->parent->io_first_child;
            while(io_obj != nullptr) {
                auto child = buildJSON(io_obj, depth);
                json = json + child.first + ",";
                childrenSize += child.second;
                io_obj = io_obj->next_sibling;
            }
            hwloc_obj_t misc_obj = obj->parent->misc_first_child;
            while(misc_obj != nullptr) {
                auto child = buildJSON(misc_obj, depth);
                json = json + child.first + ",";
                childrenSize += child.second;
                misc_obj = misc_obj->next_sibling;
            }
            json.back() = ']';
        }
        json += ",\"size\":" + std::to_string(childrenSize) + "}";
        std::pair<std::string, uint32_t> data(json, childrenSize);
        return data;
    }
    */
    json += "\"depth\":" + std::to_string(depth) + ",";
    // no children?
    if (obj->arity == 0 && obj->memory_arity == 0 &&
        obj->io_arity == 0 && obj->misc_arity == 0) {
        json += "\"size\":1}";
        childrenSize = 1;
    } else {
        json += "\"children\":[";
        hwloc_obj_t memory_obj = obj->memory_first_child;
        for (unsigned i=0; i < obj->memory_arity; i++) {
            auto child = buildJSON(memory_obj, depth);
            json = json + child.first + ",";
            childrenSize += child.second;
            memory_obj = memory_obj->next_sibling;
        }
        //if (obj->memory_arity == 0) {
            hwloc_obj_t child_obj = obj->first_child;
            for (unsigned i=0; i < obj->arity; i++) {
                auto child = buildJSON(child_obj, depth);
                json = json + child.first + ",";
                childrenSize += child.second;
                child_obj = child_obj->next_sibling;
            }
            hwloc_obj_t io_obj = obj->io_first_child;
            for (unsigned i=0; i < obj->io_arity; i++) {
                auto child = buildJSON(io_obj, depth);
                json = json + child.first + ",";
                childrenSize += child.second;
                io_obj = io_obj->next_sibling;
            }
            hwloc_obj_t misc_obj = obj->misc_first_child;
            for (unsigned i=0; i < obj->misc_arity; i++) {
                auto child = buildJSON(misc_obj, depth);
                json = json + child.first + ",";
                childrenSize += child.second;
                misc_obj = misc_obj->next_sibling;
            }
        //}
        json.back() = ']';
        json += ",\"size\":" + std::to_string(childrenSize) + "}";
    }
    std::pair<std::string, uint32_t> data(json, childrenSize);
    //std::cout << data.first << ": " << data.second << std::endl;
    return data;
}

void ScopedHWLOC::validate_hwloc(int shmrank) {
    static ScopedHWLOC myloc;
    myloc.traverse();
    if (shmrank == 0) {
        myloc.buildJSON();
    }
}

void ScopedHWLOC::buildJSON(void) {
    auto data = buildJSON(root, -1);
    // open a log file
    std::string filename{"zs.topology."};
    // prefix the rank with as many zeros as needed to sort correctly.
    filename += ZeroSum::getInstance().getHostname();
    filename += ".json";
    std::ofstream out(filename);
    out << data.first;
    out.close();
}
}
