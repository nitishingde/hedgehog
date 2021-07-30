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



#ifndef HEDGEHOG_DOT_PRINTER_H
#define HEDGEHOG_DOT_PRINTER_H

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <iterator>
#include <algorithm>

#include "options/color_scheme.h"
#include "options/structure_options.h"
#include "options/debug_options.h"
#include "../../core/io/base/receiver/core_slot.h"
#include "abstract_printer.h"

/// @brief Hedgehog main namespace
namespace hh {

/// @brief Printer to produce hedgehog dot representation of the current state of the graph
/// @details https://www.graphviz.org/doc/info/lang.html
class DotPrinter : public AbstractPrinter {
 private:
  std::vector<std::string> edges_ = {}; ///< List of gathered edges definition
  std::ofstream outputFile_ = {}; ///< Output file stream

  ColorScheme colorScheme_ = {}; ///< Color scheme chosen
  StructureOptions structureOptions_ = {}; ///< Structure options chosen
  DebugOptions debugOptions_ = {}; ///< Debug option chosen

  std::chrono::nanoseconds
      maxExecutionTime_ = {},         ///< Maximum execution time measured in all inside graph's nodes
  minExecutionTime_ = {},         ///< Minimum execution time measured in all inside graph's nodes
  rangeExecutionTime_ = {},       ///< Difference between maximum and minimum execution time
  maxWaitTime_ = {},              ///< Maximum wait time measured in all inside graph's nodes
  minWaitTime_ = {},              ///< Minimum wait time measured in all inside graph's nodes
  rangeWaitTime_ = {},            ///< Difference between maximum and minimum wait time
  graphExecutionDuration_ = {};   ///< Execution duration

 public:
  /// @brief DotPrinter constructor, opens the file for writing
  /// @param dotFilePath File's path to store the dot file
  /// @param colorScheme Color scheme chosen
  /// @param structureOptions Structure option chosen
  /// @param debugOptions Debug option chosen
  /// @param graph Graph to visit and create dot file
  /// @param suppressCout Suppresses the standard C output when marked as true, otherwise will report when dot files are
  /// created or overwritten to standard out.
  explicit DotPrinter(std::filesystem::path const &dotFilePath,
                      ColorScheme colorScheme, StructureOptions structureOptions, DebugOptions debugOptions,
                      core::CoreNode *graph, bool suppressCout = false)
      : AbstractPrinter(), edges_({}),
        colorScheme_(colorScheme), structureOptions_(structureOptions), debugOptions_(debugOptions) {
    assert(graph != nullptr);
    auto directoryPath = dotFilePath.parent_path();
    if (dotFilePath.has_filename()) {
      if (std::filesystem::exists(directoryPath)) {
        if (!suppressCout) {
          if (std::filesystem::exists(dotFilePath)) {
            std::cout
                << "The file " << dotFilePath.filename() << " will be overwritten." << std::endl;
          } else {
            std::cout
                << "The file " << dotFilePath.filename() << " will be created." << std::endl;
          }
        }
        outputFile_ = std::ofstream(dotFilePath);

      } else {
        std::ostringstream oss;
        oss << "The file " << dotFilePath.filename() << " can not be store in " << directoryPath
            << " because the directory does not  exist.";
        HLOG_SELF(0, oss.str())
        throw (std::runtime_error(oss.str()));
      }
    } else {
      std::ostringstream oss;
      oss << "The path: " << dotFilePath << " does not represent hedgehog file.";
      HLOG_SELF(0, oss.str())
      throw (std::runtime_error(oss.str()));
    }
    minExecutionTime_ = graph->minExecutionTime();
    maxExecutionTime_ = graph->maxExecutionTime();
    minWaitTime_ = graph->minWaitTime();
    maxWaitTime_ = graph->maxWaitTime();
    // Compute range
    rangeExecutionTime_ =
        maxExecutionTime_ == minExecutionTime_ ? std::chrono::nanoseconds(1) : maxExecutionTime_ - minExecutionTime_;
    rangeWaitTime_ = maxWaitTime_ == minWaitTime_ ? std::chrono::nanoseconds(1) : maxWaitTime_ - minWaitTime_;
    graphExecutionDuration_ =
        graph->executionDuration() == std::chrono::nanoseconds::zero() ?
        std::chrono::system_clock::now() - graph->startExecutionTimeStamp() : graph->executionDuration();
  }

