############################################################################
# Makefile
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
############################################################################
# Get a list of all config targets boards

ALL_CONFIG_TARGETS := $(shell tools/configure.sh -L | sed 's/:/-/g' | sort)

NINJA_BIN := ninja
ifndef NO_NINJA_BUILD
	NINJA_BUILD := $(shell $(NINJA_BIN) --version 2>/dev/null)

	ifndef NINJA_BUILD
		NINJA_BIN := ninja-build
		NINJA_BUILD := $(shell $(NINJA_BIN) --version 2>/dev/null)
	endif
endif

ifdef NINJA_BUILD
	NUTTX_CMAKE_GENERATOR := Ninja
	NUTTX_MAKE := $(NINJA_BIN)

	ifdef VERBOSE
		NUTTX_MAKE_ARGS := -v
	else
		NUTTX_MAKE_ARGS :=
	endif

	# Only override ninja default if -j is set.
	ifneq ($(j),)
		NUTTX_MAKE_ARGS := $(NUTTX_MAKE_ARGS) -j$(j)
	endif
else
	ifdef SYSTEMROOT
		# Windows
		NUTTX_CMAKE_GENERATOR := "MSYS\ Makefiles"
	else
		NUTTX_CMAKE_GENERATOR := "Unix\ Makefiles"
	endif

	# For non-ninja builds we default to -j4
	j := $(or $(j),4)
	NUTTX_MAKE = $(MAKE)
	NUTTX_MAKE_ARGS = -j$(j) --no-print-directory
endif

SRC_DIR := $(shell dirname "$(realpath $(lastword $(MAKEFILE_LIST)))")
NUTTX_BUILD_ROOT=$(SRC_DIR)/build
# --------------------------------------------------------------------
# describe how to build a cmake config
define cmake-build
	@$(eval BUILD_DIR = "$(NUTTX_BUILD_ROOT)/$(1)")
	@# check if the desired cmake configuration matches the cache then CMAKE_CACHE_CHECK stays empty
	@$(call cmake-cache-check)
	@# make sure to start from scratch when switching from GNU Make to Ninja
	@if [ $(NUTTX_CMAKE_GENERATOR) = "Ninja" ] && [ -e $(BUILD_DIR)/Makefile ]; then rm -rf $(BUILD_DIR); fi
	@# make sure to start from scratch if ninja build file is missing
	@if [ $(NUTTX_CMAKE_GENERATOR) = "Ninja" ] && [ ! -f $(BUILD_DIR)/build.ninja ]; then rm -rf $(BUILD_DIR); fi
	@# only excplicitly configure the first build, if cache file already exists the makefile will rerun cmake automatically if necessary
	@if [ ! -e $(BUILD_DIR)/CMakeCache.txt ] || [ $(CMAKE_CACHE_CHECK) ]; then \
		mkdir -p $(BUILD_DIR) \
		&& cd $(BUILD_DIR) \
		&& echo "cmake -S "$(SRC_DIR)" -B $(BUILD_DIR) -G"$(NUTTX_CMAKE_GENERATOR)" $(CMAKE_ARGS)" \
		&& cmake -S "$(SRC_DIR)" -B $(BUILD_DIR) -G"$(NUTTX_CMAKE_GENERATOR)" $(CMAKE_ARGS) \
		|| (rm -rf $(BUILD_DIR)); \
	fi
	@# run the build for the specified target
	@cmake --build $(BUILD_DIR) -- $(NUTTX_MAKE_ARGS) $(ARGS)
endef


# check if the options we want to build with in CMAKE_ARGS match the ones which are already configured in the cache inside BUILD_DIR
define cmake-cache-check
	@# change to build folder which fails if it doesn't exist and CACHED_CMAKE_OPTIONS stays empty
	@# fetch all previously configured and cached options from the build folder and transform them into the OPTION=VALUE format without type (e.g. :BOOL)
	@$(eval CACHED_CMAKE_OPTIONS = $(shell cd $(BUILD_DIR) 2>/dev/null && cmake -L 2>/dev/null | sed -n 's|\([^[:blank:]]*\):[^[:blank:]]*\(=[^[:blank:]]*\)|\1\2|gp' ))
	@# transform the options in CMAKE_ARGS into the OPTION=VALUE format without -D
	@$(eval DESIRED_CMAKE_OPTIONS = $(shell echo $(CMAKE_ARGS) | sed -n 's|-D\([^[:blank:]]*=[^[:blank:]]*\)|\1|gp' ))
	@# find each currently desired option in the already cached ones making sure the complete configured string value is the same
	@$(eval VERIFIED_CMAKE_OPTIONS = $(foreach option,$(DESIRED_CMAKE_OPTIONS),$(strip $(findstring $(option)$(space),$(CACHED_CMAKE_OPTIONS)))))
	@# if the complete list of desired options is found in the list of verified options we don't need to reconfigure and CMAKE_CACHE_CHECK stays empty
	@$(eval CMAKE_CACHE_CHECK = $(if $(findstring $(DESIRED_CMAKE_OPTIONS),$(VERIFIED_CMAKE_OPTIONS)),,y))
endef

# All targets.
help:
	@echo "make <target> where target is one targes list with tools/configure.sh -L (use a dash as the seperator)"

$(ALL_CONFIG_TARGETS):
	@$(eval NUTTX_TARGET = $@)
	$(eval NUTTX_BOARD = $(firstword $(subst -, ,$(NUTTX_TARGET))))
	$(eval NUTTX_CONFIG = $(lastword $(subst -, ,$(NUTTX_TARGET))))
	@$(eval CMAKE_ARGS += "-DNUTTX_BOARD:STRING=$(NUTTX_BOARD) -DNUTTX_CONFIG:string=$(NUTTX_CONFIG)" )
	@$(call cmake-build,$(NUTTX_TARGET))

#todo fix the tools.
clean:
	rm -fr $(NUTTX_BUILD_ROOT)
