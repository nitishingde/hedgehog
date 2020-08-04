
#include "hedgehog/hedgehog.h"

class IntDoubleCharToFloat : public hh::AbstractTask<float, int, double, char> {
public:
  explicit IntDoubleCharToFloat(size_t numberThreads = 1) : AbstractTask("IntDoubleCharToFloat", numberThreads) {}
  virtual ~IntDoubleCharToFloat() = default;
  void execute([[maybe_unused]]std::shared_ptr<int> ptr) override { this->addResult(std::make_shared<float>(*ptr)); }
  void execute([[maybe_unused]]std::shared_ptr<double> ptr) override { this->addResult(std::make_shared<float>(*ptr)); }
  void execute([[maybe_unused]]std::shared_ptr<char> ptr) override { this->addResult(std::make_shared<float>(*ptr)); }

  std::shared_ptr<AbstractTask<float, int, double, char>> copy() override {
    return std::make_shared<IntDoubleCharToFloat>(this->numberThreads());
  }
};

int main() {
  hh::Graph<float, int, double, char> g("GraphOutput");
  auto t = std::make_shared<IntDoubleCharToFloat>();
  size_t count = 0;

  g.input(t);
  g.output(t);

  g.executeGraph();

  for (uint64_t i = 0; i < 100; ++i) { g.pushData(std::make_shared<int>(i)); }

  g.finishPushingData();

  while (auto val = g.getBlockingResult()) {
    ++count;
    std::cout << "Received: " << *val << std::endl;
  }

  std::cout << "Count = " << count << std::endl;

  g.waitForTermination();
}