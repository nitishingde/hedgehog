//
// Created by Bardakoff, Alexandre (IntlAssoc) on 11/24/20.
//

#include "../data_structures/compile_time_analysis/tasks/task_int_int.h"
#include "../data_structures/compile_time_analysis/tasks/task_int_int_with_can_terminate.h"
#include "../data_structures/compile_time_analysis/graphs/graph_int_int.h"

void testCyclesWithoutCanTerminate() {
  constexpr hh::cx::CXNode<TaskIntInt> node1("Task1");
  constexpr hh::cx::CXNode<TaskIntInt> node2("Task2");
  constexpr hh::cx::CXNode<TaskIntInt> node3("Task3");
  constexpr hh::cx::CXNode<TaskIntInt> node4("Task4");
  constexpr hh::cx::CXNode<TaskIntInt> node5("Task5");
  constexpr hh::cx::CXNode<TaskIntInt> node6("Task6");
  constexpr hh::cx::CXNode<TaskIntInt> node7("Task7");

  constexpr auto defroster = [&]() {
    hh::cx::CXGraph<GraphIntInt> g("Graph with multiple cycles");

    g.input(node1);
    g.addEdge(node1, node2);
    g.addEdge(node2, node3);
    g.addEdge(node3, node4);
    g.addEdge(node4, node7);
    g.addEdge(node4, node1);
    g.addEdge(node3, node5);
    g.addEdge(node5, node6);
    g.addEdge(node6, node2);
    g.addEdge(node2, node5);
    g.output(node7);

    auto cycle = hh::cx::test::CycleTest<GraphIntInt>{};
    g.addTest(cycle);

    return hh::cx::Defroster(g);
  }();
  ASSERT_FALSE(defroster.isGraphValid());
  ASSERT_TRUE(defroster.report().find("Task1  ->  Task2  ->  Task3  ->  Task4  ->  Task1") != std::string::npos);
  ASSERT_TRUE(defroster.report().find("Task2  ->  Task3  ->  Task5  ->  Task6  ->  Task2") != std::string::npos);
  ASSERT_TRUE(defroster.report().find("Task2  ->  Task5  ->  Task6  ->  Task2") != std::string::npos);
}

void testCyclesWithCanTerminate() {
  constexpr hh::cx::CXNode<TaskIntIntWithCanTerminate> node1("Task1");
  constexpr hh::cx::CXNode<TaskIntIntWithCanTerminate> node2("Task2");
  constexpr hh::cx::CXNode<TaskIntIntWithCanTerminate> node3("Task3");
  constexpr hh::cx::CXNode<TaskIntIntWithCanTerminate> node4("Task4");
  constexpr hh::cx::CXNode<TaskIntIntWithCanTerminate> node5("Task5");
  constexpr hh::cx::CXNode<TaskIntIntWithCanTerminate> node6("Task6");
  constexpr hh::cx::CXNode<TaskIntIntWithCanTerminate> node7("Task7");

  constexpr auto defroster = [&]() {
    hh::cx::CXGraph<GraphIntInt> g("Graph with multiple cycles and canTerminated overloaded");

    g.input(node1);
    g.addEdge(node1, node2);
    g.addEdge(node2, node3);
    g.addEdge(node3, node4);
    g.addEdge(node4, node7);
    g.addEdge(node4, node1);
    g.addEdge(node3, node5);
    g.addEdge(node5, node6);
    g.addEdge(node6, node2);
    g.addEdge(node2, node5);
    g.output(node7);

    auto cycle = hh::cx::test::CycleTest<GraphIntInt>{};
    g.addTest(cycle);

    return hh::cx::Defroster(g);
  }();
  ASSERT_TRUE(defroster.isGraphValid());
}


