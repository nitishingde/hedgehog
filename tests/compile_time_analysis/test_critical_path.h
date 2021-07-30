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


