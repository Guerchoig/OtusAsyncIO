cmake_minimum_required(VERSION 3.10)

set(PATCH_VERSION "0" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION 0.0.${PATCH_VERSION})

project(OtusAsyncIO VERSION ${PROJECT_VESRION})

# Local plant
link_directories(build)
include_directories(include)
add_executable(bulk_server src/bulk_server.cpp)
add_executable(client src/client.cpp) 

# a dir where sub'CmakeLists.txt resides
add_subdirectory(AsyncLibrary)

# where to look for lib binary
target_link_libraries(bulk_server PRIVATE  async)

# where to search library's header file
target_include_directories(bulk_server PRIVATE  
                            "${PROJECT_SOURCE_DIR}/include"
                            "${PROJECT_SOURCE_DIR}/AsyncLibrary/include"
)


set_target_properties(bulk_server client PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

include_directories(${CMAKE_BINARY_DIR})

if (MSVC)
    target_compile_options(bulk_server 
        /W4
    )
    target_compile_options(client 
    /W4 )
    target_compile_options(cmd_output 
    /W4 )
else ()
    target_compile_options(bulk_server PRIVATE
        -Wall -Wextra -pedantic -Werror
    )
    target_compile_options(client PRIVATE
        -Wall -Wextra -pedantic -Werror
    )
    
endif()



install(TARGETS bulk_server RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT alex-guerchoig@yandex.ru)

include(CPack)

# enable_testing()
# include(GoogleTest)
# gtest_discover_tests(test_main_control)
# add_test(test_main_control  test_main_control)



