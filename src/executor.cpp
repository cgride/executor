/**
 *
 *  @file executor.cpp
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
#include <cgride/executor/executor.hpp>

#include <chrono>
#include <filesystem>
#include <optional>
#include <system_error>
#include <string>
#include <unordered_set>
#include <utility>

#include <cgride/core/error.hpp>
#include <cgride/core/event.hpp>
#include <cgride/executor/process.hpp>
#include <cgride/graph/topology.hpp>

namespace cgride::executor
{
  namespace
  {
    using cgride::core::Error;
    using cgride::core::ErrorCode;
    using cgride::core::Event;
    using cgride::core::EventKind;
    using cgride::core::OutputStream;

    [[nodiscard]] std::string task_display_name(const cgride::graph::Task &task)
    {
      if (!task.name().empty())
      {
        return task.name();
      }

      return std::string(task.id().value());
    }

    void emit_build_started(
        const cgride::graph::Graph &graph,
        const ExecutionOptions &options)
    {
      options.emit_event(
          Event(
              EventKind::BuildStarted,
              graph.name().empty()
                  ? "Graph execution started."
                  : "Graph execution started: " + graph.name()));
    }

    void emit_build_finished(
        const cgride::graph::Graph &graph,
        const ExecutionOptions &options,
        bool success)
    {
      options.emit_event(
          Event(
              EventKind::BuildFinished,
              graph.name().empty()
                  ? (success ? "Graph execution finished." : "Graph execution failed.")
                  : (success ? "Graph execution finished: " + graph.name()
                             : "Graph execution failed: " + graph.name())));
    }

    void emit_task_started(
        const cgride::graph::Task &task,
        const ExecutionOptions &options)
    {
      options.emit_event(
          Event(
              EventKind::TaskStarted,
              "Task started: " + task_display_name(task))
              .task_id(std::string(task.id().value())));
    }

    void emit_task_finished(
        const TaskResult &result,
        const ExecutionOptions &options)
    {
      options.emit_event(
          Event(
              result.success() ? EventKind::TaskFinished : EventKind::TaskSkipped,
              "Task finished: " + std::string(result.task_id().value()) +
                  " (" + std::string(to_string(result.status())) + ")")
              .task_id(std::string(result.task_id().value())));
    }

    void emit_process_output(
        const TaskResult &result,
        const ExecutionOptions &options)
    {
      if (!result.has_process_result())
      {
        return;
      }

      const auto &process = result.process_result().value();

      if (!process.standard_output().empty())
      {
        options.emit_event(
            Event(
                EventKind::ProcessOutput,
                process.standard_output())
                .task_id(std::string(result.task_id().value()))
                .stream(OutputStream::Stdout));
      }

      if (!process.standard_error().empty())
      {
        options.emit_event(
            Event(
                EventKind::ProcessOutput,
                process.standard_error())
                .task_id(std::string(result.task_id().value()))
                .stream(OutputStream::Stderr));
      }
    }

    [[nodiscard]] bool has_failed_dependency(
        const cgride::graph::Task &task,
        const std::unordered_set<std::string> &failed_tasks,
        std::string &dependency)
    {
      for (const auto &id : task.dependencies())
      {
        if (failed_tasks.contains(id.value()))
        {
          dependency = id.value();
          return true;
        }
      }

      return false;
    }

    [[nodiscard]] TaskResult make_dependency_failed_result(
        const cgride::graph::Task &task,
        const std::string &dependency)
    {
      auto result = TaskResult::failed(
          task.id(),
          Error(
              ErrorCode::InvalidState,
              "Task dependency failed.",
              dependency));

      result.task_name(task.name());

      return result;
    }

    [[nodiscard]] TaskResult make_cancelled_result(
        const cgride::graph::Task &task)
    {
      auto result = TaskResult::cancelled(task.id());
      result.task_name(task.name());
      return result;
    }

    [[nodiscard]] TaskResult make_skipped_result(
        const cgride::graph::Task &task,
        std::chrono::milliseconds duration)
    {
      auto result = TaskResult::skipped(
          task.id(),
          "Task has no command.",
          duration);

      result.task_name(task.name());

      return result;
    }


    [[nodiscard]] std::optional<Error> ensure_output_directories(
        const cgride::graph::Task &task)
    {
      for (const auto &output : task.outputs())
      {
        const auto parent = output.parent_path();

        if (parent.empty())
        {
          continue;
        }

        std::error_code error_code;
        std::filesystem::create_directories(parent, error_code);

        if (error_code)
        {
          return Error(
              ErrorCode::IoError,
              "Cannot create task output directory.",
              error_code.message(),
              parent);
        }
      }

      return std::nullopt;
    }

    [[nodiscard]] TaskResult execute_one_task(
        const cgride::graph::Task &task,
        const ExecutionOptions &options)
    {
      const auto start = std::chrono::steady_clock::now();

      if (options.cancelled())
      {
        return make_cancelled_result(task);
      }

      emit_task_started(task, options);

      if (!task.has_command())
      {
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start);

        return make_skipped_result(task, duration);
      }

      auto output_directory_error = ensure_output_directories(task);

      if (output_directory_error.has_value())
      {
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start);

        auto result = TaskResult::failed(
            task.id(),
            output_directory_error.value(),
            duration);

        result.task_name(task.name());

        return result;
      }

      const auto process = run_process(task.command().value(), options);

      if (!process)
      {
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start);

        auto result = TaskResult::failed(
            task.id(),
            process.error(),
            duration);

        result.task_name(task.name());

        return result;
      }

      if (process.value().success())
      {
        auto result = TaskResult::succeeded(task.id(), process.value());
        result.task_name(task.name());
        return result;
      }

      auto result = TaskResult::failed(task.id(), process.value());
      result.task_name(task.name());
      return result;
    }

  } // namespace

  cgride::core::Result<TaskResults> Executor::execute(
      const cgride::graph::Graph &graph) const
  {
    ExecutionOptions options;
    return execute(graph, options);
  }

  cgride::core::Result<TaskResults> Executor::execute(
      const cgride::graph::Graph &graph,
      const ExecutionOptions &options) const
  {
    auto ordered_result = cgride::graph::topological_sort(graph);

    if (!ordered_result)
    {
      return ordered_result.error();
    }

    TaskResults results;
    results.reserve(ordered_result.value().size());

    std::unordered_set<std::string> failed_tasks;

    emit_build_started(graph, options);

    bool execution_success = true;

    for (const auto *task : ordered_result.value())
    {
      if (task == nullptr)
      {
        continue;
      }

      if (options.cancelled())
      {
        auto result = make_cancelled_result(*task);

        failed_tasks.insert(std::string(task->id().value()));
        results.push_back(std::move(result));
        execution_success = false;

        if (options.stop_on_failure())
        {
          break;
        }

        continue;
      }

      std::string failed_dependency;

      if (has_failed_dependency(*task, failed_tasks, failed_dependency))
      {
        auto result = make_dependency_failed_result(*task, failed_dependency);

        failed_tasks.insert(std::string(task->id().value()));
        results.push_back(std::move(result));
        execution_success = false;

        if (options.stop_on_failure())
        {
          break;
        }

        continue;
      }

      auto result = execute_one_task(*task, options);

      emit_process_output(result, options);
      emit_task_finished(result, options);

      if (result.failed())
      {
        failed_tasks.insert(std::string(task->id().value()));
        execution_success = false;

        results.push_back(std::move(result));

        if (options.stop_on_failure())
        {
          break;
        }

        continue;
      }

      results.push_back(std::move(result));
    }

    emit_build_finished(graph, options, execution_success);

    return results;
  }

  cgride::core::Result<TaskResults> execute_graph(
      const cgride::graph::Graph &graph)
  {
    Executor executor;
    return executor.execute(graph);
  }

  cgride::core::Result<TaskResults> execute_graph(
      const cgride::graph::Graph &graph,
      const ExecutionOptions &options)
  {
    Executor executor;
    return executor.execute(graph, options);
  }

} // namespace cgride::executor
