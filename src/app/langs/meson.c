/* Meson */
#include "../lang_def.h"

LANGUAGE(meson) {
	.name = "meson",
	.title = "Meson",
	.extensions = "meson",
	.filenames = "meson.options meson_options.txt meson.build",

	.keywords = WORDS(
		"and", "break", "continue", "elif", "else", "endforeach", "endif",
		"foreach", "if", "not", "or"),

	.constants = WORDS(
		"false", "true"),

	.functions = WORDS(
		"add_dist_script", "add_global_arguments", "add_global_link_arguments",
		"add_install_script", "add_languages", "add_postconf_script",
		"add_project_arguments", "add_project_link_arguments", "add_test_setup",
		"alias_targ", "alignment", "append", "as_link_whole", "as_system",
		"assert", "auto", "backend", "benchmark", "both_libraries",
		"build_machine", "build_root", "build_target", "can_run_host_binaries",
		"check_header", "cmd_array", "compiled", "compiles", "compute_int",
		"configuration_data", "configure_file", "contains", "cpu", "cpu_family",
		"current_build_dir", "current_source_dir", "custom_target",
		"declare_dependency", "dependency", "disabled", "disabler", "enabled",
		"endian", "endswith", "environment", "error", "executable",
		"extract_all_objects", "extract_objects", "files", "find_library",
		"find_program", "first_supported_argument",
		"first_supported_link_argument", "format", "found", "full_path",
		"generator", "get", "get_argument_syntax", "get_compiler",
		"get_configtool_variable", "get_cross_property", "get_define",
		"get_external_property", "get_id", "get_linker_id", "get_option",
		"get_pkgconfig_variable", "get_supported_arguments",
		"get_supported_function_attributes", "get_supported_link_arguments",
		"get_unquoted", "get_variable", "has", "has_argument",
		"has_exe_wrapper", "has_function", "has_function_attribute",
		"has_header", "has_header_symbol", "has_key", "has_link_argument",
		"has_member", "has_members", "has_multi_arguments",
		"has_multi_link_arguments", "has_type", "host_machine", "import",
		"include_directories", "include_type", "install_data",
		"install_dependency_manifest", "install_headers", "install_man",
		"install_subdir", "is_cross_build", "is_disabler", "is_even", "is_odd",
		"is_subproject", "is_unity", "is_variable", "jar", "join", "join_paths",
		"keys", "length", "library", "links", "merge_from", "meson", "message",
		"name", "override_dependency", "override_find_program",
		"partial_dependency", "path", "prepend", "private_dir_include",
		"process", "project", "project_build_root", "project_license",
		"project_name", "project_source_root", "project_version", "returncode",
		"run", "run_command", "run_targ", "set", "set10", "set_quoted",
		"set_variable", "shared_library", "shared_module", "sizeof",
		"source_root", "split", "startswith", "static_library", "stderr",
		"stdout", "strip", "subdir", "subdir_done", "subproject", "substring",
		"summary", "symbols_have_underscore_prefix", "system", "target_machine",
		"test", "to_int", "to_list", "to_lower", "to_string", "to_upper",
		"type_name", "underscorify", "vcs_tag", "version", "version_compare",
		"warning"),

	.line_comments = { "#" },
	.long_strings = { { "'''", "'''" } },
	.quotes = "'",
};
