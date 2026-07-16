/**
 *
 *  @file execution_options.hpp
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
#ifndef CGRIDE_EXECUTOR_EXECUTION_OPTIONS_HPP
#define CGRIDE_EXECUTOR_EXECUTION_OPTIONS_HPP

#include <chrono>
#include <cstddef>
#include <functional>
#include <optional>

#include <cgride/core/cancellation.hpp>
#include <cgride/core/event.hpp>

namespace cgride::executor
{
  /**
   * @class ExecutionOptions
   * @brief Runtime options used by process and graph execution.
   *
   * ExecutionOptions describes how execution should behave. It does not
   * execute commands, schedule tasks or print terminal output by itself.
   */
  class ExecutionOptions
  {
  public:
    /**
     * @brief Event callback type used by the executor.
     */
    using EventHandler = std::function<void(const cgride::core::Event &)>;

    /**
     * @brief Construct default execution options.
     */
    ExecutionOptions() = default;

    /**
     * @brief Set the requested worker count.
     *
     * A value lower than 1 is normalized to 1.
     *
     * @param value Requested worker count.
     * @return Reference to these options.
     */
    ExecutionOptions &jobs(std::size_t value) noexcept;

    /**
     * @brief Control whether execution stops after the first failure.
     *
     * @param value True to stop after first failure.
     * @return Reference to these options.
     */
    ExecutionOptions &stop_on_failure(bool value) noexcept;

    /**
     * @brief Control whether commands are reported without being executed.
     *
     * @param value True to avoid running commands.
     * @return Reference to these options.
     */
    ExecutionOptions &dry_run(bool value) noexcept;

    /**
     * @brief Control whether process stdout and stderr should be captured.
     *
     * @param value True to capture output.
     * @return Reference to these options.
     */
    ExecutionOptions &capture_output(bool value) noexcept;

    /**
     * @brief Control whether up-to-date task outputs may skip command execution.
     *
     * @param value True to skip tasks whose outputs are newer than inputs.
     * @return Reference to these options.
     */
    ExecutionOptions &skip_up_to_date(bool value) noexcept;

    /**
     * @brief Control whether child processes inherit the parent environment.
     *
     * This option may be combined with per-command environment overrides.
     *
     * @param value True to inherit the environment.
     * @return Reference to these options.
     */
    ExecutionOptions &inherit_environment(bool value) noexcept;

    /**
     * @brief Set a maximum process execution duration.
     *
     * @param value Timeout duration.
     * @return Reference to these options.
     */
    ExecutionOptions &timeout(std::chrono::milliseconds value) noexcept;

    /**
     * @brief Clear the configured process timeout.
     *
     * @return Reference to these options.
     */
    ExecutionOptions &clear_timeout() noexcept;

    /**
     * @brief Set the cancellation token used during execution.
     *
     * @param token Cancellation token.
     * @return Reference to these options.
     */
    ExecutionOptions &cancellation_token(cgride::core::CancellationToken token) noexcept;

    /**
     * @brief Set the event callback.
     *
     * @param handler Event handler.
     * @return Reference to these options.
     */
    ExecutionOptions &on_event(EventHandler handler);

    /**
     * @brief Access the requested worker count.
     */
    [[nodiscard]] std::size_t jobs() const noexcept;

    /**
     * @brief Return true if execution should stop after the first failure.
     */
    [[nodiscard]] bool stop_on_failure() const noexcept;

    /**
     * @brief Return true if commands should not be executed.
     */
    [[nodiscard]] bool dry_run() const noexcept;

    /**
     * @brief Return true if process output should be captured.
     */
    [[nodiscard]] bool capture_output() const noexcept;

    /**
     * @brief Return true if up-to-date tasks may be skipped.
     */
    [[nodiscard]] bool skip_up_to_date() const noexcept;

    /**
     * @brief Return true if child processes should inherit the parent environment.
     */
    [[nodiscard]] bool inherit_environment() const noexcept;

    /**
     * @brief Access the optional process timeout.
     */
    [[nodiscard]] const std::optional<std::chrono::milliseconds> &timeout() const noexcept;

    /**
     * @brief Return true when a process timeout is configured.
     */
    [[nodiscard]] bool has_timeout() const noexcept;

    /**
     * @brief Access the cancellation token.
     */
    [[nodiscard]] const cgride::core::CancellationToken &cancellation_token() const noexcept;

    /**
     * @brief Return true if cancellation was requested.
     */
    [[nodiscard]] bool cancelled() const noexcept;

    /**
     * @brief Return true if execution may continue.
     */
    [[nodiscard]] bool active() const noexcept;

    /**
     * @brief Return true when an event handler is configured.
     */
    [[nodiscard]] bool has_event_handler() const noexcept;

    /**
     * @brief Emit an event through the configured event handler.
     *
     * If no event handler is configured, this function does nothing.
     *
     * @param event Event to emit.
     */
    void emit_event(const cgride::core::Event &event) const;

  private:
    std::size_t jobs_{1};
    bool stop_on_failure_{true};
    bool dry_run_{false};
    bool capture_output_{true};
    bool skip_up_to_date_{true};
    bool inherit_environment_{true};
    std::optional<std::chrono::milliseconds> timeout_{};
    cgride::core::CancellationToken cancellation_token_{};
    EventHandler event_handler_{};
  };

} // namespace cgride::executor

#endif // CGRIDE_EXECUTOR_EXECUTION_OPTIONS_HPP
