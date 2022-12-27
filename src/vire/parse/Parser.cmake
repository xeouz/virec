add_library(
    vire-parser

    ${SRC_DIR}/src/vire/parse/keyword_hash.hpp
    ${SRC_DIR}/src/vire/parse/keyword_hash.cpp

    ${SRC_DIR}/src/vire/parse/parser.hpp
    ${SRC_DIR}/src/vire/parse/parser.cpp
)

target_link_libraries(VIRELANG PRIVATE vire-parser)
