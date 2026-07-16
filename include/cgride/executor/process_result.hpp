/**
 *
 *  @file process_result.hpp
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
#ifndef CGRIDE_EXECUTOR_PROCESS_RESULT_HPP
#define CGRIDE_EXECUTOR_PROCESS_RESULT_HPP

#include <chrono>
#include <optional>
#include <string>
#include <string_view>

#include <cgride/core/error.hpp>

namespace cgride::executor
{
  /**
   * @enum ProcessStatus
   * @brief Final state of a process execution attempt.
   */
  enum class ProcessStatus
  {
    NotStarted,
    Exited,
    FailedToStart,
    TimedOut,
    Cancelled
  };

  /**
   * @brief Convert a process status to a stable string.
   *
   * @param status Process status.
   * @return Stable string representation.
   */
  [[nodiscard]] std::string_view to_string(ProcessStatus status) noexcept;

  /**
   * @class ProcessResult
   * @brief Structured result of one process execution.
   *
   * ProcessResult stores the observable result of running a command:
   * - final status
   * - optional exit code
   * - captured standard output
   * - captured standard error
   * - optional structured error
   * - process duration
   *
   * It does not execute commands by itself.
   */
  class ProcessResult
  {
  public:
    /**
     * @brief Construct a result for a process that has not started.
     */
    ProcessResult() = default;

    /**
     * @brief Create a result for a process that exited normally.
     *
     * @param exit_code Native process exit code.
     * @param standard_output Captured stdout.
     * @param standard_error Captured stderr.
     * @param duration Execution duration.
     * @return Process result.
     */
    [[nodiscard]] static ProcessResult exited(
        int exit_code,
        std::string standard_output = {},
        std::string standard_error = {},
        std::chrono::milliseconds duration = std::chrono::milliseconds{0});

    /**
     * @brief Create a result for a process that failed to start.
     *
     * @param error Structured error.
     * @param standard_error Captured stderr or platform error text.
     * @return Process result.
     */
    [[nodiscard]] static ProcessResult failed_to_start(
        cgride::core::Error error,
        std::string standard_error = {});

    /**
     * @brief Create a result for a cancelled process.
     *
     * @param error Structured cancellation error.
     * @return Process result.
     */
    [[nodiscard]] static ProcessResult cancelled(
        cgride::core::Error error = cgride::core::Error(
            cgride::core::ErrorCode::Cancelled,
            "Process execution was cancelled."));

    /**
     * @brief Create a result for a timed-out process.
     *
     * @param error Structured timeout error.
     * @return Process result.
     */
    [[nodiscard]] static ProcessResult timed_out(
        cgride::core::Error error = cgride::core::Error(
            cgride::core::ErrorCode::Timeout,
            "Process execution timed out."));

    /**
     * @brief Set the process status.
     *
     * @param status Process status.
     * @return Reference to this result.
     */
    ProcessResult &status(ProcessStatus status) noexcept;

    /**
     * @brief Set the native exit code.
     *
     * @param exit_code Process exit code.
     * @return Reference to this result.
     */
    ProcessResult &exit_code(int exit_code) noexcept;

    /**
     * @brief Set captured standard output.
     *
     * @param output Captured stdout.
     * @return Reference to this result.
     */
    ProcessResult &standard_output(std::string output);

    /**
     * @brief Append captured standard output.
     *
     * @param output Captured stdout chunk.
     * @return Reference to this result.
     */
    ProcessResult &append_standard_output(std::string_view output);

    /**
     * @brief Set captured standard error.
     *
     * @param output Captured stderr.
     * @return Reference to this result.
     */
    ProcessResult &standard_error(std::string output);

    /**
     * @brief Append captured standard error.
     *
     * @param output Captured stderr chunk.
     * @return Reference to this result.
     */
    ProcessResult &append_standard_error(std::string_view output);

    /**
     * @brief Set a structured execution error.
     *
     * @param error Structured error.
     * @return Reference to this result.
     */
    ProcessResult &error(cgride::core::Error error);

    /**
     * @brief Set process duration.
     *
     * @param duration Execution duration.
     * @return Reference to this result.
     */
    ProcessResult &duration(std::chrono::milliseconds duration) noexcept;

    /**
     * @brief Access the process status.
     */
    [[nodiscard]] ProcessStatus status() const noexcept;

    /**
     * @brief Access the optional process exit code.
     */
    [[nodiscard]] const std::optional<int> &exit_code() const noexcept;

    /**
     * @brief Return true when an exit code is available.
     */
    [[nodiscard]] bool has_exit_code() const noexcept;

    /**
     * @brief Access captured standard output.
     */
    [[nodiscard]] const std::string &standard_output() const noexcept;

    /**
     * @brief Access captured standard error.
     */
    [[nodiscard]] const std::string &standard_error() const noexcept;

    /**
     * @brief Access the optional structured error.
     */
    [[nodiscard]] const std::optional<cgride::core::Error> &error() const noexcept;

    /**
     * @brief Return true when a structured error is available.
     */
    [[nodiscard]] bool has_error() const noexcept;

    /**
     * @brief Access process duration.
     */
    [[nodiscard]] std::chrono::milliseconds duration() const noexcept;

    /**
     * @brief Return true if the process exited with code 0.
     */
    [[nodiscard]] bool success() const noexcept;

    /**
     * @brief Return true if the process did not complete successfully.
     */
    [[nodiscard]] bool failed() const noexcept;

    /**
     * @brief Return true if the process reached a final state.
     */
    [[nodiscard]] bool finished() const noexcept;

  private:
    ProcessStatus status_{ProcessStatus::NotStarted};
    std::optional<int> exit_code_{};
    std::string standard_output_{};
    std::string standard_error_{};
    std::optional<cgride::core::Error> error_{};
    std::chrono::milliseconds duration_{0};
  };

} // namespace cgride::executor

#endif // CGRIDE_EXECUTOR_PROCESS_RESULT_HPP
