add_library(
    vire-error-builder

    src/Vire/Error-Builder/builder.hpp
    src/Vire/Error-Builder/builder.cpp
)

target_link_libraries(VIRELANG PRIVATE vire-error-builder)