add_library(
    vire-vanalyzer

    src/Vire/VAnalyzer/analyzer.hpp
    src/Vire/VAnalyzer/analyzer.cpp
)

target_link_libraries(VIRELANG PRIVATE vire-vanalyzer)