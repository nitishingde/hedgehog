#include "states/my_state.h"
#include "tasks/my_task.h"
#include "tasks/my_task_2.h"

#include "../../../hedgehog/tools/graph_signal_handler.h"
void testCycles() {
  for(int r = 0; r < 100; ++r) {
    int count = 0;
    auto myGraph = std::make_shared<Graph<float, int, double, float>>();
    auto myTask1 = std::make_shared<MyTask>("myTask1", 5);
    auto myTask2 = std::make_shared<MyTask2>("myTask2", 3);
    auto myState = std::make_shared<MyState>();
    auto stateManager = std::make_shared<DefaultStateManager<float, float>>(myState);

    myGraph->input(myTask1);
    myGraph->addEdge(myTask1, myTask2);
    myGraph->addEdge(myTask2, myTask1);
    myGraph->addEdge(myTask1, stateManager);
    myGraph->output(stateManager);
    myGraph->executeGraph();


    for (int i = 0; i < 100; i++) {
      myGraph->pushData(std::make_shared<int>(i));
      myGraph->pushData(std::make_shared<double>(i));
      myGraph->pushData(std::make_shared<float>(i));
    }

    myGraph->finishPushingData();

    while (std::shared_ptr<float> graphOutput = myGraph->getBlockingResult()) { count++; }
    ASSERT_EQ(count, 1200);

    myGraph->waitForTermination();
  }
}