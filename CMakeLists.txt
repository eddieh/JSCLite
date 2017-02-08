cmake_minimum_required (VERSION 2.6)
project (JSCLite)

set(APPLE_CHANGES 1)

# if "PLATFORM_MACOSX=='1'?
# set(USE_CONSERVATIVE_GC 0)
# set(USE_CONSERVATIVE_GC 1)

# if GNU?
# <!-- FIXME: we need proper configure checks -->
# <define>HAVE_FUNC_ISNAN</define>
# <!-- check for undefined symbols for debugging reasons -->
# <ldflags>-Wl,--no-undefined</ldflags>

# <if cond="PLATFORM_WIN32=='1'">
# <include>$(SRCDIR)/os-win32</include>
# <define>HAVE_SYS_TIMEB_H=1</define>
# <define>HAVE_FLOAT_H=1</define>
# <define>HAVE_FUNC__FINITE=1</define>
# </if>

include_directories(
  "${PROJECT_SOURCE_DIR}"
  "${PROJECT_SOURCE_DIR}/kjs"
  "${PROJECT_SOURCE_DIR}/pcre"
  "${CMAKE_CURRENT_BINARY_DIR}/DerivedSources/JavaScriptCore"
  )

if (CMAKE_CROSSCOMPILING)
  set(IMPORT_PATH "IMPORTFILE-NOTFOUND" CACHE FILEPATH "Point it to the export file path from a native build")
  file(TO_CMAKE_PATH "${IMPORT_PATH}" IMPORT_PATH_CMAKE)
  include(${IMPORT_PATH_CMAKE}/dftablesTargets.cmake)
endif()

if (NOT CMAKE_CROSSCOMPILING)
  add_executable(dftables "${PROJECT_SOURCE_DIR}/pcre/dftables.c")
  export(TARGETS dftables FILE "${CMAKE_CURRENT_BINARY_DIR}/dftablesTargets.cmake")
endif()

add_custom_command(
  OUTPUT DerivedSources/JavaScriptCore/grammar.cpp
  COMMAND sh "${PROJECT_SOURCE_DIR}/make-generated-sources.sh"
  ARGS "${PROJECT_SOURCE_DIR}"
  )

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/DerivedSources/JavaScriptCore")

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/DerivedSources/JavaScriptCore/chartables.c
  DEPENDS dftables
  COMMAND dftables
  ARGS chartables.c
  WORKING_DIRECTORY DerivedSources/JavaScriptCore
  )

add_custom_target(
  chartables_file ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/DerivedSources/JavaScriptCore/chartables.c)

add_library(js SHARED
  DerivedSources/JavaScriptCore/grammar.cpp
  kjs/array_object.cpp
  kjs/bool_object.cpp
  kjs/collector.cpp
  kjs/Context.cpp
  kjs/date_object.cpp
  kjs/debugger.cpp
  kjs/dtoa.cpp
  kjs/error_object.cpp
  kjs/ExecState.cpp
  kjs/fpconst.cpp
  kjs/function.cpp
  kjs/function_object.cpp
  kjs/identifier.cpp
  kjs/internal.cpp
  kjs/interpreter.cpp
  kjs/JSImmediate.cpp
  kjs/JSLock.cpp
  kjs/lexer.cpp
  kjs/list.cpp
  kjs/lookup.cpp
  kjs/math_object.cpp
  kjs/nodes.cpp
  kjs/nodes2string.cpp
  kjs/number_object.cpp
  kjs/PropertyNameArray.cpp
  kjs/object.cpp
  kjs/object_object.cpp
  kjs/operations.cpp
  kjs/Parser.cpp
  kjs/property_map.cpp
  kjs/property_slot.cpp
  #kjs/reference.cpp
  #kjs/reference_list.cpp
  kjs/regexp.cpp
  kjs/regexp_object.cpp
  kjs/scope_chain.cpp
  kjs/string_object.cpp
  kjs/ustring.cpp
  kjs/value.cpp

  wtf/Assertions.cpp
  wtf/FastMalloc.cpp
  wtf/HashTable.cpp

  pcre/pcre_compile.c
  pcre/pcre_config.c
  pcre/pcre_exec.c
  pcre/pcre_fullinfo.c
  pcre/pcre_get.c
  pcre/pcre_globals.c
  pcre/pcre_info.c
  pcre/pcre_maketables.c
  pcre/pcre_ord2utf8.c
  pcre/pcre_printint.c
  pcre/pcre_refcount.c
  pcre/pcre_study.c
  pcre/pcre_tables.c
  pcre/pcre_try_flipped.c
  pcre/pcre_ucp_findchar.c
  pcre/pcre_version.c
  pcre/pcre_xclass.c
  pcre/ucp_findchar.c

  )

add_dependencies(js chartables_file)

# set_target_properties(js PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")

# set(CMAKE_CXX_FLAGS "-s -Os -fno-exceptions -fno-rtti -fno-stack-protector -ffunction-sections -fdata-sections -fno-unroll-loops")
# set(CMAKE_EXE_LINKER_FLAGS "-s")

# # add test executable
# add_executable(testkjs "${PROJECT_SOURCE_DIR}/kjs/testkjs.cpp")
# set_target_properties(testkjs PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
# target_link_libraries(testkjs js)
