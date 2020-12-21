/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/*!
 * \file runtime/crt/logging.h
 * \brief A replacement of the dmlc logging system that avoids
 *  the usage of GLOG and C++ headers
 */

#ifndef TVM_RUNTIME_CRT_LOGGING_H_
#define TVM_RUNTIME_CRT_LOGGING_H_

#include <tvm/runtime/crt/platform.h>

#define TVM_CRT_LOG_LEVEL_DEBUG 3
#define TVM_CRT_LOG_LEVEL_INFO 2
#define TVM_CRT_LOG_LEVEL_WARN 1
#define TVM_CRT_LOG_LEVEL_ERROR 0

#ifdef __cplusplus
extern "C" {
#endif

void __attribute__((format(printf, 1, 2))) TVMLogf(const char* fmt, ...);

#define TVM_LOG(level, x, ...)          \
  if (TVM_CRT_LOG_LEVEL >= level) { \
    TVMLogf(x, ##__VA_ARGS__);      \
  }

#define LOG_ERROR(x, ...) TVM_LOG(TVM_CRT_LOG_LEVEL_ERROR, x, ##__VA_ARGS__)
#define LOG_WARN(x, ...) TVM_LOG(TVM_CRT_LOG_LEVEL_WARN, x, ##__VA_ARGS__)
#define LOG_INFO(x, ...) TVM_LOG(TVM_CRT_LOG_LEVEL_INFO, x, ##__VA_ARGS__)
#define LOG_DEBUG(x, ...) TVM_LOG(TVM_CRT_LOG_LEVEL_DEBUG, x, ##__VA_ARGS__)

#ifndef TVM_CHECK
#define TVM_CHECK(x)                                                   \
  do {                                                             \
    if (!(x)) {                                                    \
      LOG_ERROR(__FILE__ ":%d: Check failed: %s\n", __LINE__, #x); \
      TVMPlatformAbort(kTvmErrorPlatformCheckFailure);             \
    }                                                              \
  } while (0)
#endif

#ifndef TVM_CHECK_BINARY_OP
#define TVM_CHECK_BINARY_OP(op, x, y, fmt, ...)                                               \
  do {                                                                                    \
    if (!(x op y)) {                                                                      \
      LOG_ERROR(__FILE__ ":%d: Check failed: %s %s %s: " fmt "\n", __LINE__, #x, #op, #y, \
                ##__VA_ARGS__);                                                           \
      TVMPlatformAbort(kTvmErrorPlatformCheckFailure);                                    \
    }                                                                                     \
  } while (0)
#endif

#ifndef TVM_CHECK_LT
#define TVM_CHECK_LT(x, y, fmt, ...) TVM_CHECK_BINARY_OP(<, x, y, fmt, ##__VA_ARGS__)
#endif

#ifndef TVM_CHECK_GT
#define TVM_CHECK_GT(x, y, fmt, ...) TVM_CHECK_BINARY_OP(>, x, y, fmt, ##__VA_ARGS__)
#endif

#ifndef TVM_CHECK_LE
#define TVM_CHECK_LE(x, y, fmt, ...) TVM_CHECK_BINARY_OP(<=, x, y, fmt, ##__VA_ARGS__)
#endif

#ifndef TVM_CHECK_GE
#define TVM_CHECK_GE(x, y, fmt, ...) TVM_CHECK_BINARY_OP(>=, x, y, fmt, ##__VA_ARGS__)
#endif

#ifndef TVM_CHECK_EQ
#define TVM_CHECK_EQ(x, y, fmt, ...) TVM_CHECK_BINARY_OP(==, x, y, fmt, ##__VA_ARGS__)
#endif

#ifndef TVM_CHECK_NE
#define TVM_CHECK_NE(x, y, fmt, ...) TVM_CHECK_BINARY_OP(!=, x, y, fmt, ##__VA_ARGS__)
#endif

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // TVM_RUNTIME_CRT_LOGGING_H_
