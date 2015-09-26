file(GLOB tubes_sources "*.cpp")
file(GLOB tubes_io_sources "io/*.cpp")
define_module(tubes OBJECT SOURCES ${tubes_sources} ${tubes_io_sources}  LINKS util imageprocessing volumes region_features lemon-hg)
add_subdirectory(gui)
