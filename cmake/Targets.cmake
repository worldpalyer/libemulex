if (BUILD_TESTS)
        set(BOOST_LIBRARIES ${BOOST_LIBRARIES} unit_test_framework filesystem)
        set(l_flags "${l_flags} -lboost_unit_test_framework")
endif()

set(executables loader)


find_host_package(Boost 1.40 REQUIRED ${BOOST_LIBRARIES})
FIND_PACKAGE(SQLite3 REQUIRED)
if (NOT SQLITE3_FOUND)
    message( FATAL_ERROR "the sqlite3 not found")
endif()

include_directories(${Boost_INCLUDE_DIR} SQLITE3_INCLUDE_DIRS)
link_directories(${Boost_LIBRARY_DIRS})

file(MAKE_DIRECTORY ${out_dir})


file(GLOB headers include/emulex/*.hpp include/emulex/db/*.hpp)
source_group(include FILES ${headers})
file(GLOB sources src/*.cpp src/*.c src/db/*.cpp)



if (BUILD_SHARED)
    set(l_flags "${l_flags} -lboost_thread -lboost_system -lboost_regex -lboost_filesystem -lz -led2k -lsqlite3 -lcrypto")
    add_library(emulex SHARED ${headers} ${sources})
    install(TARGETS emulex LIBRARY DESTINATION lib)
else()
    add_library(emulex STATIC ${headers} ${sources})
    install(TARGETS emulex ARCHIVE DESTINATION lib)
endif()

install(DIRECTORY include/emulex DESTINATION include)

set_target_properties(emulex PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${out_dir} )
set_target_properties(emulex PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_dir})
set_target_properties(emulex PROPERTIES LINK_FLAGS ${l_flags})
set_target_properties(emulex PROPERTIES COMPILE_FLAGS ${cxx_flags})

target_compile_definitions(emulex PRIVATE ${cxx_definitions})

 if (BUILD_TOOLS)
     foreach(emulex_component ${executables})
         file(GLOB_RECURSE component_headers test/${emulex_component}/*hpp)
         file(GLOB_RECURSE component_sources test/${emulex_component}/*cpp)
         add_executable(${emulex_component} ${headers} ${component_headers} ${component_sources})
         set_target_properties(${emulex_component} PROPERTIES COMPILE_FLAGS ${cxx_flags})
         set_target_properties(${emulex_component} PROPERTIES LINK_FLAGS ${l_flags})
         set_target_properties(${emulex_component} PROPERTIES RUNTIME_OUTPUT_DIRECTORY  ${out_dir})
         target_compile_definitions(${emulex_component} PRIVATE ${cxx_definitions})
         target_link_libraries(${emulex_component} emulex)
         # link boost and system libraries
         TARGET_LINK_LIBRARIES(${emulex_component} ${Boost_LIBRARIES} ${COMMON_LIBRARIES} ${SQLITE3_LIBRARIES})
     endforeach(emulex_component)
 endif()
 
 if (BUILD_TESTS)
     set(test_dir "${PROJECT_SOURCE_DIR}/unit")
     file(GLOB_RECURSE test_headers ${test_dir}/*hpp)
     file(GLOB_RECURSE test_sources ${test_dir}/*cpp)
     add_executable(run_tests ${headers} ${test_headers} ${test_sources})
     set_target_properties(run_tests PROPERTIES COMPILE_FLAGS ${cxx_flags})
     set_target_properties(run_tests PROPERTIES LINK_FLAGS ${l_flags})
     set_target_properties(run_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY  ${test_dir})
     target_compile_definitions(run_tests PRIVATE ${cxx_definitions})
     target_link_libraries(run_tests emulex)
     TARGET_LINK_LIBRARIES(run_tests ${Boost_LIBRARIES} ${COMMON_LIBRARIES} ${SQLITE3_LIBRARIES})
 endif(BUILD_TESTS)
