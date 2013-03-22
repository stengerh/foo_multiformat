#pragma once

namespace multiformat
{
	class meta_branch_definition
	{
	public:
		meta_branch_definition() {}
		explicit meta_branch_definition(const char * p_name, t_size p_name_length, bool p_remapped, t_size p_meta_index, t_size p_value_count, bool p_found_flag)
			: m_name(p_name, p_name_length), m_remapped(p_remapped), m_meta_index(p_meta_index), m_value_count(p_value_count), m_value_index(0), m_found_flag(p_found_flag) {};

		bool matches(const char * p_name, t_size p_name_length, bool p_remapped) const {return (m_remapped == p_remapped) && (0 == pfc::strcmp_ex(m_name.get_ptr(), m_name.get_length(), p_name, p_name_length));}
		void get(t_size & p_meta_index, t_size & p_value_index, bool & p_found_flag) const {p_meta_index = m_meta_index; p_value_index = m_value_index; p_found_flag = m_found_flag;}

		boolean is_empty() const {return (0 == m_value_count);}
		void next() {if (m_value_index < m_value_count) {m_value_index += 1;}}
		boolean is_done() const {return !(m_value_index < m_value_count);}

	private:
		pfc::string m_name;
		bool m_remapped;

		t_size m_meta_index;
		t_size m_value_count;
		t_size m_value_index;
		bool m_found_flag;
	};

	class branch_point
	{
	public:
		branch_point() {}
		explicit branch_point(pfc::list_t<meta_branch_definition> * p_definitions, t_size p_definition_index, bool p_reference)
			: m_definitions(p_definitions), m_definition_index(p_definition_index), m_reference(p_reference) {}

		boolean is_empty() const;
		void next();
		boolean is_done() const;
		void remove();

		bool matches(const char * p_name, t_size p_name_length, bool p_remapped) const;
		void get(t_size & p_meta_index, t_size & p_value_index, bool & p_found_flag) const;

	private:
		meta_branch_definition & get_definition() {return (*m_definitions)[m_definition_index];};
		const meta_branch_definition & get_definition() const {return (*m_definitions)[m_definition_index];};

	private:
		pfc::list_t<meta_branch_definition> * m_definitions;
		t_size m_definition_index;
		bool m_reference;
	};

	class NOVTABLE branch_point_callback
	{
	protected:
		branch_point_callback() {}
		~branch_point_callback() {}

	public:
		virtual bool replay_definition(const char * p_name, t_size p_name_length, bool p_remapped, t_size & p_meta_index, t_size & p_value_count, bool & p_found_flag) = 0;
		virtual void add_definition(const char * p_name, t_size p_name_length, bool p_remapped, t_size p_meta_index, t_size p_value_count, bool p_found_flag) = 0;
		virtual bool add_reference(const char * p_name, t_size p_name_length, bool p_remapped, t_size & p_meta_index, t_size & p_value_count, bool & p_found_flag) = 0;

		virtual t_size get_current_branch_point_index() const = 0;
	};

	class branch_point_manager : public branch_point_callback
	{
	public:
		branch_point_manager();
		~branch_point_manager();

		virtual bool replay_definition(const char * p_name, t_size p_name_length, bool p_remapped, t_size & p_meta_index, t_size & p_value_index, bool & p_found_flag);
		virtual void add_definition(const char * p_name, t_size p_name_length, bool p_remapped, t_size p_meta_index, t_size p_value_count, bool p_found_flag);
		virtual bool add_reference(const char * p_name, t_size p_name_length, bool p_remapped, t_size & p_meta_index, t_size & p_value_index, bool & p_found_flag);

		virtual t_size get_current_branch_point_index() const;

		bool is_branch_empty() const;
		bool is_done() const;
		void next_branch();
		void reset();
		t_size get_branch_point_count();

	private:
		pfc::list_t<meta_branch_definition> m_definitions;
		pfc::list_t<branch_point> m_branch_points;
		t_size m_branch_point_index;
		bool m_done;
	};

	class branch_formatter
	{
	public:
		branch_formatter(titleformat_object::ptr p_script);

		void run_nonlocking(pfc::list_base_t<pfc::string8> & p_list_out, metadb_handle *p_handle, titleformat_text_filter * p_filter);

	private:
		titleformat_object::ptr m_script;
		pfc::string_formatter m_buffer;

		branch_point_manager m_manager;
	};

	class titleformat_hook_impl_file_info_branch : public titleformat_hook
	{
	public:
		titleformat_hook_impl_file_info_branch(branch_point_callback & p_callback, const playable_location & p_location,const file_info * p_info);

		virtual bool process_field(titleformat_text_out * p_out, const char * p_name, t_size p_name_length, bool & p_found_flag);
		virtual bool process_function(titleformat_text_out * p_out, const char * p_name, t_size p_name_length, titleformat_hook_function_params * p_params, bool & p_found_flag);

	protected:
		bool remap_meta(t_size & p_index, const char * p_name, t_size p_name_length) {return m_api->remap_meta(*m_info,p_index,p_name,p_name_length);}
		const file_info * m_info;

	private:
		bool process_meta_branch(titleformat_text_out * p_out, const char * p_name, t_size p_name_length, bool p_remap, bool & p_found_flag);

	private:
		const playable_location & m_location;
		static_api_ptr_t<titleformat_common_methods> m_api;

	private:
		branch_point_callback & m_callback;
	};
}
