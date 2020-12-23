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
 * \file tvm/support/dmlc.h
 * \brief dmlc headers without polluting the global namespace.
 */

#ifndef TVM_SUPPORT_DMLC_H_
#define TVM_SUPPORT_DMLC_H_

#include <dmlc/common.h>
#include <dmlc/io.h>
#include <dmlc/json.h>
#include <dmlc/memory_io.h>
#include <dmlc/optional.h>
#include <dmlc/parameter.h>
#include <dmlc/serializer.h>
#include <dmlc/thread_local.h>

// Undefine common macros from dmlc so they are not accidentally used
#undef CHECK
#undef CHECK_EQ
#undef CHECK_NE
#undef CHECK_LT
#undef CHECK_LE
#undef CHECK_GT
#undef CHECK_GE
#undef CHECK_NOTNULL
#undef LOG
#undef LOG_IF
#undef DLOG
#undef DLOG_IF
#undef LOG_FATAL
#undef LOG_WARNING
#undef LOG_ERROR
#undef LOG_INFO

#include <tvm/support/logging.h> // so we don't undef our own logging macros

#endif
