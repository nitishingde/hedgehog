

#ifndef HEDGEHOG_TESTS_TESTGTEST_H
#define HEDGEHOG_TESTS_TESTGTEST_H

#include <gtest/gtest.h>

#include "tests/test_link.h"
#include "tests/test_link2.h"
#include "tests/test_cycles.h"
#include "tests/test_small_graph.h"
#include "tests/test_complex_graph.h"
#include "tests/test_memory_manager.h"
#include "tests/test_execution_pipeline.h"
#include "tests/test_simple_partial_input.h"
#include "tests/test_execution_pipeline_composition.h"
#include "compile_time_analysis/test_critical_path.h"
#include "compile_time_analysis/test_race_condition.h"
#include "compile_time_analysis/test_cycle_improved.h"
#include "compile_time_analysis/test_basic.h"
#include "compile_time_analysis/test_matrix_multiplication.h"
#include "compile_time_analysis/test_error_message.h"

#ifdef HH_USE_CUDA
#include "tests/test_cuda.h"
#endif

TEST(TEST_GRAPH, TEST_GLOBAL_GRAPH) {
  ASSERT_NO_FATAL_FAILURE(testSmallGraph());
  ASSERT_NO_FATAL_FAILURE(testComplexGraph());
}

TEST(TEST_GRAPH, TEST_GLOBAL_GRAPH_CUDA) {
#ifdef HH_USE_CUDA
  ASSERT_NO_FATAL_FAILURE(testCUDA());
#endif //HH_USE_CUDA
  ASSERT_NO_FATAL_FAILURE(testMemoryManagers());
}

TEST(TEST_GRAPH, TEST_GLOBAL_GRAPH_EP) {
  ASSERT_NO_FATAL_FAILURE(testEP());
}

TEST(TEST_GRAPH, TEST_GLOBAL_GRAPH_EP_COMPO) {
  ASSERT_NO_FATAL_FAILURE(testEPComposition());
}

TEST(TEST_GRAPH, TEST_GLOBAL_GRAPH_CYCLES) {
  ASSERT_NO_FATAL_FAILURE(testCycles());
}

TEST(TEST_GRAPH, TEST_GLOBAL_PARTIAL_INPUT) {
  ASSERT_NO_FATAL_FAILURE(testSimplePartialInput());
  ASSERT_NO_FATAL_FAILURE(testPartialInputEP());
}

TEST(TEST_GRAPH, TEST_LINK) {
  ASSERT_NO_FATAL_FAILURE(testLink());
  ASSERT_NO_FATAL_FAILURE(testLink2());
}

TEST(TEST_STATIC_ANALYSIS, TEST_CRTITICAL_PATH) {
  ASSERT_NO_FATAL_FAILURE(testCriticalPath());
}

TEST(TEST_STATIC_ANALYSIS, TEST_DATA_RACES) {
  ASSERT_NO_FATAL_FAILURE(testDataRacesWithAllEdgesTreatedAsRW());
  ASSERT_NO_FATAL_FAILURE(testDataRacesWithAllConstEdges());
  ASSERT_NO_FATAL_FAILURE(testDataRacesWithSomeEdgesTreatedAsRO());
}

TEST(TEST_STATIC_ANALYSIS, TEST_CYCLE_DETECTION) {
  ASSERT_NO_FATAL_FAILURE(testTarjanSimpleCycle());
  ASSERT_NO_FATAL_FAILURE(testTarjanSameNodeCycle());
  ASSERT_NO_FATAL_FAILURE(testTarjanComplexCycles());
  ASSERT_NO_FATAL_FAILURE(testTarjanComplexCyclesConst());
  ASSERT_NO_FATAL_FAILURE(testCycleMultiInputs());
  ASSERT_NO_FATAL_FAILURE(testTarjanSimpleCylce3nodes());
  ASSERT_NO_FATAL_FAILURE(testCyclesWithoutCanTerminate());
  ASSERT_NO_FATAL_FAILURE(testCyclesWithCanTerminate());
}

TEST(TEST_STATIC_ANALYSIS, TEST_COMPOSITION){
  ASSERT_NO_FATAL_FAILURE(testCompositionAsStaticNode());
  ASSERT_NO_FATAL_FAILURE(testCompositionAsStaticGraph());
}


TEST(TEST_STATIC_ANALYSIS, TEST_BASIC){
  ASSERT_NO_FATAL_FAILURE(testTarjanNoCylce());
  ASSERT_NO_FATAL_FAILURE(testSameNodeType());
}

TEST(TEST_STATIC_ANALYSIS, TEST_ERROR_MESSAGE){
  ASSERT_NO_FATAL_FAILURE(testErrorMessage());
}

TEST(TEST_STATIC_ANALYSIS, MATRIX_MULTIPLICATION){
  ASSERT_NO_FATAL_FAILURE(testMatrixMultiplication());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}

#endif //HEDGEHOG_TESTS_TESTGTEST_H