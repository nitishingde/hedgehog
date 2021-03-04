//
// Created by Bardakoff, Alexandre (IntlAssoc) on 11/23/20.
//

#include "../data_structures/compile_time_analysis/graphs/graph_int_int.h"
#include "../data_structures/compile_time_analysis/tasks/task_int_int.h"
#include "../data_structures/compile_time_analysis/static_tests/test_critical_path.h"

void testCriticalPath() {
  constexpr hh::cx::CXNode<TaskIntInt> node0("Task0");
  constexpr hh::cx::CXNode<TaskIntInt> node1("Task1");
  constexpr hh::cx::CXNode<TaskIntInt> node2("Task2");
  constexpr hh::cx::CXNode<TaskIntInt> node3("Task3");
  constexpr hh::cx::CXNode<TaskIntInt> node4("Task4");
  constexpr hh::cx::CXNode<TaskIntInt> node5("Task5");
  constexpr hh::cx::CXNode<TaskIntInt> node6("Task6");
  constexpr hh::cx::CXNode<TaskIntInt> node7("Task7");

  constexpr auto defroster = [&]() {
    hh::cx::CXGraph<GraphIntInt> g("Graph of matrix multiplication");

    g.input(node0);
    g.input(node1);
    g.input(node2);
    g.addEdge(node0, node3);
    g.addEdge(node1, node3);
    g.addEdge(node2, node4);
    g.addEdge(node3, node5);
    g.addEdge(node5, node4);
    g.addEdge(node4, node6);
    g.addEdge(node6, node4);
    g.addEdge(node6, node7);
    g.output(node7);

    PropertyMap<double> propertyMap;

    propertyMap.insert("Task0", 1);
    propertyMap.insert("Task1", 1);
    propertyMap.insert("Task2", 1);
    propertyMap.insert("Task3", 1);
    propertyMap.insert("Task4", 1);
    propertyMap.insert("Task5", 1);
    propertyMap.insert("Task6", 1);
    propertyMap.insert("Task7", 1);

    auto criticalPath = TestCriticalPath<GraphIntInt>(propertyMap);
    g.addTest(criticalPath);

    return hh::cx::Defroster(g);
  }();

  ASSERT_FALSE(defroster.isGraphValid());
  ASSERT_TRUE(defroster.report().find("Task0  ->  Task3  ->  Task5  ->  Task4  ->  Task6  ->  Task7") !=
  std::string::npos);
}


