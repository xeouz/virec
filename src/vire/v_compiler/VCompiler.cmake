add_library(
    vire-vcompiler

    src/vire/v_compiler/codegen.hpp
    src/vire/v_compiler/codegen.cpp
)

target_link_libraries(VIRELANG PRIVATE vire-vcompiler)