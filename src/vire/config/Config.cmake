add_library(
    vire-pconfig

    src/vire/config/config.hpp
    src/vire/config/config.cpp
)

target_link_libraries(VIRELANG PRIVATE vire-pconfig)