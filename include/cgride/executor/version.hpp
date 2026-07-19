/**
 *
 *  @file version.hpp
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
#ifndef CGRIDE_EXECUTOR_VERSION_HPP
#define CGRIDE_EXECUTOR_VERSION_HPP

#include <string_view>

namespace cgride::executor
{
  /**
   * @brief Major version of the Cgride executor module.
   */
  inline constexpr int version_major = 0;

  /**
   * @brief Minor version of the Cgride executor module.
   */
  inline constexpr int version_minor = 1;

  /**
   * @brief Patch version of the Cgride executor module.
   */
  inline constexpr int version_patch = 1;

  /**
   * @brief Public API version of the Cgride executor module.
   */
  inline constexpr int api_version = 1;

  /**
   * @brief Human-readable version string.
   */
  inline constexpr std::string_view version_string = "0.1.1";

} // namespace cgride::executor

#endif // CGRIDE_EXECUTOR_VERSION_HPP
