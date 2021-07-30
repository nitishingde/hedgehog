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
// Created by Bardakoff, Alexandre (IntlAssoc) on 06/7/21.
//

#include "../data_structures/compile_time_analysis/tasks/task_int_int.h"
#include "../data_structures/compile_time_analysis/graphs/graph_int_int.h"

void testErrorMessage () {
  constexpr hh::cx::CXNode<TaskIntInt> nodeInit("TaskInit");
  constexpr hh::cx::CXNode<TaskIntInt> node3("Task3");
  constexpr hh::cx::CXNode<TaskIntInt> node4("Task4");
  constexpr hh::cx::CXNode<TaskIntInt> node5("Task5");
  constexpr hh::cx::CXNode<TaskIntInt> node6("Task6");
  constexpr hh::cx::CXNode<TaskIntInt> node7("Task7");
  constexpr hh::cx::CXNode<TaskIntInt> node8("Task8");
  constexpr hh::cx::CXNode<TaskIntInt> node9("Task9");
  constexpr hh::cx::CXNode<TaskIntInt> node10("Task10");
  constexpr hh::cx::CXNode<TaskIntInt> node11("Task11");
  constexpr hh::cx::CXNode<TaskIntInt> node12("Task12");
  constexpr hh::cx::CXNode<TaskIntInt> node13("Task13");
  constexpr hh::cx::CXNode<TaskIntInt> node14("Task14");
  constexpr hh::cx::CXNode<TaskIntInt> node15("Task15");

  constexpr hh::cx::CXNode<TaskIntInt> nodeFinal("TaskFinal");

  constexpr auto defroster = [&]() {
    hh::cx::CXGraph<GraphIntInt, 64, 2048> g("Graph of matrix multiplication");

    g.input(nodeInit);
    g.output(nodeFinal);
    g.addEdge(nodeFinal, nodeInit);

    g.addEdge(nodeInit, node3);
    g.addEdge(node3, node4);
    g.addEdge(node4, node5);
    g.addEdge(node5, node6);
    g.addEdge(node6, node7);
    g.addEdge(node7, node8);
    g.addEdge(node8, node9);
    g.addEdge(node9, node10);
    g.addEdge(node10, node11);
    g.addEdge(node11, node12);
    g.addEdge(node12, node13);
    g.addEdge(node13, node14);
    g.addEdge(node14, node15);
    g.addEdge(node15, nodeFinal);

    auto cycleTest = hh::cx::test::CycleTest<GraphIntInt, 64, 200, 2048>{};
    auto dataRaceTest = hh::cx::test::DataRaceTest<GraphIntInt, 64, 2048>{};
    g.addTest(cycleTest);
    g.addTest(dataRaceTest);

    return hh::cx::Defroster<GraphIntInt, 64, 2048>(g);
  }();
  ASSERT_FALSE(defroster.isGraphValid());
}