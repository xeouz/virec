add_library(
    vire-proto-file

    src/Vire/Proto/File.hpp
    src/Vire/Proto/File.cpp
)

target_link_libraries(VIRELANG PRIVATE vire-proto-file)