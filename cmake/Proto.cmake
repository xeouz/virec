add_library(
    vire-proto-file

    src/vire/proto/file.hpp
    src/vire/proto/file.cpp
)

target_link_libraries(VIRELANG PRIVATE vire-proto-file)