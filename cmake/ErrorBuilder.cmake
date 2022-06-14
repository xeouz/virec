add_library(
    vire-error-builder

    src/vire/errors/builder.hpp
    src/vire/errors/builder.cpp
)

target_link_libraries(VIRELANG PRIVATE vire-error-builder)