add_library(
    vire-compiler

    ${SRC_DIR}/src/vire/v_compiler/codegen.hpp
    ${SRC_DIR}/src/vire/v_compiler/codegen.cpp
)

target_link_libraries(VIRELANG PRIVATE vire-compiler)