  /// @brief Destructor, close the file
  ~DotPrinter() override { outputFile_.close(); }

  /// @brief Print node main information
  /// @param node Node to print information
  void printNodeInformation(core::CoreNode *node) final {
    // If the node is not hedgehog graph
    if (node->type() != core::NodeType::Graph) {
      //If all cluster node to be printed
      if (this->structureOptions_ == StructureOptions::ALL
          || this->structureOptions_ == StructureOptions::ALLTHREADING) {
        // Get and print the node information
        outputFile_ << getNodeInformation(node);
        // If only one node per cluster need to be printed with gathered information
      } else {
        // If the node is the cluster main node
        if (node->id() == node->coreClusterNode()->id()) {
          // Get and print the node information
          outputFile_ << getNodeInformation(node);
        }
      }
    }
    outputFile_.flush();
  }

  /// @brief Print header for the graph
  /// @param node Graph to print
  void printGraphHeader(core::CoreNode const *node) final {
    // If the graph is the outer graph, i.e. the main graph
    if (!node->isInside()) {
      outputFile_
          << "digraph " << node->id()
          << " {\nlabel=\"" << node->name();
      if (debugOptions_ == DebugOptions::ALL) {
        outputFile_ << " " << node->id();
      }
      outputFile_ << "\\nExecution time:" << durationPrinter(this->graphExecutionDuration_)
                  << "\\nCreation time:" << durationPrinter(node->creationDuration())
                  << "\"; fontsize=25; penwidth=5; ranksep=0; labelloc=top; labeljust=left; \n";
      // If the graph is an inner graph, i.e. hedgehog node of the outer graph
    } else {
      outputFile_ << "subgraph cluster" << node->id() << " {\nlabel=\"" << node->name();
      if (debugOptions_ == DebugOptions::ALL) {
        outputFile_ << " " << node->id();
      }
      outputFile_ << "\"; fontsize=25; penwidth=5; fillcolor=white;\n";
    }
    outputFile_.flush();
  }

  /// @brief Print the header of hedgehog cluster
  /// @param clusterNode Cluster main node
  void printClusterHeader(core::CoreNode const *clusterNode) final {
    //If all cluster node to be printed
    if (this->structureOptions_ == StructureOptions::ALLTHREADING || this->structureOptions_ == StructureOptions::ALL) {
      // Create hedgehog dot subgraph for the task cluster
      outputFile_ << "subgraph cluster" << clusterNode->id()
                  << " {\nlabel=\"\"; penwidth=3; style=filled; fillcolor=\"#4e78cf63\"; color=\"#4e78cf\";\n";
      // Add an "egg" node to represent the entry point of cluster
      outputFile_ << "box" << clusterNode->id() << "[label=\"\", shape=egg];\n";
      outputFile_.flush();
    }
  }

  /// @brief Print the footer of hedgehog task cluster
  void printClusterFooter() final {
    //If all cluster node to be printed
    if (this->structureOptions_ == StructureOptions::ALLTHREADING || this->structureOptions_ == StructureOptions::ALL) {
      // Close the subgraph
      outputFile_ << "}\n";
      outputFile_.flush();
    }
  }

  /// @brief Print the graph footer
  /// @param graph Core graph node
  void printGraphFooter(core::CoreNode const *graph) final {
    // If the graph is the outer graph
    if (!graph->isInside()) {
      // Print all the stored edges
      std::copy(edges_.begin(), edges_.end(), std::ostream_iterator<std::string>(outputFile_, "\n"));
    }
    // Close the dot subgraph
    outputFile_ << "}\n";
    outputFile_.flush();
  }

  /// @brief Print the inside edges of hedgehog cluster for hedgehog task
  /// @param clusterNode Task part of hedgehog cluster
  void printClusterEdge(core::CoreNode const *clusterNode) final {
    //If all cluster node to be printed
    if (this->structureOptions_ == StructureOptions::ALLTHREADING || this->structureOptions_ == StructureOptions::ALL) {
      // Print an edge from the cluster "egg" node to the task
      std::ostringstream ss;
      ss << "box" << clusterNode->coreClusterNode()->id() << " -> " << clusterNode->id();
      edges_.push_back(ss.str());
    }
  }

