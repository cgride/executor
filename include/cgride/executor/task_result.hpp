/**
 *
 *  @file task_result.hpp
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
#ifndef CGRIDE_EXECUTOR_TASK_RESULT_HPP
#define CGRIDE_EXECUTOR_TASK_RESULT_HPP

#include <chrono>
#include <optional>
#include <string>
#include <string_view>

#include <cgride/core/error.hpp>
#include <cgride/executor/process_result.hpp>
#include <cgride/graph/task_id.hpp>

namespace cgride::executor
{
  /**
   * @enum TaskStatus
   * @brief Final or intermediate state of a graph task execution.
   */
  enum class TaskStatus
  {
    Pending,
    Started,
    Skipped,
    Succeeded,
    Failed,
    Cancelled
  };

  /**
   * @brief Convert a task status to a stable string.
   *
   * @param status Task status.
   * @return Stable string representation.
   */
  [[nodiscard]] std::string_view to_string(TaskStatus status) noexcept;

  /**
   * @class TaskResult
   * @brief Structured result of one graph task execution.
   *
   * TaskResult stores the observable result of executing one task:
   * - task id
   * - optional task name
   * - task status
   * - optional message
   * - optional process result
   * - optional structured error
   * - task duration
   *
   * It does not execute tasks by itself.
   */
  class TaskResult
  {
  public:
    /**
     * @brief Construct an empty pending task result.
     */
    TaskResult() = default;

    /**
     * @brief Construct a pending task result for a task id.
     *
     * @param task_id Task id.
     */
    explicit TaskResult(cgride::graph::TaskId task_id);

    /**
     * @brief Create a pending task result.
     *
     * @param task_id Task id.
     * @return Task result.
     */
    [[nodiscard]] static TaskResult pending(cgride::graph::TaskId task_id);

    /**
     * @brief Create a started task result.
     *
     * @param task_id Task id.
     * @return Task result.
     */
    [[nodiscard]] static TaskResult started(cgride::graph::TaskId task_id);

    /**
     * @brief Create a skipped task result.
     *
     * @param task_id Task id.
     * @param message Human-readable skip reason.
     * @param duration Task duration.
     * @return Task result.
     */
    [[nodiscard]] static TaskResult skipped(
        cgride::graph::TaskId task_id,
        std::string message = {},
        std::chrono::milliseconds duration = std::chrono::milliseconds{0});

    /**
     * @brief Create a successful task result without a process result.
     *
     * @param task_id Task id.
     * @param duration Task duration.
     * @return Task result.
     */
    [[nodiscard]] static TaskResult succeeded(
        cgride::graph::TaskId task_id,
        std::chrono::milliseconds duration = std::chrono::milliseconds{0});

    /**
     * @brief Create a successful task result with a process result.
     *
     * @param task_id Task id.
     * @param process_result Process result.
     * @return Task result.
     */
    [[nodiscard]] static TaskResult succeeded(
        cgride::graph::TaskId task_id,
        ProcessResult process_result);

    /**
     * @brief Create a failed task result from a structured error.
     *
     * @param task_id Task id.
     * @param error Structured error.
     * @param duration Task duration.
     * @return Task result.
     */
    [[nodiscard]] static TaskResult failed(
        cgride::graph::TaskId task_id,
        cgride::core::Error error,
        std::chrono::milliseconds duration = std::chrono::milliseconds{0});

    /**
     * @brief Create a failed task result from a process result.
     *
     * @param task_id Task id.
     * @param process_result Process result.
     * @return Task result.
     */
    [[nodiscard]] static TaskResult failed(
        cgride::graph::TaskId task_id,
        ProcessResult process_result);

    /**
     * @brief Create a cancelled task result.
     *
     * @param task_id Task id.
     * @param error Structured cancellation error.
     * @return Task result.
     */
    [[nodiscard]] static TaskResult cancelled(
        cgride::graph::TaskId task_id,
        cgride::core::Error error = cgride::core::Error(
            cgride::core::ErrorCode::Cancelled,
            "Task execution was cancelled."));

    /**
     * @brief Set the task id.
     *
     * @param task_id Task id.
     * @return Reference to this result.
     */
    TaskResult &task_id(cgride::graph::TaskId task_id);

    /**
     * @brief Set the human-readable task name.
     *
     * @param name Task name.
     * @return Reference to this result.
     */
    TaskResult &task_name(std::string name);

    /**
     * @brief Set the task status.
     *
     * @param status Task status.
     * @return Reference to this result.
     */
    TaskResult &status(TaskStatus status) noexcept;

    /**
     * @brief Set a human-readable message.
     *
     * @param message Result message.
     * @return Reference to this result.
     */
    TaskResult &message(std::string message);

    /**
     * @brief Set the process result attached to this task result.
     *
     * @param result Process result.
     * @return Reference to this result.
     */
    TaskResult &process_result(ProcessResult result);

    /**
     * @brief Set a structured task error.
     *
     * @param error Structured error.
     * @return Reference to this result.
     */
    TaskResult &error(cgride::core::Error error);

    /**
     * @brief Set task duration.
     *
     * @param duration Task duration.
     * @return Reference to this result.
     */
    TaskResult &duration(std::chrono::milliseconds duration) noexcept;

    /**
     * @brief Access the task id.
     */
    [[nodiscard]] const cgride::graph::TaskId &task_id() const noexcept;

    /**
     * @brief Access the human-readable task name.
     */
    [[nodiscard]] const std::string &task_name() const noexcept;

    /**
     * @brief Access the task status.
     */
    [[nodiscard]] TaskStatus status() const noexcept;

    /**
     * @brief Access the result message.
     */
    [[nodiscard]] const std::string &message() const noexcept;

    /**
     * @brief Access the optional process result.
     */
    [[nodiscard]] const std::optional<ProcessResult> &process_result() const noexcept;

    /**
     * @brief Return true when a process result is attached.
     */
    [[nodiscard]] bool has_process_result() const noexcept;

    /**
     * @brief Access the optional structured error.
     */
    [[nodiscard]] const std::optional<cgride::core::Error> &error() const noexcept;

    /**
     * @brief Return true when a structured error is attached.
     */
    [[nodiscard]] bool has_error() const noexcept;

    /**
     * @brief Access task duration.
     */
    [[nodiscard]] std::chrono::milliseconds duration() const noexcept;

    /**
     * @brief Return true when the task id is usable.
     */
    [[nodiscard]] bool valid() const noexcept;

    /**
     * @brief Return true when the task succeeded.
     */
    [[nodiscard]] bool success() const noexcept;

    /**
     * @brief Return true when the task failed or was cancelled.
     */
    [[nodiscard]] bool failed() const noexcept;

    /**
     * @brief Return true when the task reached a final state.
     */
    [[nodiscard]] bool finished() const noexcept;

  private:
    cgride::graph::TaskId task_id_{};
    std::string task_name_{};
    TaskStatus status_{TaskStatus::Pending};
    std::string message_{};
    std::optional<ProcessResult> process_result_{};
    std::optional<cgride::core::Error> error_{};
    std::chrono::milliseconds duration_{0};
  };

} // namespace cgride::executor

#endif // CGRIDE_EXECUTOR_TASK_RESULT_HPP
