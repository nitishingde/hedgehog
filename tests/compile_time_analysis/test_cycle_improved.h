// NIST-developed software is provided by NIST as a public service. You may use, copy and distribute copies of the
// software in any medium, provided that you keep intact this entire notice. You may improve, modify and create
// derivative works of the software or any portion of the software, and you may copy and distribute such modifications
// or works. Modified works should carry a notice stating that you changed the software and should note the date and
// nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the
// source of the software. NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND,
// EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR
// WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE
// CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS
// THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE. You
// are solely responsible for determining the appropriateness of using and distributing the software and you assume
// all risks associated with its use, including but not limited to the risks and costs of program errors, compliance
// with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of 
// operation. This software is not intended to be used in any situation where a failure could cause risk of injury or
// damage to property. The software developed by NIST employees is not subject to copyright protection within the
// United States.
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


