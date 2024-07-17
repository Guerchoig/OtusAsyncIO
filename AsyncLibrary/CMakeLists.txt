cmake_minimum_required(VERSION 3.10)
project(async)

add_library(async SHARED src/async.cpp src/cmd_output.cpp)

set_target_properties(async PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)


target_include_directories(async PRIVATE  
                            "${PROJECT_SOURCE_DIR}/include"
)


if (MSVC)
        target_compile_options(async 
    /W4 )
else ()
       target_compile_options(async PRIVATE
        -Wall -Wextra -pedantic -Werror
    )
    
endif()



# install(TARGETS async RUNTIME DESTINATION bin)

# set(CPACK_GENERATOR DEB)

# set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
# set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
# set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

# set(CPACK_PACKAGE_CONTACT alex-guerchoig@yandex.ru)

# include(CPack)

# enable_testing()
# include(GoogleTest)
# gtest_discover_tests(test_main_control)
# add_test(test_main_control  test_main_control)



