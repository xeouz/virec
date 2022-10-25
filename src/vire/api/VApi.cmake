add_library(
    vire-api

    src/vire/api/VApi.hpp
    src/vire/api/VApi.cpp
)

target_link_libraries(VIRELANG PRIVATE vire-api)