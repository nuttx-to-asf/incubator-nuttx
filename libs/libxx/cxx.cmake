############################################################################
# libs/libxx/cxx.defs
#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.  The
# ASF licenses this file to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance with the
# License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations
# under the License.
#
###########################################################################

target_sources(xx PRIVATE
  libxx_cxa_guard.cxx libxx_cxapurevirtual.cxx
  libxx_delete.cxx libxx_delete_sized.cxx libxx_deletea.cxx
  libxx_deletea_sized.cxx libxx_new.cxx libxx_newa.cxx
  libxx_stdthrow.cxx)

# Note: Our implementations of operator new are not conforming to
# the standard. (no bad_alloc implementation)
#
# libxx_new.cxx:64:11: error: 'operator new' is missing exception specification
#       'throw(std::bad_alloc)' [-Werror,-Wmissing-exception-spec]
# FAR void *operator new(std::size_t nbytes)
#           ^
#                                            throw(std::bad_alloc)

set_property(SOURCE libxx_new.cxx APPEND PROPERTY COMPILE_OPTIONS -Wno-missing-exception-spec)
set_property(SOURCE libxx_newa.cxx APPEND PROPERTY COMPILE_OPTIONS -Wno-missing-exception-spec)

target_include_directories(xx PUBLIC ${NUTTX_SOURCE_DIR}/include/cxx)
