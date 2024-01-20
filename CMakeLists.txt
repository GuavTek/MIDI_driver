
add_library(midi_driver INTERFACE)
target_sources(midi_driver PUBLIC MIDI_driver.cpp)
target_include_directories(midi_driver INTERFACE ${CMAKE_CURRENT_LIST_DIR})
