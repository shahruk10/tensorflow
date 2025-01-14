/* Copyright 2021 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#ifndef TENSORFLOW_CORE_PROFILER_LIB_SCOPED_MEMORY_DEBUG_ANNOTATION_H_
#define TENSORFLOW_CORE_PROFILER_LIB_SCOPED_MEMORY_DEBUG_ANNOTATION_H_

#include <cstdint>
#include <functional>
#include <string>

namespace tensorflow {
namespace profiler {

// Annotations for memory profiling and debugging purpose.
// ScopedMemoryDebugAnnotation will cache the annotations in thread-local
// memory, and some allocators will try to tag allocations with the annotations.
struct MemoryDebugAnnotation {
  const char* pending_op_name = nullptr;
  int64_t pending_step_id = 0;
  const char* pending_region_type = nullptr;
  int32_t pending_data_type = 0;
  // A lambda function, when invoked, it will generate the string that describe
  // the shape of the pending tensor. By default, the TensorShape string is an
  // empty string.
  std::function<std::string()> pending_shape_func = []() { return ""; };
};

// Wrapper class of MemoryDebugAnnotation for RAII.
class ScopedMemoryDebugAnnotation {
 public:
  static const MemoryDebugAnnotation& CurrentAnnotation() {
    return annotation_;
  }

  explicit ScopedMemoryDebugAnnotation(const char* op_name) {
    last_annotation_ = annotation_;
    annotation_ = MemoryDebugAnnotation();
    annotation_.pending_op_name = op_name;
  }

  explicit ScopedMemoryDebugAnnotation(const char* op_name, int64_t step_id) {
    last_annotation_ = annotation_;
    annotation_ = MemoryDebugAnnotation();
    annotation_.pending_op_name = op_name;
    annotation_.pending_step_id = step_id;
  }

  // This constructor keeps the pending_op_name and pending_step_id from parent
  // (if any).  Otherwise it overwrites with op_name.
  explicit ScopedMemoryDebugAnnotation(
      const char* op_name, const char* region_type, int32_t data_type,
      const std::function<std::string()>& pending_shape_func) {
    last_annotation_ = annotation_;
    if (!annotation_.pending_op_name) {
      annotation_.pending_op_name = op_name;
    }
    annotation_.pending_region_type = region_type;
    annotation_.pending_data_type = data_type;
    annotation_.pending_shape_func = pending_shape_func;
  }

  explicit ScopedMemoryDebugAnnotation(
      const char* op_name, int64_t step_id, const char* region_type,
      int32_t data_type,
      const std::function<std::string()>& pending_shape_func) {
    last_annotation_ = annotation_;
    annotation_.pending_op_name = op_name;
    annotation_.pending_step_id = step_id;
    annotation_.pending_region_type = region_type;
    annotation_.pending_data_type = data_type;
    annotation_.pending_shape_func = pending_shape_func;
  }

  ~ScopedMemoryDebugAnnotation() { annotation_ = last_annotation_; }

 private:
  // Stores the current annotations.
  static thread_local MemoryDebugAnnotation annotation_;

  // Stores the previous values in case the annotations are nested.
  MemoryDebugAnnotation last_annotation_;

  ScopedMemoryDebugAnnotation(const ScopedMemoryDebugAnnotation&) = delete;
  ScopedMemoryDebugAnnotation& operator=(const ScopedMemoryDebugAnnotation&) =
      delete;
};

}  // namespace profiler
}  // namespace tensorflow

#endif  // TENSORFLOW_CORE_PROFILER_LIB_SCOPED_MEMORY_DEBUG_ANNOTATION_H_
