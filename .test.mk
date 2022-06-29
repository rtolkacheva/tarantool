#
# CI testing rules
#

SRC_DIR = .
BUILD_DIR ?= .
VARDIR ?= /tmp/t
MAX_FILES ?= 4096

LUAJIT_TEST_BUILD_DIR ?= ${BUILD_DIR}
TEST_RUN_PARAMS ?= --builddir ${PWD}/${BUILD_DIR}

configure:
	${CMAKE_ENV} cmake -S ${SRC_DIR} -B ${BUILD_DIR} ${CMAKE_PARAMS} ${CMAKE_EXTRA_PARAMS}

# Static Analysis

luacheck: configure
	${CMAKE_ENV} cmake --build ${BUILD_DIR} --parallel $(NPROC) --target luacheck

# Building

NPROC=$(shell nproc || sysctl -n hw.logicalcpu)

build: configure
	${CMAKE_ENV} cmake --build ${BUILD_DIR} --parallel $(NPROC)
	if [ "${CTEST}" = "true" ]; then cd ${BUILD_DIR} && ctest -V; fi

# Testing

run-luajit-test:
	${CMAKE_ENV} cmake --build ${LUAJIT_TEST_BUILD_DIR} --parallel $(NPROC) --target LuaJIT-test

install-test-deps:
	python3 -m pip install -r test-run/requirements.txt

run-test: install-test-deps
	cd test && ${TEST_RUN_ENV} ./test-run.py --force --vardir ${VARDIR} ${TEST_RUN_PARAMS} ${TEST_RUN_EXTRA_PARAMS}

##############################
# Linux                      #
##############################

# Release build

test-release: CMAKE_PARAMS = -DCMAKE_BUILD_TYPE=RelWithDebInfo -DENABLE_WERROR=ON
test-release: build run-luajit-test run-test

# Release ASAN build

test-release-asan: CMAKE_ENV = CC=clang-11 CXX=clang++-11
test-release-asan: CMAKE_PARAMS = -DCMAKE_BUILD_TYPE=RelWithDebInfo -DENABLE_WERROR=ON -DENABLE_ASAN=ON \
	-DENABLE_UB_SANITIZER=ON -DENABLE_FUZZER=ON
# Temporary suppressed some checks in the scope of the issue https://github.com/tarantool/tarantool/issues/4360:
#  - ASAN: to suppress failures of memory error checks caught while tests run, the tarantool/asan/asan.supp file is
#      used. It is set as a value for -fsanitize-blacklist option at the build time in the cmake/profile.cmake file.
#  - LSAN: to suppress failures of memory leak checks caught while tests run, the tarantool/asan/lsan.supp file is used.
test-release-asan: TEST_RUN_ENV = ASAN=ON LSAN_OPTIONS=suppressions=${PWD}/asan/lsan.supp ASAN_OPTIONS=heap_profile=0:$\
unmap_shadow_on_exit=1:detect_invalid_pointer_pairs=1:symbolize=1:detect_leaks=1:dump_instruction_bytes=1:$\
print_suppressions=0
test-release-asan: build run-test

# Debug build

test-debug: CMAKE_PARAMS = -DCMAKE_BUILD_TYPE=Debug
test-debug: build run-luajit-test run-test

# Static build

test-static: CMAKE_PARAMS = -DCMAKE_BUILD_TYPE=RelWithDebInfo -DENABLE_WERROR=ON -DBUILD_STATIC=ON
test-static: build run-luajit-test run-test

# Static build (cmake)

test-static-cmake: SRC_DIR = static-build
test-static-cmake: BUILD_DIR = static-build
test-static-cmake: CTEST = true
test-static-cmake: CMAKE_PARAMS = -DCMAKE_TARANTOOL_ARGS="-DCMAKE_BUILD_TYPE=RelWithDebInfo;-DENABLE_WERROR=ON"
test-static-cmake: LUAJIT_TEST_BUILD_DIR = ${BUILD_DIR}/tarantool-prefix/src/tarantool-build
test-static-cmake: TEST_RUN_PARAMS = --builddir ${PWD}/${BUILD_DIR}/tarantool-prefix/src/tarantool-build
test-static-cmake: build run-luajit-test run-test

# Coverage build

test-coverage: CMAKE_PARAMS = -G Ninja -DCMAKE_BUILD_TYPE=Debug -DENABLE_GCOV=ON
test-coverage: TEST_RUN_PARAMS = --builddir ${PWD}/${BUILD_DIR} --long
test-coverage: build run-luajit-test run-test
	lcov \
		--capture \
		--compat-libtool \
		--directory ${BUILD_DIR}/src/ \
		--output-file coverage.info \
		--rc lcov_function_coverage=1 \
		--rc lcov_branch_coverage=1 \
		--exclude '/usr/*' \
		--exclude '*/build/*' \
		--exclude '*/test/*' \
		--exclude '*/third_party/*'
	lcov --list coverage.info

##############################
# OSX                        #
##############################

prebuild-osx:
	sysctl vm.swapusage

