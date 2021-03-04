//
// Created by Bardakoff, Alexandre (IntlAssoc) on 11/9/20.
//

#include "../data_structures/compile_time_analysis/graphs/graph_int_int.h"
#include "../data_structures/compile_time_analysis/tasks/task_int_int.h"

#include "../data_structures/compile_time_analysis/graphs/graph_const_int_const_int.h"
#include "../data_structures/compile_time_analysis/tasks/task_const_int_const_int.h"

#include "../data_structures/compile_time_analysis/graphs/inside_graph.h"

void testTarjanNoCylce() {
  constexpr hh::cx::CXNode<TaskIntInt> node("Task1");
  constexpr hh::cx::CXNode<TaskIntInt> node2("Task2");

  constexpr auto defroster = [&]() {
    hh::cx::CXGraph<GraphIntInt> g("Graph without cycle");

    g.input(node);
    g.addEdge(node, node2);
    g.output(node2);

    auto cycleTest = hh::cx::test::CycleTest<GraphIntInt>{};
    auto constTest = hh::cx::test::DataRaceTest<GraphIntInt>{};
    g.addTest(cycleTest);
    g.addTest(constTest);

    return hh::cx::Defroster(g);
  }();
  ASSERT_TRUE(defroster.isGraphValid());
}

void testTarjanSimpleCycle() {
  constexpr hh::cx::CXNode<TaskIntInt> node("Task1");
  constexpr hh::cx::CXNode<TaskIntInt> node2("Task2");

  constexpr auto defroster = [&]() {
    hh::cx::CXGraph<GraphIntInt> g("Graph with hedgehog simple cycle between input and output");

    g.input(node);
    g.addEdge(node, node2);
    g.addEdge(node2, node);
    g.output(node2);

    auto cycle = hh::cx::test::CycleTest<GraphIntInt>{};
    auto constTest = hh::cx::test::DataRaceTest<GraphIntInt>{};
    g.addTest(cycle);
    g.addTest(constTest);

    return hh::cx::Defroster(g);
  }();
  ASSERT_FALSE(defroster.isGraphValid());
  ASSERT_TRUE(defroster.report().find("Task1  ->  Task2  ->  Task1") != std::string::npos);
}

void testTarjanSimpleCylce3nodes() {
  constexpr hh::cx::CXNode<TaskIntInt> node("Task1");
  constexpr hh::cx::CXNode<TaskIntInt> node2("Task2");
  constexpr hh::cx::CXNode<TaskIntInt> node3("Task3");

  constexpr auto defroster = [&]() {
    hh::cx::CXGraph<GraphIntInt> g("Graph with hedgehog cycle in the middle");

    g.input(node);
    g.addEdge(node, node2);
    g.addEdge(node2, node3);
    g.addEdge(node3, node2);
    g.output(node3);

    auto cycle = hh::cx::test::CycleTest<GraphIntInt>{};
    auto constTest = hh::cx::test::DataRaceTest<GraphIntInt>{};
    g.addTest(cycle);
    g.addTest(constTest);

    return hh::cx::Defroster(g);
  }();
  ASSERT_FALSE(defroster.isGraphValid());
  ASSERT_TRUE(defroster.report().find("Task2  ->  Task3  ->  Task2") != std::string::npos);
}

void testTarjanComplexCycles() {
  constexpr hh::cx::CXNode<TaskIntInt> node1("Task1");
  constexpr hh::cx::CXNode<TaskIntInt> node2("Task2");
  constexpr hh::cx::CXNode<TaskIntInt> node3("Task3");
  constexpr hh::cx::CXNode<TaskIntInt> node4("Task4");
  constexpr hh::cx::CXNode<TaskIntInt> node5("Task5");
  constexpr hh::cx::CXNode<TaskIntInt> node6("Task6");
  constexpr hh::cx::CXNode<TaskIntInt> node7("Task7");

  constexpr auto defroster = [&]() {
    hh::cx::CXGraph<GraphIntInt> g("Graph with multiple cycles, some inside");

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
    auto constTest = hh::cx::test::DataRaceTest<GraphIntInt>{};
    g.addTest(cycle);
    g.addTest(constTest);

    return hh::cx::Defroster(g);
  }();

  ASSERT_FALSE(defroster.isGraphValid());
  ASSERT_TRUE(defroster.report().find("Task1  ->  Task2  ->  Task3  ->  Task4  ->  Task1") != std::string::npos);
  ASSERT_TRUE(defroster.report().find("Task2  ->  Task3  ->  Task5  ->  Task6  ->  Task2") != std::string::npos);
  ASSERT_TRUE(defroster.report().find("Task2  ->  Task5  ->  Task6  ->  Task2") != std::string::npos);
  ASSERT_TRUE(defroster.report().find("Task2 -> Task3 / Task5") != std::string::npos);
  ASSERT_TRUE(defroster.report().find("Task3 -> Task4 / Task5") != std::string::npos);
  ASSERT_TRUE(defroster.report().find("Task4 -> Task1 / Task7") != std::string::npos);
}

