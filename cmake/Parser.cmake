add_library(
    vire-parser

    src/Vire/AST-Parse/parser.hpp
    src/Vire/AST-Parse/parser.cpp
)

target_link_libraries(VIRELANG PRIVATE vire-parser)
