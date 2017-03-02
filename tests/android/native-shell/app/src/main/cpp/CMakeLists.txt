#
# Copyright (C)  The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

cmake_minimum_required(VERSION 3.4.1)

set(PROJ_ROOT "${CMAKE_SOURCE_DIR}/../../../../../../..")

# build native_app_glue as a static lib
add_library(app-glue STATIC
  ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

# now build app's shared lib
add_library(native-shell SHARED
            main.c)

target_include_directories(native-shell PRIVATE
  ${ANDROID_NDK}/sources/android/native_app_glue)

target_include_directories(native-shell PRIVATE
  "${PROJ_ROOT}/build/include")

add_library(js SHARED IMPORTED)
set_property(TARGET js PROPERTY IMPORTED_LOCATION
  ${PROJ_ROOT}/build/Android.prepackaged-libs/${ANDROID_ABI}/libjs.so)

# add lib dependencies
target_link_libraries(native-shell
                      android
                      app-glue
                      log
                      m
                      js)