  /// @brief Print the execution pipeline header
  /// @param epNode Execution pipeline to print
  /// @param switchNode Switch bound to the execution pipeline
  void printExecutionPipelineHeader(core::CoreNode *epNode, core::CoreNode *switchNode) override {
    //Print the dot subgraph header
    outputFile_ << "subgraph cluster" << epNode->id() << " {\nlabel=\"" << epNode->name();
    if (debugOptions_ == DebugOptions::ALL) { outputFile_ << " " << epNode->id() << " / " << switchNode->id(); }
    // Print hedgehog "triangle" node to represent the execution pipeline switch
    outputFile_ << "\"; penwidth=1; style=dotted; style=filled; fillcolor=gray80;\n "
                << switchNode->id() << "[label=\"\", shape=triangle];\n";
    outputFile_.flush();
  }

  /// @brief Print the footer for the execution pipeline
  void printExecutionPipelineFooter() override {
    outputFile_ << "}\n";
    outputFile_.flush();
  }

  /// @brief Print an edge between the switch and hedgehog graph's input node
  /// @param to Graph's input node
  /// @param idSwitch Switch id
  /// @param edgeType Edge's type
  /// @param queueSize Current queue size
  /// @param maxQueueSize Current max queue size
  /// @param isMemoryManaged Flag to tag if the data is memory managed
  void printEdgeSwitchGraphs(core::CoreNode *to,
                             std::string const &idSwitch,
                             std::string_view const &edgeType,
                             size_t const &queueSize,
                             size_t const &maxQueueSize,
                             bool isMemoryManaged) override {
    std::ostringstream
        oss;

    std::string
        idDest,
        headLabel,
        penWidth = ",penwidth=1",
        queueStr;

    // If the queue information need to be printed
    if (this->structureOptions_ == StructureOptions::QUEUE || this->structureOptions_ == StructureOptions::ALL) {
      oss << " QS:" << queueSize << " MQS:" << maxQueueSize;
      queueStr = oss.str();
      oss.str("");
    }

    // Change the width of the edge if the data is memory managed
    if (isMemoryManaged) { penWidth = ",penwidth=3"; }

    // If all nodes in hedgehog cluster need to be printed
    if (this->structureOptions_ == StructureOptions::ALLTHREADING || this->structureOptions_ == StructureOptions::ALL) {
      // If the node is part of hedgehog cluster
      if (to->isInCluster()) {
        // Loop over all input node of the current node
        // (could be simply itself or, all input nodes in the case of graph)
        for (auto &dest: to->ids()) {
          // Create cluster id (dest.first is the input node, dest.second is the reference to the cluster main node)
          idDest = "box" + dest.second;
          // Create the edge
          oss << idSwitch << " -> " << idDest << "[label=\"" << edgeType << queueStr << "\""
              << penWidth << "];";
        }
        // If the node is not part of hedgehog cluster
      } else {
        // Crete the edge
        oss << idSwitch << " -> " << to->id() << "[label=\"" << edgeType << queueStr << "\"" << penWidth << "];";
      }
      // If all nodes should not be printed but node is the main cluster node
    } else if (to->id() == to->coreClusterNode()->id()) {
      // Create the edge
      oss << idSwitch << " -> " << to->id() << "[label=\"" << edgeType << queueStr << "\"" << penWidth << "];";
    }
    edges_.push_back(oss.str());
  }

