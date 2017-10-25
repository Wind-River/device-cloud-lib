#
# Copyright (C) 2017 Wind River Systems, Inc. All Rights Reserved.
#
# The right to copy, distribute or otherwise make use of this software may
# be licensed only pursuant to the terms of an applicable Wind River license
# agreement.  No license to Wind River intellectual property rights is granted
# herein.  All rights not licensed by Wind River are reserved by Wind River.
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
	"os_uuid_generate"
	"os_uuid_to_string_lower"
	"os_vfprintf"
	"os_vsnprintf"
)

