add_library(
    vire-parser

    src/vire/parse/parser.hpp
    src/vire/parse/parser.cpp
)

target_link_libraries(VIRELANG PRIVATE vire-parser)
