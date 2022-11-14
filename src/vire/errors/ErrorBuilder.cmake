add_library(
    vire-error-builder

    ${SRC_DIR}/src/vire/errors/builder.hpp
    ${SRC_DIR}/src/vire/errors/builder.cpp
)

target_link_libraries(VIRELANG PRIVATE vire-error-builder)