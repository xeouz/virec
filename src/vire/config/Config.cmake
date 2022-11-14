add_library(
    vire-pconfig

    ${SRC_DIR}/src/vire/config/config.hpp
    ${SRC_DIR}/src/vire/config/config.cpp
)

target_link_libraries(VIRELANG PRIVATE vire-pconfig)