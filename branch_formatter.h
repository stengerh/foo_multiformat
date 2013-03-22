#pragma once

namespace multiformat
{
	class branch_point
	{
	public:
		branch_point() {}
		explicit branch_point(t_size p_total, t_size p_meta_index = ~0) : m_value_count(p_total), m_value_index(0), m_meta_index(p_meta_index) {}

		boolean is_empty() const {return (0 == m_value_count);}
		void next() {if (m_value_index < m_value_count) {m_value_index += 1;}}
		boolean has_next() const {return (m_value_index < m_value_count);}

		t_size get_value_index() const {return m_value_index;}
		t_size get_meta_index() const {return m_meta_index;}

	private:
		t_size m_meta_index;
		t_size m_value_count;
		t_size m_value_index;
	};

	class NOVTABLE branch_point_callback
	{
	protected:
		branch_point_callback() {}
		~branch_point_callback() {}

	public:
		virtual t_size add_branch_point(const branch_point &p_branch_point) = 0;
		virtual bool next_branch_point() = 0;

		virtual t_size get_current_branch_point_index() const = 0;
		virtual const branch_point & get_branch_point(t_size p_index) const = 0;

		const branch_point & get_current_branch_point() const {return get_branch_point(get_current_branch_point_index());}
	};

	class NOVTABLE titleformat_branch_hook
	{
	protected:
		titleformat_branch_hook() {}
		~titleformat_branch_hook() {}

	public:
		virtual void set_callback(branch_point_callback * p_callback) = 0;
		virtual void prepare_branch(t_size p_branch_point_limit) = 0;
		virtual bool process_field(titleformat_text_out * p_out,const char * p_name,t_size p_name_length,bool & p_found_flag) = 0;
		virtual bool process_function(titleformat_text_out * p_out,const char * p_name,t_size p_name_length,titleformat_hook_function_params * p_params,bool & p_found_flag) = 0;
	};

	class branch_point_manager : public branch_point_callback
	{
	public:
		branch_point_manager();
		~branch_point_manager();

		t_size add_branch_point(const branch_point &p_item);
		bool next_branch_point();

		void set_current_branch_point_index(t_size p_index);
		t_size get_current_branch_point_index() const;
		const branch_point & get_branch_point(t_size p_index) const;

		bool is_branch_empty() const;
		bool next_branch();
		void reset();
		t_size get_branch_point_count();
	private:
		pfc::list_t<branch_point> m_stack;
		t_size m_current;
	};

	class branch_formatter
	{
	public:
		branch_formatter(titleformat_object::ptr p_script);

		void run_nonlocking(pfc::list_base_t<pfc::string8> & p_list_out, metadb_handle *p_handle, titleformat_text_filter * p_filter);

	private:
		void run_nonlocking(pfc::list_base_t<pfc::string8> & p_list_out, metadb_handle *p_handle, titleformat_hook * p_branch_neutral_hook, titleformat_branch_hook * p_branch_aware_hook, titleformat_text_filter * p_filter);

	private:
		titleformat_object::ptr m_script;
		pfc::string_formatter m_buffer;

		branch_point_manager m_manager;
	};

	class titleformat_hook_impl_branch_splitter : public titleformat_hook
	{
	public:
		titleformat_hook_impl_branch_splitter(titleformat_hook * p_branch_neutral_hook, titleformat_branch_hook * p_branch_aware_hook, branch_point_callback * p_callback);
		~titleformat_hook_impl_branch_splitter();

		void prepare_branch(t_size p_branch_point_limit);

		virtual bool process_field(titleformat_text_out * p_out,const char * p_name,t_size p_name_length,bool & p_found_flag);
		virtual bool process_function(titleformat_text_out * p_out,const char * p_name,t_size p_name_length,titleformat_hook_function_params * p_params,bool & p_found_flag);

	private:
		titleformat_hook * m_branch_neutral_hook;
		titleformat_branch_hook * m_branch_aware_hook;
	};

	class titleformat_hook_impl_file_info_branch : public titleformat_branch_hook
	{
	public:
		titleformat_hook_impl_file_info_branch(const playable_location & p_location,const file_info * p_info);

		virtual void set_callback(branch_point_callback * p_callback);
		virtual void prepare_branch(t_size p_branch_point_limit);

		virtual bool process_field(titleformat_text_out * p_out, const char * p_name, t_size p_name_length, bool & p_found_flag);
		virtual bool process_function(titleformat_text_out * p_out, const char * p_name, t_size p_name_length, titleformat_hook_function_params * p_params, bool & p_found_flag);

	protected:
		bool remap_meta(t_size & p_index, const char * p_name, t_size p_name_length) {return m_api->remap_meta(*m_info,p_index,p_name,p_name_length);}
		const file_info * m_info;

	private:
		bool process_meta_branch(titleformat_text_out * p_out, const char * p_name, t_size p_name_length, bool p_remap, bool & p_found_flag);
		void remove_invalid_entries(pfc::map_t<pfc::string8, t_size> & p_map, t_size p_branch_point_limit);

	private:
		const playable_location & m_location;
		static_api_ptr_t<titleformat_common_methods> m_api;

	private:
		branch_point_callback * m_callback;
		pfc::map_t<pfc::string8, t_size> m_meta_map;
		pfc::map_t<pfc::string8, t_size> m_meta_remap_map;
	};
}