void testTarjanComplexCyclesConst() {
  constexpr hh::cx::CXNode<TaskConstIntConstInt> node1("Task1");
  constexpr hh::cx::CXNode<TaskConstIntConstInt> node2("Task2");
  constexpr hh::cx::CXNode<TaskConstIntConstInt> node3("Task3");
  constexpr hh::cx::CXNode<TaskConstIntConstInt> node4("Task4");
  constexpr hh::cx::CXNode<TaskConstIntConstInt> node5("Task5");
  constexpr hh::cx::CXNode<TaskConstIntConstInt> node6("Task6");
  constexpr hh::cx::CXNode<TaskConstIntConstInt> node7("Task7");

  constexpr auto defroster = [&]() {
    hh::cx::CXGraph<GraphConstIntConstInt>
        g("Graph with const inputs, with multiple cycles, some inside");

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

    auto cycle = hh::cx::test::CycleTest<GraphConstIntConstInt>{};
    auto constTest = hh::cx::test::DataRaceTest<GraphConstIntConstInt>{};
    g.addTest(cycle);
    g.addTest(constTest);

    return hh::cx::Defroster(g);
  }();
  ASSERT_FALSE(defroster.isGraphValid());
  ASSERT_TRUE(defroster.report().find("Task1  ->  Task2  ->  Task3  ->  Task4  ->  Task1") != std::string::npos);
  ASSERT_TRUE(defroster.report().find("Task2  ->  Task3  ->  Task5  ->  Task6  ->  Task2") != std::string::npos);
  ASSERT_TRUE(defroster.report().find("Task2  ->  Task5  ->  Task6  ->  Task2") != std::string::npos);
}

void testTarjanSameNodeCycle() {
  constexpr hh::cx::CXNode<TaskIntInt> node("Task1");

  constexpr auto defroster = [&]() {
    hh::cx::CXGraph<GraphIntInt> g("Graph with hedgehog cycle in hedgehog single node");

    g.input(node);
    g.addEdge(node, node);
    g.output(node);

    auto cycle = hh::cx::test::CycleTest<GraphIntInt>{};
    auto constTest = hh::cx::test::DataRaceTest<GraphIntInt>{};
    g.addTest(cycle);
    g.addTest(constTest);

    return hh::cx::Defroster(g);
  }();

  ASSERT_FALSE(defroster.isGraphValid());
  ASSERT_TRUE(defroster.report().find("Task1  ->  Task1") != std::string::npos);
}

void testCycleMultiInputs() {
  constexpr hh::cx::CXNode<TaskIntInt> node0("Task0");
  constexpr hh::cx::CXNode<TaskIntInt> node1("Task1");
  constexpr hh::cx::CXNode<TaskIntInt> node2("Task2");
  constexpr hh::cx::CXNode<TaskIntInt> node3("Task3");

  constexpr auto defroster = [&]() {
    hh::cx::CXGraph<GraphIntInt> g("Graph with multiple inputs and cycles");

    g.input(node0);
    g.input(node1);
    g.addEdge(node0, node2);
    g.addEdge(node1, node2);
    g.addEdge(node2, node3);
    g.addEdge(node3, node2);
    g.addEdge(node3, node1);
    g.output(node3);

    auto cycle = hh::cx::test::CycleTest<GraphIntInt>{};
    auto constTest = hh::cx::test::DataRaceTest<GraphIntInt>{};
    g.addTest(cycle);
    g.addTest(constTest);

    return hh::cx::Defroster(g);
  }();

  ASSERT_FALSE(defroster.isGraphValid());
  ASSERT_TRUE(defroster.report().find("Task1  ->  Task2  ->  Task3  ->  Task1") != std::string::npos);
  ASSERT_TRUE(defroster.report().find("Task2  ->  Task3  ->  Task2") != std::string::npos);
  ASSERT_TRUE(defroster.report().find("Task3 -> Task1 / Task2") != std::string::npos);
}