pretest-osx:
	ulimit -n ${MAX_FILES} || : && ulimit -n

# Release build

test-osx-release: CMAKE_PARAMS = -DCMAKE_BUILD_TYPE=RelWithDebInfo -DENABLE_WERROR=ON
test-osx-release: prebuild-osx build run-luajit-test pretest-osx run-test

# FIXME: Temporary target with reduced number of tests. Use 'test-release-osx'
# target instead when all M1 issues are resolved.
test-osx-release-arm64: CMAKE_PARAMS = -DCMAKE_BUILD_TYPE=RelWithDebInfo -DENABLE_WERROR=ON
test-osx-release-arm64: prebuild-osx build run-luajit-test

# Debug build

test-osx-debug: CMAKE_PARAMS = -DCMAKE_BUILD_TYPE=Debug
test-osx-debug: prebuild-osx build run-luajit-test pretest-osx run-test

# FIXME: Temporary target with reduced number of tests. Use 'test-release-osx'
# target instead when all M1 issues are resolved.
test-osx-debug-arm64: CMAKE_PARAMS = -DCMAKE_BUILD_TYPE=Debug
test-osx-debug-arm64: prebuild-osx build run-luajit-test

# Static build

test-osx-static-cmake: SRC_DIR = static-build
test-osx-static-cmake: BUILD_DIR = static-build
test-osx-static-cmake: CTEST = true
test-osx-static-cmake: CMAKE_PARAMS = -DCMAKE_TARANTOOL_ARGS="-DCMAKE_BUILD_TYPE=RelWithDebInfo;-DENABLE_WERROR=ON"
test-osx-static-cmake: LUAJIT_TEST_BUILD_DIR = ${BUILD_DIR}/tarantool-prefix/src/tarantool-build
test-osx-static-cmake: TEST_RUN_PARAMS = --builddir ${PWD}/${BUILD_DIR}/tarantool-prefix/src/tarantool-build
test-osx-static-cmake: prebuild-osx build run-luajit-test pretest-osx run-test

##############################
# FreeBSD                    #
##############################

prebuild-freebsd:
	if [ "$$(swapctl -l | wc -l)" != "1" ]; then swapoff -a; fi; swapctl -l

# Release build

test-freebsd-release: CMAKE_PARAMS = -DCMAKE_BUILD_TYPE=RelWithDebInfo -DENABLE_WERROR=ON
test-freebsd-release: prebuild-freebsd build run-luajit-test run-test

##############################
# Jepsen testing             #
##############################

prebuild-jepsen:
	# Jepsen build uses git commands internally, like command `git stash --all`
	# that fails w/o git configuration setup.
	git config --get user.name || git config --global user.name "Nodody User"
	git config --get user.email || git config --global user.email "nobody@nowhere.com"

test-jepsen: CMAKE_PARAMS = -DCMAKE_BUILD_TYPE=RelWithDebInfo -DENABLE_WERROR=ON -DWITH_JEPSEN=ON
test-jepsen: configure prebuild-jepsen
	# TODO: Support parallel build. In order to do it we need to run Jepsen testing in a `debian-buster` container
	# instead of `debian-stretch` due to the old `cmake` version. The `cmake` utility started to support `--parallel`
	# option since 3.12 version. In Debian Stretch `cmake` has version 3.7. In Debian Buster `cmake` has version 3.13.
	${CMAKE_ENV} cmake --build ${BUILD_DIR} --target run-jepsen

##############################
# Coverity testing           #
##############################

build-coverity: CMAKE_PARAMS = -DCMAKE_BUILD_TYPE=RelWithDebInfo -DENABLE_WERROR=ON
build-coverity: COV_BUILD_ENV = PATH=${PATH}:/cov-analysis/bin
build-coverity: configure
	${COV_BUILD_ENV} cov-build --dir cov-int cmake --build ${BUILD_DIR} --parallel $(NPROC)

test-coverity: build-coverity
	tar czvf tarantool.tgz cov-int
	if [ -n "$${COVERITY_TOKEN}" ]; then \
		echo "Exporting code coverity information to scan.coverity.com"; \
		curl \
			--location \
			--fail \
			--silent \
			--show-error \
			--retry 5 \
			--retry-delay 5 \
			--form token=$${COVERITY_TOKEN} \
			--form email=tarantool@tarantool.org \
			--form file=@tarantool.tgz \
			--form version=$(shell git describe HEAD) \
			--form description="Tarantool Coverity" \
			https://scan.coverity.com/builds?project=tarantool%2Ftarantool ; \
	else \
		echo "Coverity token is not provided"; \
		exit 1; \
	fi

##############################
# LuaJIT integration testing #
##############################

# TBD

.PHONY: configure luacheck build run-luajit-test install-test-deps run-test test-release test-release-asan test-debug \
	test-static test-static-cmake test-coverage prebuild-osx pretest-osx test-osx-release test-osx-release-arm64 \
	test-osx-debug test-osx-debug-arm64 test-osx-static-cmake prebuild-freebsd test-freebsd-release prebuild-jepsen \
	test-jepsen build-coverity test-coverity
