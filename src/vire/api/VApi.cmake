add_library(
    vire-api
    ${SRC_DIR}/src/vire/api/VApi.hpp
    ${SRC_DIR}/src/vire/api/VApi.cpp
)

target_link_libraries(VIRELANG PRIVATE vire-api)