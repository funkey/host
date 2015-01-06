define_module(host          BINARY SOURCES host.cpp          LINKS inference io util)
define_module(extract_tubes BINARY SOURCES extract_tubes.cpp LINKS imageprocessing util)
