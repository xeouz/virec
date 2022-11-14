add_library(
    vire-proto-file

    ${SRC_DIR}/src/vire/proto/file.hpp
    ${SRC_DIR}/src/vire/proto/file.cpp

    ${SRC_DIR}/src/vire/proto/iname.hpp
    ${SRC_DIR}/src/vire/proto/iname.cpp
)

target_link_libraries(VIRELANG PRIVATE vire-proto-file)