if (BUILD_TESTS)
        set(BOOST_LIBRARIES ${BOOST_LIBRARIES} unit_test_framework)
        set(l_flags "${l_flags} -lboost_unit_test_framework")
endif()

set(executables loader)


find_host_package(Boost 1.40 REQUIRED ${BOOST_LIBRARIES})
find_host_package(OpenSSL 0.1 REQUIRED)
include_directories(${Boost_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} ${DEPS_DIR}/include)
link_directories(${Boost_LIBRARY_DIRS} ${DEPS_DIR}/lib)

file(MAKE_DIRECTORY ${out_dir})


file(GLOB headers include/emulex/*.hpp)
source_group(include FILES ${headers})
file(GLOB sources src/*.cpp src/*.c)

file(GLOB sources_node src/node/*.cpp src/node/*.cc)

if (BUILD_SHARED)
    set(cxx_definitions ${cxx_definitions} BOOST_ALL_NO_LIB)
    add_library(emulex SHARED ${headers} ${sources})
    install(TARGETS emulex ARCHIVE DESTINATION lib)
    install(TARGETS emulex RUNTIME DESTINATION lib)
else()
	add_library(emulex STATIC ${headers} ${sources})
    install(TARGETS emulex ARCHIVE DESTINATION lib)
endif()
install(DIRECTORY include/emulex DESTINATION include)


set_target_properties(emulex PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${out_dir})
set_target_properties(emulex PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_dir})
set_target_properties(emulex PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${out_dir})
set_target_properties(emulex PROPERTIES LINK_FLAGS ${l_flags})
set_target_properties(emulex PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
target_link_libraries(emulex ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} ed2k)
target_compile_definitions(emulex PRIVATE ${cxx_definitions})

 if (BUILD_TOOLS)
     foreach(emulex_component ${executables})
         file(GLOB_RECURSE component_headers test/${emulex_component}/*hpp)
         file(GLOB_RECURSE component_sources test/${emulex_component}/*cpp)
         add_executable(${emulex_component} ${component_headers} ${component_sources})
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
     add_executable(run_emulex_tests ${test_headers} ${test_sources})
     set_target_properties(run_emulex_tests PROPERTIES COMPILE_FLAGS ${cxx_flags})
     set_target_properties(run_emulex_tests PROPERTIES LINK_FLAGS ${l_flags})
     set_target_properties(run_emulex_tests PROPERTIES RUNTIME_OUTPUT_DIRECTORY  ${test_dir})
     target_compile_definitions(run_emulex_tests PRIVATE ${cxx_definitions})
     target_link_libraries(run_emulex_tests emulex)
     TARGET_LINK_LIBRARIES(run_emulex_tests ${Boost_LIBRARIES} ${COMMON_LIBRARIES} ${SQLITE3_LIBRARIES})
 endif(BUILD_TESTS)
