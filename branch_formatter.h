#pragma once

namespace multiformat
{
	struct branch_info
	{
		t_size total;
		t_size current;
		t_size meta_index;

		branch_info() {}
		explicit branch_info(t_size p_total, t_size p_meta_index = ~0) : total(p_total), current(0), meta_index(p_meta_index) {}
	};

	class branch_formatter
	{
	public:
		friend class titleformat_hook_impl_file_info_branch;

		branch_formatter(titleformat_object::ptr p_script);

		void run_nonlocking(pfc::list_base_t<pfc::string8> & p_list_out, metadb_handle *p_handle, titleformat_text_filter * p_filter);

	private:
		bool is_branch_empty() const;
		bool next_branch();

		titleformat_object::ptr m_script;
		pfc::string_formatter m_buffer;

		pfc::list_t<branch_info> m_stack;
		t_size m_current;

		pfc::map_t<pfc::string8, t_size> m_map;
	};

	class titleformat_hook_impl_file_info_branch : public titleformat_hook_impl_file_info
	{
	public:
		titleformat_hook_impl_file_info_branch(branch_formatter & p_formatter, const playable_location & p_location,const file_info * p_info);

		virtual bool process_field(titleformat_text_out * p_out, const char * p_name, t_size p_name_length, bool & p_found_flag);
		virtual bool process_function(titleformat_text_out * p_out, const char * p_name, t_size p_name_length, titleformat_hook_function_params * p_params, bool & p_found_flag);

	private:
		branch_formatter & m_formatter;
	};
}
