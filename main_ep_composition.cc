//
// Created by anb22 on 6/20/19.
//

#include "hedgehog/hedgehog.h"

int numberDuplicate = 1;
std::vector<int> vDeviceIds(numberDuplicate, 0);

class IITask : public AbstractTask<int, int> {
 public:
  IITask() : AbstractTask("IITask", numberDuplicate, false) {}
  void execute(std::shared_ptr<int> ptr) override { addResult(ptr); }
  std::shared_ptr<AbstractTask<int, int>> copy() override { return std::make_shared<IITask>(); }
};

class IIEP : public AbstractExecutionPipeline<int, int> {
 public:
  IIEP(std::shared_ptr<Graph<int, int>> const &graph) : AbstractExecutionPipeline(graph, numberDuplicate, vDeviceIds) {}
  bool sendToGraph([[maybe_unused]]std::shared_ptr<int> &data, [[maybe_unused]]size_t const &graphId) override {
    return true;
  }
};

std::shared_ptr<IITask> task() { return std::make_shared<IITask>(); }

std::shared_ptr<Graph<int, int>> innerGraph() {
  auto g = std::make_shared<Graph<int, int>>();
  auto t = task();
  g->input(t);
  g->output(t);
  return g;
}

std::shared_ptr<Graph<int, int>> wrapperGraph(const std::shared_ptr<Graph<int, int>> &innerGraph) {
  auto g = std::make_shared<Graph<int, int>>();
  auto ep = std::make_shared<IIEP>(innerGraph);
  auto t1 = task(), t2 = task();

  g->input(t1);
  g->addEdge(t1, ep);
  g->addEdge(ep, t2);
  g->output(t2);

  return g;
}

int main() {
  std::shared_ptr<Graph<int, int>>
      graph = nullptr,
      tempGraph = innerGraph();

  size_t count = 0;

  for (int i = 0; i < numberDuplicate - 1; ++i) { tempGraph = wrapperGraph(tempGraph); }

  graph = wrapperGraph(tempGraph);
  graph->executeGraph();

  for (int i = 0; i < numberDuplicate; ++i) { graph->pushData(std::make_shared<int>(i)); }

  graph->finishPushingData();

  while (graph->getBlockingResult()) { count++; }

  std::cout << "Get " << count << " results!" << std::endl;
  graph->createDotFile("graph.dot", ColorScheme::EXECUTION, StructureOptions::ALL);
  graph->waitForTermination();

  return 0;
}




