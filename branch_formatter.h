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

	/**
	 * NOT USED YET.
	 * Life cycle for each track:
	 * 1. on_track_started(p_handle) - formatter begins formatting a new track p_handle.
	 * 2a. on_branch_started() - formatter begins evaluation of a new branch.
	 * 2b. on_branch_replay_finished() - formatter reached the last branch point, i.e. it has restored the state where the latest decision was made and continues evaluating another choice.
	 * 2c. on_branch_finished(p_valid) - formatter finished evaluation of a branch, p_valid specifies whether the branch is used or discarded.
	 * 3. on_track_finished() - formatter evaluated all branches for the current track.
	 */
	NOVTABLE class branch_formatter_callback
	{
	protected:
		branch_formatter_callback() {}
		~branch_formatter_callback() {}

	public:
		virtual void on_track_started(metadb_handle *p_handle) = 0;
		virtual void on_track_finished() = 0;

		virtual void on_branch_started() = 0;
		virtual void on_branch_replay_finished() = 0;
		virtual void on_branch_finished(bool p_valid) = 0;
	};

	NOVTABLE class branch_point_callback
	{
	protected:
		branch_point_callback() {}
		~branch_point_callback() {}

	public:
		virtual void add_branch_point(const branch_point &p_branch_point) = 0;
		virtual bool next_branch_point() = 0;

		virtual t_size get_current_branch_point_index() const = 0;
		virtual const branch_point & get_branch_point(t_size p_index) const = 0;

		const branch_point & get_current_branch_point() const {return get_branch_point(get_current_branch_point_index());}
	};

	class branch_point_manager : public branch_point_callback
	{
	public:
		branch_point_manager();
		~branch_point_manager();

		void add_branch_point(const branch_point &p_item);
		bool next_branch_point();

		void set_current_branch_point_index(t_size p_index);
		t_size get_current_branch_point_index() const;
		const branch_point & get_branch_point(t_size p_index) const;

		bool is_branch_empty() const;
		bool next_branch();
		void reset();
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
		bool is_branch_empty() const;
		bool next_branch();

		titleformat_object::ptr m_script;
		pfc::string_formatter m_buffer;

		branch_point_manager m_manager;
	};

	class titleformat_hook_impl_file_info_branch : public titleformat_hook_impl_file_info
	{
	public:
		titleformat_hook_impl_file_info_branch(branch_point_callback & p_callback, const playable_location & p_location,const file_info * p_info);

		virtual bool process_field(titleformat_text_out * p_out, const char * p_name, t_size p_name_length, bool & p_found_flag);
		virtual bool process_function(titleformat_text_out * p_out, const char * p_name, t_size p_name_length, titleformat_hook_function_params * p_params, bool & p_found_flag);

	private:
		bool process_meta_branch(titleformat_text_out * p_out, const char * p_name, t_size p_name_length, bool p_remap, bool & p_found_flag);

	private:
		branch_point_callback & m_callback;
		pfc::map_t<pfc::string8, t_size> m_meta_map;
		pfc::map_t<pfc::string8, t_size> m_meta_remap_map;
	};
}