  /// @brief Create an edge between from and to
  /// @param from Edge origin node
  /// @param to Edge destination node
  /// @param edgeType Edge type
  /// @param queueSize Current queue size
  /// @param maxQueueSize Current maximum queue size
  /// @param isMemoryManaged Flag to determine if the edge data is memory managed
  void printEdge(core::CoreNode const *from, core::CoreNode const *to, std::string_view const &edgeType,
                 size_t const &queueSize, size_t const &maxQueueSize,
                 bool isMemoryManaged) final {
    std::ostringstream
        oss;

    std::string
        headLabel,
        tailLabel,
        idDest,
        queueStr,
        penWidth = ",penwidth=1";

    // Change the width of the edge if the data is memory managed
    if (isMemoryManaged) { penWidth = ",penwidth=3"; }

    // If the queue information need to be printed, add queue information
    if (this->structureOptions_ == StructureOptions::QUEUE || this->structureOptions_ == StructureOptions::ALL) {
      oss << " QS:" << queueSize << " MQS:" << maxQueueSize;
      queueStr = oss.str();
      oss.str("");
    }

    // If all nodes in hedgehog cluster need to be printed
    if (this->structureOptions_ == StructureOptions::ALLTHREADING || this->structureOptions_ == StructureOptions::ALL) {
      // If the source node is in hedgehog cluster
      if (from->isInCluster()) {
        // Loop over all source node of the current node
        // (could be simply itself or, all input nodes in the case of graph)
        for (auto &source: from->ids()) {
          headLabel = ",ltail=cluster" + source.second;
          // If the destination node is in hedgehog cluster
          if (to->isInCluster()) {
            // Loop over all destination node of the current node
            // (could be simply itself or, all input nodes in the case of graph)
            for (auto &dest : to->ids()) {
              tailLabel = ",lhead=cluster" + dest.second;
              idDest = "box" + dest.second;
              oss << source.first << " -> " << idDest << "[label=\"" << edgeType << queueStr << "\"" << headLabel
                  << tailLabel << penWidth << "];";
            }
            // If the destination node is NOT in hedgehog cluster
          } else {
            oss << source.first << " -> " << to->id() << "[label=\"" << edgeType << queueStr << "\"" << headLabel
                << penWidth
                << "];";
          }
        }
        // If the source node is NOT in hedgehog cluster
      } else {
        // If the destination node is in hedgehog cluster
        if (to->isInCluster()) {
          // Loop over all destination node of the current node
          // (could be simply itself or, all input nodes in the case of graph)
          for (auto &dest : to->ids()) {
            tailLabel = ",lhead=cluster" + dest.second;
            idDest = "box" + dest.second;
            oss << from->id() << " -> " << idDest << "[label=\"" << edgeType << queueStr << "\"" << headLabel
                << tailLabel << penWidth << "];";
          }
          // If the destination node is NOT in hedgehog cluster
        } else {
          oss << from->id() << " -> " << to->id() << "[label=\"" << edgeType << queueStr << "\"" << headLabel
              << penWidth
              << "];";
        }
      }
      edges_.push_back(oss.str());
      // If all nodes should NOT be printed and, from and to are the main nodes of their cluster
    } else if (from->id() == from->coreClusterNode()->id() && to->id() == to->coreClusterNode()->id()) {
      oss << from->id() << " -> " << to->id() << "[label=\"" << edgeType << queueStr << "\"" << penWidth << "];";
      edges_.push_back(oss.str());
    }
  }

