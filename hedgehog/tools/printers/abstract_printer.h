//
// Created by anb22 on 3/13/19.
//

#ifndef HEDGEHOG_ABSTRACT_PRINTER_H
#define HEDGEHOG_ABSTRACT_PRINTER_H
#include <set>
#include "../logger.h"

class CoreNode;

class AbstractPrinter {
 public:
  AbstractPrinter() = default;
  virtual ~AbstractPrinter() = default;

  virtual void printGraphHeader(CoreNode const *node) = 0;
  virtual void printGraphFooter(CoreNode const *node) = 0;
  virtual void printNodeInformation(CoreNode *node) = 0;

  virtual void printEdge(CoreNode const *from,
                         CoreNode const *to,
                         std::string_view const &edgeType,
                         size_t const &queueSize,
                         size_t const &maxQueueSize) = 0;

  virtual void printClusterHeader(std::string const &id) = 0;
  virtual void printClusterFooter() = 0;
  virtual void printClusterEdge(CoreNode const *clusterNode) = 0;

  virtual void printExecutionPipelineHeader(std::string_view const &executionPipelineName,
                                            std::string const &id,
                                            std::string const &switchId) = 0;
  virtual void printExecutionPipelineFooter() = 0;
  virtual void printEdgeSwitchGraphs(CoreNode *from,
                                     std::string const &idSwitch,
                                     std::string_view const &edgeType) = 0;

  virtual bool hasNotBeenVisited(CoreNode const *node) = 0;
};

#endif //HEDGEHOG_ABSTRACT_PRINTER_H
