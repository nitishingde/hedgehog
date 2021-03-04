//
// Created by Bardakoff, Alexandre (IntlAssoc) on 11/25/20.
//

#include "../data_structures/compile_time_analysis/tasks/task_int_int.h"
#include "../data_structures/compile_time_analysis/tasks/task_const_int_const_int.h"

#include "../data_structures/compile_time_analysis/graphs/graph_int_int.h"
#include "../data_structures/compile_time_analysis/graphs/graph_const_int_const_int.h"

void testDataRacesWithAllEdgesTreatedAsRW() {
  constexpr hh::cx::CXNode<TaskIntInt> node1("Task1");
  constexpr hh::cx::CXNode<TaskIntInt> node2("Task2");
  constexpr hh::cx::CXNode<TaskIntInt> node3("Task3");
  constexpr hh::cx::CXNode<TaskIntInt> node4("Task4");
  constexpr hh::cx::CXNode<TaskIntInt> node5("Task5");
  constexpr hh::cx::CXNode<TaskIntInt> node6("Task6");
  constexpr hh::cx::CXNode<TaskIntInt> node7("Task7");

  constexpr auto defroster = [&]() {
    hh::cx::CXGraph<GraphIntInt> g("Graph with all edges treated as RW");

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

    auto constTest = hh::cx::test::DataRaceTest<GraphIntInt>{};
    g.addTest(constTest);

    return hh::cx::Defroster(g);
  }();
  ASSERT_FALSE(defroster.isGraphValid());
  ASSERT_TRUE(defroster.report().find("Task2 -> Task3 / Task5") != std::string::npos);
  ASSERT_TRUE(defroster.report().find("Task3 -> Task4 / Task5") != std::string::npos);
  ASSERT_TRUE(defroster.report().find("Task4 -> Task1 / Task7") != std::string::npos);
}

void testDataRacesWithAllConstEdges() {
  constexpr hh::cx::CXNode<TaskConstIntConstInt> node1("Task1");
  constexpr hh::cx::CXNode<TaskConstIntConstInt> node2("Task2");
  constexpr hh::cx::CXNode<TaskConstIntConstInt> node3("Task3");
  constexpr hh::cx::CXNode<TaskConstIntConstInt> node4("Task4");
  constexpr hh::cx::CXNode<TaskConstIntConstInt> node5("Task5");
  constexpr hh::cx::CXNode<TaskConstIntConstInt> node6("Task6");
  constexpr hh::cx::CXNode<TaskConstIntConstInt> node7("Task7");

  constexpr auto defroster = [&]() {
    hh::cx::CXGraph<GraphConstIntConstInt> g("Graph with all const edges.");

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

    auto constTest = hh::cx::test::DataRaceTest<GraphConstIntConstInt>{};
    g.addTest(constTest);

    return hh::cx::Defroster(g);
  }();
  ASSERT_TRUE(defroster.isGraphValid());
}

void testDataRacesWithSomeEdgesTreatedAsRO() {
  constexpr hh::cx::CXNode<TaskIntInt, int> node1("Task1");
  constexpr hh::cx::CXNode<TaskIntInt> node2("Task2");
  constexpr hh::cx::CXNode<TaskIntInt, int> node3("Task3");
  constexpr hh::cx::CXNode<TaskIntInt, int> node4("Task4");
  constexpr hh::cx::CXNode<TaskIntInt, int> node5("Task5");
  constexpr hh::cx::CXNode<TaskIntInt> node6("Task6");
  constexpr hh::cx::CXNode<TaskIntInt, int> node7("Task7");

  constexpr auto defroster = [&]() {
    hh::cx::CXGraph<GraphIntInt> g("Graph with some edges treated as RO");

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

    auto constTest = hh::cx::test::DataRaceTest<GraphIntInt>{};
    g.addTest(constTest);

    return hh::cx::Defroster(g);
  }();
  ASSERT_TRUE(defroster.isGraphValid());
}