 private:
  /// @brief Extract and create node information
  /// @param node Node to exploit
  /// @return std::string with node information
  std::string getNodeInformation(core::CoreNode *node) {
    std::stringstream ss;

    // Print the name
    ss << node->id() << " [label=\"" << node->name();
    // Print the id (adress) in case of debug
    if (debugOptions_ == DebugOptions::ALL) {
      ss << " " << node->id() << " \\(" << node->threadId() << ", " << node->graphId() << "\\)";
    }

    switch (node->type()) {
      // Set hedgehog specific shape for the source
      case core::NodeType::Source:ss << "\", shape=doublecircle";
        break;
        // Set hedgehog specific shape for the sink
      case core::NodeType::Sink:ss << "\",shape=point";
        break;
        // For the execution task
      case core::NodeType::Task:
        // If the cluster has to be presented as hedgehog single dot node
        if (!(this->structureOptions_ == StructureOptions::ALLTHREADING
            || this->structureOptions_ == StructureOptions::ALL)) {
          if (node->isInCluster()) {
            ss << " x " << node->numberThreads();
          }
        }
        // If debug information printed
        if (debugOptions_ == DebugOptions::ALL) {
          // Print number of active input connection
          ss << "\\nActive input connection: " << dynamic_cast<core::CoreSlot *>(node)->numberInputNodes();
          // If all nodes in hedgehog cluster need to be printed
          if (this->structureOptions_ == StructureOptions::ALLTHREADING
              || this->structureOptions_ == StructureOptions::ALL) {
            ss << "\\nThread Active?: " << std::boolalpha << dynamic_cast<core::CoreSlot *>(node)->isActive();
            // If all nodes in hedgehog cluster should NOT be printed
          } else {
            ss << "\\nActive threads: " << dynamic_cast<core::CoreSlot *>(node)->numberActiveThreadInCluster();
          }
        }
        // If all nodes in hedgehog cluster need to be printed
        if (this->structureOptions_ == StructureOptions::ALLTHREADING
            || this->structureOptions_ == StructureOptions::ALL) {
          ss << "\\nNumber Elements Received: " << node->numberReceivedElements();
          ss << "\\nWait Time: " << durationPrinter(node->waitTime());
          ss << "\\nDequeue + Execution Time: " << durationPrinter(node->executionTime());
          ss << "\\nExecution Time Per Element: " << durationPrinter(node->executionTimePerElement());
          if (node->hasMemoryManagerAttached()) {
            ss << "\\nMemory Wait Time: " << durationPrinter(node->memoryWaitTime());
          }
          // If all nodes in hedgehog cluster should NOT be printed
        } else {
          //Get the time in the clusters
          auto minmaxWait = node->minmaxWaitTimeCluster();
          auto minmaxExec = node->minmaxExecTimeCluster();
          auto minmaxElements = node->minmaxNumberElementsReceivedCluster();
          auto minmaxExePerElement = node->minmaxExecTimePerElementCluster();
          auto minmaxMemoryWait = node->minmaxMemoryWaitTimeCluster();
          // Print the number of element received per task
          ss << "\\nNumber of Elements Received Per Task: ";
          if (node->numberThreads() > 1) {
            ss << "\\n"
               << "  Min: " << minmaxElements.first << "\\n"
               << "  Avg: " << std::setw(3) << node->meanNumberElementsReceivedCluster()
               << " +- " << std::setw(3) << node->stdvNumberElementsReceivedCluster()
               << "\\n"
               << "  Max: " << std::setw(3) << minmaxElements.second << "\\n";
          } else {
            ss << node->meanNumberElementsReceivedCluster() << "\\n";
          }
          // Print the wait time
          ss << "Wait Time: ";
          if (node->numberThreads() > 1) {
            ss << "\\n"
               << "  Min: " << durationPrinter(minmaxWait.first) << "\\n"
               << "  Avg: " << durationPrinter(node->meanWaitTimeCluster()) << " +- "
               << durationPrinter(node->stdvWaitTimeCluster()) << "\\n"
               << "  Max: " << durationPrinter(minmaxWait.second) << "\\n";
          } else {
            ss << durationPrinter(node->meanWaitTimeCluster()) << "\\n";
          }
          // Print the execution time
          ss << "Dequeue + Execution Time: ";
          if (node->numberThreads() > 1) {
            ss << "\\n"
               << "  Min: " << durationPrinter(minmaxExec.first) << "\\n"
               << "  Avg: " << durationPrinter(node->meanExecTimeCluster()) << " +- "
               << durationPrinter(node->stdvExecTimeCluster()) << "\\n"
               << "  Max: " << durationPrinter(minmaxExec.second) << "\\n";
          } else {
            ss << durationPrinter(node->meanExecTimeCluster()) << "\\n";
          }

          // Print the execution time per Element
          ss << "Execution Time Per Element: ";
          if (node->numberThreads() > 1) {
            ss << "\\n"
               << "  Min: " << durationPrinter(minmaxExePerElement.first) << "\\n"
               << "  Avg: " << durationPrinter(node->meanExecTimePerElementCluster()) << " +- "
               << durationPrinter(node->stdvExecPerElementTimeCluster()) << "\\n"
               << "  Max: " << durationPrinter(minmaxExePerElement.second) << "\\n";
          } else {
            ss << durationPrinter(node->meanExecTimePerElementCluster()) << "\\n";
          }

          // Print the memory wait time
          if (node->hasMemoryManagerAttached()) {
            ss << "Memory Wait Time: ";
            if (node->numberThreads() > 1) {
              ss << "\\n"
                 << "  Min: " << durationPrinter(minmaxMemoryWait.first) << "\\n"
                 << "  Avg: " << durationPrinter(node->meanMemoryWaitTimeCluster()) << " +-"
                 << durationPrinter(node->stdvMemoryWaitTimeCluster()) << "\\n"
                 << "  Max: " << durationPrinter(minmaxMemoryWait.second) << "\\n";
            } else {
              ss << durationPrinter(node->meanMemoryWaitTimeCluster()) << "\\n";
            }
          }
        }
        // If extra information has been defined by the user, print it
        if (!node->extraPrintingInformation().empty()) {
          ss << "\\n" << node->extraPrintingInformation();
        }
        ss << "\"";
        ss << ",shape=circle";
        // Change the color of the circle depending on the user choice
        switch (this->colorScheme_) {
          case ColorScheme::EXECUTION:ss << ",color=" << this->getExecRGB(node->executionTime()) << ", penwidth=3";
            break;
          case ColorScheme::WAIT:ss << ",color=" << this->getWaitRGB(node->waitTime()) << ", penwidth=3";
            break;
          default:break;
        }

        if (node->isCudaRelated()) { ss << R"(, style=filled, fillcolor="#76b900", fontcolor="#8946ff")"; }
        break;
        // Print state manager
      case core::NodeType::StateManager:
        if (debugOptions_ == DebugOptions::ALL) {
          ss << "\\nActive input connection: " << dynamic_cast<core::CoreSlot *>(node)->numberInputNodes();
          ss << "\\nActive threads: " << dynamic_cast<core::CoreSlot *>(node)->numberActiveThreadInCluster();
        }
        ss << "\\nNumber Elements Received: " << node->numberReceivedElements();
        ss << "\\nWait Time: " << durationPrinter(node->waitTime());
        ss << "\\nDequeue + Execution Time: " << durationPrinter(node->executionTime());
        ss << "\\nExecution Time Per Element: " << durationPrinter(node->executionTimePerElement());
        ss << "\"";
        ss << ",shape=diamond";
        // Change the color of the circle depending on the user choice
        switch (this->colorScheme_) {
          case ColorScheme::EXECUTION:ss << ",color=" << this->getExecRGB(node->executionTime()) << ", penwidth=3";
            break;
          case ColorScheme::WAIT:ss << ",color=" << this->getWaitRGB(node->waitTime()) << ", penwidth=3";
            break;
          default:break;
        }
        break;
      default:break;
    }
    ss << "];\n";

    return ss.str();
  }

  /// @brief Return hedgehog RGB color for hedgehog value knowing the minimum value and the range, blue least, red highest
  /// @param ns Value to get the color
  /// @param min Minimum value
  /// @param range Range value
  /// @return RGB color associated with the value
  static std::string getRGBFromRange(std::chrono::nanoseconds const &ns, std::chrono::nanoseconds const &min,
                                     std::chrono::nanoseconds const &range) {
    auto posRedToBlue = (uint64_t) std::round((double) (ns.count() - min.count()) / (double) range.count() * 255);
    posRedToBlue = std::clamp(posRedToBlue, (uint64_t) 0, (uint64_t) 255);
    std::stringstream ss;
    ss << "\"#"
       << std::setfill('0') << std::setw(2) << std::hex << posRedToBlue
       << "00"
       << std::setfill('0') << std::setw(2) << std::hex << 255 - posRedToBlue
       << "\"";

    HLOG(0, "PRINT RGB RANGE " << val << " " << min << " " << range << " " << posRedToBlue)
    return ss.str();
  }

  /// @brief Get the rgb color for the execution time value
  /// @param ns Execution value to get the RGB color
  /// @return RGB color for val
  std::string getExecRGB(std::chrono::nanoseconds const &ns) {
    return getRGBFromRange(ns, this->minExecutionTime_, this->rangeExecutionTime_);
  }

  /// @brief Get the rgb color for the wait time value
  /// @param ns Execution value to get the RGB color
  /// @return RGB color for val
  std::string getWaitRGB(std::chrono::nanoseconds const &ns) {
    return getRGBFromRange(ns, this->minWaitTime_, this->rangeWaitTime_);
  }

  /// @brief Print hedgehog duration with the good unit
  /// @param ns Duration to print
  /// @return std::string with the duration and the unit
  static std::string durationPrinter(std::chrono::nanoseconds const &ns) {
    std::ostringstream oss;

    // Cast with precision loss
    auto s = std::chrono::duration_cast<std::chrono::seconds>(ns);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(ns);
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(ns);

    if (s > std::chrono::seconds::zero()) {
      oss << s.count() << "." << std::setfill('0') << std::setw(3) << (ms - s).count() << "s";
    } else if (ms > std::chrono::milliseconds::zero()) {
      oss << ms.count() << "." << std::setfill('0') << std::setw(3) << (us - ms).count() << "ms";
    } else if (us > std::chrono::microseconds::zero()) {
      oss << us.count() << "." << std::setfill('0') << std::setw(3) << (ns - us).count() << "us";
    } else {
      oss << std::setw(3) << ns.count() << "ns";
    }
    return oss.str();
  }
};
}
#endif //HEDGEHOG_DOT_PRINTER_H