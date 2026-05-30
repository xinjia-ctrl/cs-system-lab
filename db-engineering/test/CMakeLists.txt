# ============================================================
# Google Test 集成
# ============================================================
include(GoogleTest)
enable_testing()

# 自动发现所有 *_test.cpp
file(GLOB_RECURSE TEST_SRC "*_test.cpp")

if(NOT TEST_SRC)
  message(STATUS "No test files found, skipping test targets")
  return()
endif()

foreach(TEST_FILE ${TEST_SRC})
  get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)

  add_executable(${TEST_NAME} ${TEST_FILE})
  target_link_libraries(${TEST_NAME}
    PRIVATE
      db_engine
      gtest
      gtest_main
      fmt::fmt
  )
  target_include_directories(${TEST_NAME}
    PRIVATE
      ${CMAKE_SOURCE_DIR}/src/include
  )
  gtest_discover_tests(${TEST_NAME})
endforeach()
