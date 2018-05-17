#
# Copyright (C) 2017-2018 Wind River Systems, Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software  distributed
# under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
# OR CONDITIONS OF ANY KIND, either express or implied.
#

set( MOCK_OSAL_LIBS "mock_osal" )

set( MOCK_OSAL_FUNC
	"os_calloc"
	"os_directory_current"
	"os_directory_create"
	"os_directory_exists"
	"os_env_expand"
	"os_env_get"
	"os_file_chown"
	"os_file_close"
	"os_file_eof"
	"os_file_exists"
	"os_file_open"
	"os_file_read"
	"os_file_write"
	"os_flush"
	"os_fprintf"
	"os_free"
	"os_free_null"
	"os_make_path"
	"os_malloc"
	"os_memcpy"
	"os_memmove"
	"os_memset"
	"os_memzero"
	"os_path_is_absolute"
	"os_path_executable"
	"os_printf"
	"os_realloc"
	"os_snprintf"
	"os_socket_initialize"
	"os_socket_terminate"
	"os_strcasecmp"
	"os_strchr"
	"os_strcmp"
	"os_strlen"
	"os_strncasecmp"
	"os_strncmp"
	"os_strncpy"
	"os_strpbrk"
	"os_strrchr"
	"os_strstr"
	"os_strtod"
	"os_strtok"
	"os_strtol"
	"os_strtoul"
	"os_system_error_last"
	"os_system_error_string"
	"os_system_pid"
	"os_system_run"
	"os_terminal_vt100_support"
	"os_thread_condition_broadcast"
	"os_thread_condition_create"
	"os_thread_condition_destroy"
	"os_thread_condition_signal"
	"os_thread_condition_wait"
	"os_thread_create"
	"os_thread_destroy"
	"os_thread_mutex_create"
	"os_thread_mutex_destroy"
	"os_thread_mutex_lock"
	"os_thread_mutex_unlock"
	"os_thread_rwlock_create"
	"os_thread_rwlock_destroy"
	"os_thread_rwlock_read_lock"
	"os_thread_rwlock_read_unlock"
	"os_thread_rwlock_write_lock"
	"os_thread_rwlock_write_unlock"
	"os_thread_wait"
	"os_time"
	"os_time_sleep"
	"os_uuid_generate"
	"os_uuid_to_string_lower"
	"os_vfprintf"
	"os_vsnprintf"
)

