#include "stdafx.h"
#include "branch_formatter.h"

namespace multiformat
{
	boolean branch_point::is_empty() const
	{
		return get_definition().is_empty();
	}

	void branch_point::next()
	{
		if (!m_reference) get_definition().next();
	}

	boolean branch_point::is_done() const
	{
		if (m_reference)
		{
			return true;
		}
		else
		{
			return get_definition().is_done();
		}
	}

	void branch_point::remove()
	{
		if (!m_reference) m_definitions->remove_by_idx(m_definition_index);
	}

	bool branch_point::matches(const char * p_name, t_size p_name_length, bool p_remapped) const
	{
		return get_definition().matches(p_name, p_name_length, p_remapped);
	}

	void branch_point::get(t_size & p_meta_index, t_size & p_value_index, bool & p_found_flag) const
	{
		get_definition().get(p_meta_index, p_value_index, p_found_flag);
	}

	branch_point_manager::branch_point_manager()
		: m_branch_point_index(0)
	{
	}

	branch_point_manager::~branch_point_manager()
	{
	}

	bool branch_point_manager::replay_definition(const char * p_name, t_size p_name_length, bool p_remapped, t_size & p_meta_index, t_size & p_value_index, bool & p_found_flag)
	{
		if (m_branch_point_index < m_branch_points.get_count())
		{
			if (m_branch_points[m_branch_point_index].matches(p_name, p_name_length, p_remapped))
			{
				// Replay existing definition.
				m_branch_points[m_branch_point_index].get(p_meta_index, p_value_index, p_found_flag);
				++m_branch_point_index;
				return true;
			}
			else
			{
				// Replay is inconsistent.
				for (t_size n = m_branch_points.get_count(); n > m_branch_point_index; --n)
				{
					m_branch_points[n-1].remove();
				}
				m_branch_points.remove_from_idx(m_branch_point_index, m_branch_points.get_count() - m_branch_point_index);

				m_done = true;
				return false;
			}
		}
		else
		{
			// Replay ended.
			return false;
		}
	}

	void branch_point_manager::add_definition(const char * p_name, t_size p_name_length, bool p_remapped, t_size p_meta_index, t_size p_value_count, bool p_found_flag)
	{
		if (m_branch_point_index < m_branch_points.get_count())
		{
			// Replay is inconsistent.
			uBugCheck();
		}

		t_size definition_index = m_definitions.add_item(meta_branch_definition(p_name, p_name_length, p_remapped, p_meta_index, p_value_count, p_found_flag));
		m_branch_points.add_item(branch_point(&m_definitions, definition_index, false));
		++m_branch_point_index;
	}

	bool branch_point_manager::add_reference(const char * p_name, t_size p_name_length, bool p_remapped, t_size & p_meta_index, t_size & p_value_index, bool & p_found_flag)
	{
		if (m_branch_point_index < m_branch_points.get_count())
		{
			// Replay is inconsistent.
			uBugCheck();
		}

		for (t_size n = 0; n < m_definitions.get_count(); ++n)
		{
			if (m_definitions[n].matches(p_name, p_name_length, p_remapped))
			{
				m_branch_points.add_item(branch_point(&m_definitions, n, true));
				m_definitions[n].get(p_meta_index, p_value_index, p_found_flag);
				++m_branch_point_index;
				return true;
			}
		}

		return false;
	}

	bool branch_point_manager::is_branch_empty() const
	{
		for (t_size n = 0; n < m_branch_points.get_count(); ++n)
		{
			if (m_branch_points[n].is_empty())
			{
				return true;
			}
		}
		return false;
	}

	bool branch_point_manager::is_done() const
	{
		return m_done;
	}

	void branch_point_manager::next_branch()
	{
		m_branch_point_index = 0;

		for (t_size n = m_branch_points.get_count(); n > 0; --n)
		{
			m_branch_points[n-1].next();
			if (m_branch_points[n-1].is_done())
			{
				m_branch_points[n-1].remove();
				m_branch_points.remove_by_idx(n-1);
			}
			else
			{
				return;
			}
		}

		m_done = true;
	}

	t_size branch_point_manager::get_current_branch_point_index() const
	{
		return m_branch_point_index;
	}

	void branch_point_manager::reset()
	{
		m_branch_points.remove_all();
		m_branch_point_index = 0;
		m_done = false;
	}

	t_size branch_point_manager::get_branch_point_count()
	{
		return m_branch_points.get_count();
	}

