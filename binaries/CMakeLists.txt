define_module(host             BINARY SOURCES host.cpp             LINKS inference io util)
define_module(create_project   BINARY SOURCES create_project.cpp   LINKS imageprocessing)
define_module(extract_tubes    BINARY SOURCES extract_tubes.cpp    LINKS tubes)
define_module(extract_features BINARY SOURCES extract_features.cpp LINKS tubes)
