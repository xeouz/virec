add_library(
    vire-analyzer

    ${SRC_DIR}/src/vire/v_analyzer/analyzer.hpp
    ${SRC_DIR}/src/vire/v_analyzer/analyzer.cpp
)

target_link_libraries(VIRELANG PRIVATE vire-analyzer)