void testCompositionAsStaticGraph() {
  constexpr auto insideGraph = hh::cx::CXGraph<InsideGraph>("Inside Graph");
  constexpr auto insideNode = hh::cx::CXNode<TaskIntInt>("Output Node");

  constexpr auto defroster = [&]() {
    auto graph = hh::cx::CXGraph<GraphIntInt>("Outside Grpah");

    graph.input(insideGraph);
    graph.addEdge(insideGraph, insideNode);
    graph.output(insideNode);

    auto cycle = hh::cx::test::CycleTest<GraphIntInt>{};
    auto constTest = hh::cx::test::DataRaceTest<GraphIntInt>{};
    graph.addTest(cycle);
    graph.addTest(constTest);

    return hh::cx::Defroster(graph);
  }();

  ASSERT_TRUE(defroster.isGraphValid());
  if constexpr(defroster.isGraphValid()) {
    auto dynGraph = std::make_shared<InsideGraph>("DynInsideGRaph");
    auto dynNode = std::make_shared<TaskIntInt>("DynInsideNode");

    auto graph = defroster.convert(
        insideGraph, dynGraph,
        insideNode, dynNode
    );

    graph->executeGraph();
    for (int i = 0; i < 10; ++i) { graph->pushData(std::make_shared<int>(i)); }
    graph->finishPushingData();

    uint8_t numberReceived = 0;
    while(graph->getBlockingResult()){numberReceived++;}
    ASSERT_EQ((int)numberReceived, 10);

    graph->waitForTermination();
  }
}

void testCompositionAsStaticNode() {
  constexpr auto insideGraph = hh::cx::CXNode<InsideGraph>("Inside Graph");
  constexpr auto insideNode = hh::cx::CXNode<TaskIntInt>("Output Node");

  constexpr auto defroster = [&]() {
    auto graph = hh::cx::CXGraph<GraphIntInt>("Outside Grpah");

    graph.input(insideGraph);
    graph.addEdge(insideGraph, insideNode);
    graph.output(insideNode);

    auto cycle = hh::cx::test::CycleTest<GraphIntInt>{};
    auto constTest = hh::cx::test::DataRaceTest<GraphIntInt>{};
    graph.addTest(cycle);
    graph.addTest(constTest);

    return hh::cx::Defroster(graph);
  }();
  ASSERT_TRUE(defroster.isGraphValid());

  if constexpr(defroster.isGraphValid()) {
    auto dynGraph = std::make_shared<InsideGraph>("DynInsideGRaph");
    auto dynNode = std::make_shared<TaskIntInt>("DynInsideNode");

    auto graph = defroster.convert(
        insideGraph, dynGraph,
        insideNode, dynNode
    );

    graph->executeGraph();

    for (int i = 0; i < 10; ++i) { graph->pushData(std::make_shared<int>(i)); }
    graph->finishPushingData();

    int numberReceived = 0;
    while(auto val = graph->getBlockingResult()){numberReceived++;}
    ASSERT_EQ((int)numberReceived, 10);

    graph->waitForTermination();
  }
}

void testSameNodeType() {
  constexpr hh::cx::CXNode<TaskIntInt> node("StaticTask1");
  constexpr hh::cx::CXNode<TaskIntInt> node2("StaticTask2");

  constexpr auto defroster = [&]() {
    hh::cx::CXGraph<GraphIntInt> g("Graph without cycle");

    g.input(node);
    g.addEdge(node, node2);
    g.output(node2);

    auto cycle = hh::cx::test::CycleTest<GraphIntInt>{};
    auto constTest = hh::cx::test::DataRaceTest<GraphIntInt>{};
    g.addTest(cycle);
    g.addTest(constTest);

    return hh::cx::Defroster(g);
  }();
  ASSERT_TRUE(defroster.isGraphValid());

  if constexpr(defroster.isGraphValid()) {
    auto dynNode1 = std::make_shared<TaskIntInt>("t1");
    auto dynNode2 = std::make_shared<TaskIntInt>("t2");

    auto graph = defroster.convert(
        node, dynNode1,
        node2, dynNode2
    );

    graph->executeGraph();

    for (int i = 0; i < 10; ++i) { graph->pushData(std::make_shared<int>(i)); }
    graph->finishPushingData();
    uint8_t numberReceived = 0;
    while(graph->getBlockingResult()){numberReceived++;}
    ASSERT_EQ((int)numberReceived, 10);
    graph->waitForTermination();
  }
}