	branch_formatter::branch_formatter(titleformat_object::ptr p_script)
		: m_script(p_script)
	{
	}

	void branch_formatter::run_nonlocking(pfc::list_base_t<pfc::string8> & p_list_out, metadb_handle *p_handle, titleformat_text_filter * p_filter)
	{
		const file_info * info = 0;
		if (!p_handle->get_info_locked(info)) return;
		titleformat_hook_impl_file_info_branch branch_hook(m_manager, p_handle->get_location(), info);

		m_manager.reset();

		//titleformat_hook_impl_splitter hook(&branch_hook, &memoize_hook);
		titleformat_hook & hook = branch_hook;

		while (!m_manager.is_done())
		{
			//memoize_hook.prepare_branch(m_manager.get_branch_point_count());

			p_handle->format_title_nonlocking(&hook, m_buffer, m_script, p_filter);

			if (!m_manager.is_branch_empty())
			{
				p_list_out.add_item(m_buffer);
			}

			m_manager.next_branch();
		}
	}

	titleformat_hook_impl_file_info_branch::titleformat_hook_impl_file_info_branch(branch_point_callback & p_callback, const playable_location & p_location,const file_info * p_info)
		: m_callback(p_callback), m_location(p_location), m_info(p_info)
	{
	}

	bool titleformat_hook_impl_file_info_branch::process_function(titleformat_text_out *p_out, const char *p_name, t_size p_name_length, titleformat_hook_function_params *p_params, bool &p_found_flag)
	{
		if (0 == pfc::strcmp_ex(p_name, p_name_length, "meta_branch_remap", pfc::infinite_size))
		{
			if (1 == p_params->get_param_count())
			{
				const char *field_name = 0;
				t_size field_name_length = 0;
				p_params->get_param(0, field_name, field_name_length);
				return process_meta_branch(p_out, field_name, field_name_length, true, p_found_flag);
			}
			else
			{
				p_found_flag = false;
				return true;
			}
		}
		else if (0 == pfc::strcmp_ex(p_name, p_name_length, "meta_branch", pfc::infinite_size))
		{
			if (1 == p_params->get_param_count())
			{
				const char *field_name = 0;
				t_size field_name_length = 0;
				p_params->get_param(0, field_name, field_name_length);
				return process_meta_branch(p_out, field_name, field_name_length, false, p_found_flag);
			}
			else
			{
				p_found_flag = false;
				return true;
			}
		}

		return false;
	}

	bool titleformat_hook_impl_file_info_branch::process_field(titleformat_text_out *p_out, const char *p_name, t_size p_name_length, bool &p_found_flag)
	{
		if ((p_name_length >= 2) && (p_name[0] == '<') && (p_name[p_name_length-1] == '>'))
		{
			return process_meta_branch(p_out, p_name+1, p_name_length-2, true, p_found_flag);
		}
		else
		{
			return false;
		}
	}

	bool titleformat_hook_impl_file_info_branch::process_meta_branch(titleformat_text_out *p_out, const char *p_name, t_size p_name_length, bool p_remap, bool &p_found_flag)
	{
		t_size meta_index;
		t_size value_index;

		if (m_callback.replay_definition(p_name, p_name_length, p_remap, meta_index, value_index, p_found_flag))
		{
			// Use replayed definition.
		}
		else
		{
			if (m_callback.add_reference(p_name, p_name_length, p_remap, meta_index, value_index, p_found_flag))
			{
				// Use existing definition.
			}
			else
			{
				// Add new definition.
				if (p_remap)
				{
					p_found_flag = remap_meta(meta_index, p_name, p_name_length);
				}
				else
				{
					meta_index = m_info->meta_find_ex(p_name, p_name_length);
					p_found_flag = (meta_index != pfc::infinite_size);
				}

				if (p_found_flag)
				{
					t_size value_count = m_info->meta_enum_value_count(meta_index);
					value_index = 0;
					m_callback.add_definition(p_name, p_name_length, p_remap, meta_index, pfc::max_t(t_size(1), value_count), true);
				}
				else
				{
					m_callback.add_definition(p_name, p_name_length, p_remap, 0, 1, false);
				}
			}
		}

		if (p_found_flag)
		{
			p_out->write(titleformat_inputtypes::meta, m_info->meta_enum_value(meta_index, value_index));
		}

		// p_found_flag is already set.
		return true;
	}

} // namespace multiformat