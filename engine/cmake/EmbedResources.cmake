# embed all resources in res/ as byte arrays in EmbeddedResource.cpp

function(embed_resources target root_dir)
  message(STATUS "Embedding from: ${root_dir}")
  get_filename_component(root_name "${root_dir}" NAME)   # "res"
  file(GLOB_RECURSE files RELATIVE "${root_dir}" CONFIGURE_DEPENDS "${root_dir}/*")
  list(SORT files)                                       # needed for binary search

  set(decls "")
  set(table "")
  set(i 0)
  foreach(rel IN LISTS files)
    set(abs "${root_dir}/${rel}")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${abs}")

    file(READ "${abs}" hex HEX)
    string(LENGTH "${hex}" hexlen)
    math(EXPR size "${hexlen} / 2")
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," bytes "${hex}")

    string(APPEND decls "static const unsigned char r${i}[] = { ${bytes} 0x00 };\n")
    string(APPEND table "  { \"${root_name}/${rel}\", r${i}, ${size} },\n")
    math(EXPR i "${i} + 1")
  endforeach()

  set(out "${CMAKE_CURRENT_BINARY_DIR}/asset/EmbeddedResource.cpp")
  file(WRITE "${out}" "
#include <engine/asset/EmbeddedResource.h>
#include <algorithm>
#include <string>
#include <string_view>

${decls}
namespace engine::asset {
namespace {
struct Entry { const char* path; const unsigned char* data; size_t size; };
const Entry kTable[] = {
${table}  { nullptr, nullptr, 0 } // sentinel so the table is never empty
};
const size_t kCount = sizeof(kTable) / sizeof(kTable[0]) - 1;
} // anonymous namespace

EmbeddedResource EmbeddedResource::load(std::string_view path) {
    std::string p(path);
    std::replace(p.begin(), p.end(), '\\\\', '/');
    if (p.rfind(\"./\", 0) == 0) p.erase(0, 2);

    auto it = std::lower_bound(kTable, kTable + kCount, p,
        [](const Entry& e, const std::string& s) { return std::string_view(e.path) < s; });
    if (it != kTable + kCount && p == it->path) {
        EmbeddedResource r;
        r.data = it->data;
        r.size = it->size;
        return r;
    }
    return {};
}
} // namespace engine::asset
")
  target_sources(${target} PRIVATE "${out}")
endfunction()

