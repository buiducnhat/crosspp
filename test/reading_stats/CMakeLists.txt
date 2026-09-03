add_executable(ReadingStatsDateMathTest
  ReadingStatsDateMathTest.cpp
)

target_include_directories(ReadingStatsDateMathTest PRIVATE
  ${REPO_ROOT}/lib
)

target_link_libraries(ReadingStatsDateMathTest PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(ReadingStatsDateMathTest)
