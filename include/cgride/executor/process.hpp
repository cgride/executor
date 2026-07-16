/**
 *
 *  @file process.hpp
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
#ifndef CGRIDE_EXECUTOR_PROCESS_HPP
#define CGRIDE_EXECUTOR_PROCESS_HPP

#include <string>

#include <cgride/core/command.hpp>
#include <cgride/core/result.hpp>
#include <cgride/executor/execution_options.hpp>
#include <cgride/executor/process_result.hpp>

namespace cgride::executor
{
  /**
   * @brief Convert a command to a readable command line.
   *
   * This function is intended for diagnostics and dry-run output only. It does
   * not perform shell parsing and should not be used to execute commands.
   *
   * @param command Command to describe.
   * @return Human-readable command line.
   */
  [[nodiscard]] std::string describe_command(const cgride::core::Command &command);

  /**
   * @brief Execute one command and capture its process result.
   *
   * The command is executed without using a shell. The returned value describes
   * the process status, exit code, output, error output and duration.
   *
   * @param command Command to execute.
   * @param options Execution options.
   * @return Process result or validation error.
   */
  [[nodiscard]] cgride::core::Result<ProcessResult> run_process(
      const cgride::core::Command &command,
      const ExecutionOptions &options);

  /**
   * @brief Execute one command with default execution options.
   *
   * @param command Command to execute.
   * @return Process result or validation error.
   */
  [[nodiscard]] cgride::core::Result<ProcessResult> run_process(
      const cgride::core::Command &command);

} // namespace cgride::executor

#endif // CGRIDE_EXECUTOR_PROCESS_HPP
