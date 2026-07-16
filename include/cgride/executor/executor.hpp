/**
 *
 *  @file executor.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2026, Gaspard Kirira.
 *  All rights reserved.
 *  https://github.com/cgride/executor
 *
 *  Use of this source code is governed by a MIT license
 *  that can be found in the LICENSE file.
 *
 *  Cgride
 *
 */
#ifndef CGRIDE_EXECUTOR_EXECUTOR_HPP
#define CGRIDE_EXECUTOR_EXECUTOR_HPP

#include <vector>

#include <cgride/core/result.hpp>
#include <cgride/executor/execution_options.hpp>
#include <cgride/executor/task_result.hpp>
#include <cgride/graph/graph.hpp>

namespace cgride::executor
{
  /**
   * @brief Collection of task execution results.
   */
  using TaskResults = std::vector<TaskResult>;

  /**
   * @class Executor
   * @brief Executes a Cgride task graph.
   *
   * Executor runs graph tasks in dependency-first order. It does not create
   * build plans, discover toolchains, inspect cache state or print terminal
   * output directly.
   *
   * The first implementation is intentionally sequential. The public API keeps
   * a jobs option so a later parallel scheduler can be added without changing
   * the integration surface.
   */
  class Executor
  {
  public:
    /**
     * @brief Construct an executor.
     */
    Executor() = default;

    /**
     * @brief Execute a graph with default options.
     *
     * @param graph Graph to execute.
     * @return Task results or a validation error.
     */
    [[nodiscard]] cgride::core::Result<TaskResults> execute(
        const cgride::graph::Graph &graph) const;

    /**
     * @brief Execute a graph with explicit options.
     *
     * @param graph Graph to execute.
     * @param options Execution options.
     * @return Task results or a validation error.
     */
    [[nodiscard]] cgride::core::Result<TaskResults> execute(
        const cgride::graph::Graph &graph,
        const ExecutionOptions &options) const;
  };

  /**
   * @brief Execute a graph with default options.
   *
   * @param graph Graph to execute.
   * @return Task results or a validation error.
   */
  [[nodiscard]] cgride::core::Result<TaskResults> execute_graph(
      const cgride::graph::Graph &graph);

  /**
   * @brief Execute a graph with explicit options.
   *
   * @param graph Graph to execute.
   * @param options Execution options.
   * @return Task results or a validation error.
   */
  [[nodiscard]] cgride::core::Result<TaskResults> execute_graph(
      const cgride::graph::Graph &graph,
      const ExecutionOptions &options);

} // namespace cgride::executor

#endif // CGRIDE_EXECUTOR_EXECUTOR_HPP
