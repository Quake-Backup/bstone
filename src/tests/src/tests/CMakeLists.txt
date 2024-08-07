#[[
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
]]

cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(bstone_tests VERSION 1.0.0 LANGUAGES CXX)

message(STATUS "[bstone_tests] v${PROJECT_VERSION}")

add_executable(bstone_tests "")

set_target_properties(bstone_tests PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

target_compile_definitions(bstone_tests
	PRIVATE
		$<$<BOOL:${WIN32}>:UNICODE>
		$<$<BOOL:${WIN32}>:_UNICODE>
		$<$<BOOL:${WIN32}>:_BIND_TO_CURRENT_VCLIBS_VERSION=1>
		$<$<BOOL:${WIN32}>:NTDDI_VERSION=0x05010000>
		$<$<BOOL:${WIN32}>:_WIN32_WINNT=0x0501>
		$<$<BOOL:${WIN32}>:WINVER=0x0501>
)

target_compile_options(bstone_tests
	PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:/W4>
		$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
)

target_include_directories(bstone_tests
	PRIVATE
		${PROJECT_SOURCE_DIR}/../../..
)

target_link_libraries(bstone_tests
	PRIVATE
		${CMAKE_DL_LIBS}
		bstone_tests_shared_library
)

target_sources(bstone_tests
	PRIVATE
		../../../bstone_char_traits.cpp
		../../../bstone_char_traits.h
		../../../bstone_exception.cpp
		../../../bstone_exception.h
		../../../bstone_source_location.cpp
		../../../bstone_source_location.h
		../../../bstone_span.cpp
		../../../bstone_span.h
		../../../bstone_utility.cpp
		../../../bstone_utility.h
		../../../bstone_algorithm.cpp
		../../../bstone_algorithm.h
		../../../bstone_ascii.cpp
		../../../bstone_ascii.h
		../../../bstone_endian.cpp
		../../../bstone_endian.h
		../../../bstone_memory.cpp
		../../../bstone_memory.h
		../../../bstone_memory_resource.cpp
		../../../bstone_memory_resource.h
		../../../bstone_scope_exit.cpp
		../../../bstone_scope_exit.h
		../../../bstone_array.cpp
		../../../bstone_array.h
		../../../bstone_char_conv.cpp
		../../../bstone_char_conv.cpp
		../../../bstone_unique_resource.cpp
		../../../bstone_unique_resource.h
		../../../bstone_cgm_vec.cpp
		../../../bstone_cgm_vec.h
		../../../bstone_cgm_mat.cpp
		../../../bstone_cgm_mat.h
		../../../bstone_cgm_clip_space.cpp
		../../../bstone_cgm_clip_space.h
		../../../bstone_cgm_transform.cpp
		../../../bstone_cgm_transform.h
		../../../bstone_sha1.cpp
		../../../bstone_sha1.h
		../../../bstone_memory_pool_bitmap.cpp
		../../../bstone_memory_pool_bitmap.h
		../../../bstone_unicode.cpp
		../../../bstone_unicode.h
		../../../bstone_utf.cpp
		../../../bstone_utf.h
		../../../bstone_utf16.cpp
		../../../bstone_utf16.h
		../../../bstone_utf8.cpp
		../../../bstone_utf8.h
		../../../bstone_single_pool_resource.cpp
		../../../bstone_single_pool_resource.h
		../../../bstone_fixed_pool_resource.cpp
		../../../bstone_fixed_pool_resource.h
		../../../bstone_generic_pool_resource.cpp
		../../../bstone_generic_pool_resource.h
		../../../bstone_auto_arena_resource.cpp
		../../../bstone_auto_arena_resource.h
		../../../bstone_ts_auto_arena_resource.cpp
		../../../bstone_ts_auto_arena_resource.h
		../../../bstone_win32_wstring.cpp
		../../../bstone_win32_wstring.h
		../../../bstone_file_posix.cpp
		../../../bstone_file_win32.cpp
		../../../bstone_file.cpp
		../../../bstone_file.h
		../../../bstone_stream.cpp
		../../../bstone_stream.h
		../../../bstone_file_stream.cpp
		../../../bstone_file_stream.h
		../../../bstone_memory_stream.cpp
		../../../bstone_memory_stream.h
		../../../bstone_static_memory_stream.cpp
		../../../bstone_static_memory_stream.h
		../../../bstone_static_ro_memory_stream.cpp
		../../../bstone_static_ro_memory_stream.h
		../../../bstone_binary_reader.cpp
		../../../bstone_binary_reader.h
		../../../bstone_binary_writer.cpp
		../../../bstone_binary_writer.h
		../../../bstone_crc32.cpp
		../../../bstone_crc32.h
		../../../bstone_shared_library.cpp
		../../../bstone_shared_library.h
		../../../bstone_shared_library_posix.cpp
		../../../bstone_shared_library_win32.cpp
		../../../bstone_win32_registry_key.cpp
		../../../bstone_win32_registry_key.h
		../../../bstone_api_string.cpp
		../../../bstone_api_string.h
		../../../bstone_cxx.cpp
		../../../bstone_cxx.h
		../../../bstone_string_view.cpp
		../../../bstone_string_view.h
		../../../bstone_string.cpp
		../../../bstone_string.h
		../../../bstone_fs_posix.cpp
		../../../bstone_fs_win32.cpp
		../../../bstone_fs.cpp
		../../../bstone_fs.h
		../../../bstone_posix_string.cpp
		../../../bstone_posix_string.h
		../../../bstone_win32_advapi32_symbols.cpp
		../../../bstone_win32_advapi32_symbols.h
		../../../bstone_four_cc.cpp
		../../../bstone_four_cc.h
		../../../bstone_uuid.cpp
		../../../bstone_uuid_unix.cpp
		../../../bstone_uuid_win32.cpp
		../../../bstone_uuid.h
		../../../bstone_win32_unique_resources.cpp
		../../../bstone_win32_unique_resources.h
		../../../bstone_process.cpp
		../../../bstone_process_posix.cpp
		../../../bstone_process_win32.cpp
		../../../bstone_process.h
	PRIVATE
		src/bstone_tests_algorithm.cpp
		src/bstone_tests_array.cpp
		src/bstone_tests_ascii.cpp
		src/bstone_tests_char_conv.cpp
		src/bstone_tests_char_traits.cpp
		src/bstone_tests_endian.cpp
		src/bstone_tests_exception.cpp
		src/bstone_tests_memory.cpp
		src/bstone_tests_memory_resource.cpp
		src/bstone_tests_scope_exit.cpp
		src/bstone_tests_source_location.cpp
		src/bstone_tests_span.cpp
		src/bstone_tests_unique_resource.cpp
		src/bstone_tests_utility.cpp
		src/bstone_tests_cgm_vec.cpp
		src/bstone_tests_cgm_mat.cpp
		src/bstone_tests_cgm_clip_space.cpp
		src/bstone_tests_cgm_transform.cpp
		src/bstone_tests_sha1.cpp
		src/bstone_tests_memory_pool_bitmap.cpp
		src/bstone_tests_utf.cpp
		src/bstone_tests_single_pool_resource.cpp
		src/bstone_tests_fixed_pool_resource.cpp
		src/bstone_tests_generic_pool_resource.cpp
		src/bstone_tests_auto_arena_resource.cpp
		src/bstone_tests_ts_auto_arena_resource.cpp
		src/bstone_tests_win32_wstring.cpp
		src/bstone_tests_configurations.cpp
		src/bstone_tests_file.cpp
		src/bstone_tests_file_stream.cpp
		src/bstone_tests_memory_stream.cpp
		src/bstone_tests_static_memory_stream.cpp
		src/bstone_tests_static_ro_memory_stream.cpp
		src/bstone_tests_binary_reader.cpp
		src/bstone_tests_binary_writer.cpp
		src/bstone_tests_crc32.cpp
		src/bstone_tests_shared_library.cpp
		src/bstone_tests_win32_registry_key.cpp
		src/bstone_tests_api_string.cpp
		src/bstone_tests_string_view.cpp
		src/bstone_tests_string.cpp
		src/bstone_tests_fs.cpp
		src/bstone_tests_four_cc.cpp
		src/bstone_tests_uuid.cpp
		src/bstone_tests_process.cpp
	PRIVATE
		src/bstone_tester.cpp
		src/bstone_tester.h
		src/bstone_tests.cpp
)

install(TARGETS bstone_tests RUNTIME DESTINATION "tests")
install(DIRECTORY data DESTINATION "tests")
