add_library(
    vire-vanalyzer

    src/vire/v_analyzer/analyzer.hpp
    src/vire/v_analyzer/analyzer.cpp
)

target_link_libraries(VIRELANG PRIVATE vire-vanalyzer)