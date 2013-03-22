#include "stdafx.h"
#include "branch_formatter.h"

namespace multiformat
{
	branch_point_manager::branch_point_manager()
		: m_current(0)
	{
	}

	branch_point_manager::~branch_point_manager()
	{
	}

	t_size branch_point_manager::add_branch_point(const branch_point &p_branch_point)
	{
		return m_stack.add_item(p_branch_point);
	}

	bool branch_point_manager::next_branch_point()
	{
		if (m_current < m_stack.get_count())
		{
			m_current += 1;
		}

		return (m_current < m_stack.get_count());
	}

	bool branch_point_manager::is_branch_empty() const
	{
		for (t_size n = 0; n < m_stack.get_count(); ++n)
		{
			if (m_stack[n].is_empty())
			{
				return true;
			}
		}
		return false;
	}

	bool branch_point_manager::next_branch()
	{
		for (t_size n = m_stack.get_count(); n > 0; --n)
		{
			m_stack[n-1].next();
			if (m_stack[n-1].has_next())
			{
				return true;
			}
			else
			{
				m_stack.remove_by_idx(n-1);
			}
		}
		return false;
	}

	void branch_point_manager::set_current_branch_point_index(t_size p_index)
	{
		m_current = p_index;
	}

	t_size branch_point_manager::get_current_branch_point_index() const
	{
		return m_current;
	}

	const branch_point & branch_point_manager::get_branch_point(t_size p_index) const
	{
		return m_stack[p_index];
	}

	void branch_point_manager::reset()
	{
		m_stack.remove_all();
		// Push guard element onto stack.
		add_branch_point(branch_point(1));
	}

	t_size branch_point_manager::get_branch_point_count()
	{
		return m_stack.get_count();
	}

	branch_formatter::branch_formatter(titleformat_object::ptr p_script)
		: m_script(p_script)
	{
	}

	void branch_formatter::run_nonlocking(pfc::list_base_t<pfc::string8> & p_list_out, metadb_handle *p_handle, titleformat_text_filter * p_filter)
	{
		const file_info * info = 0;
		if (!p_handle->get_info_locked(info)) return;
		titleformat_hook_impl_file_info_branch hook(p_handle->get_location(), info);

		run_nonlocking(p_list_out, p_handle, 0, &hook, p_filter);
	}

	void branch_formatter::run_nonlocking(pfc::list_base_t<pfc::string8> & p_list_out, metadb_handle *p_handle, titleformat_hook * p_branch_neutral_hook, titleformat_branch_hook * p_branch_aware_hook, titleformat_text_filter * p_filter)
	{
		m_manager.reset();

		titleformat_hook_impl_branch_splitter hook(p_branch_neutral_hook, p_branch_aware_hook, &m_manager);

		do
		{
			m_manager.set_current_branch_point_index(0);

			hook.prepare_branch(m_manager.get_branch_point_count());

			p_handle->format_title_nonlocking(&hook, m_buffer, m_script, p_filter);

			if (!m_manager.is_branch_empty())
			{
				p_list_out.add_item(m_buffer);
			}
		} while (m_manager.next_branch());
	}

	titleformat_hook_impl_branch_splitter::titleformat_hook_impl_branch_splitter(titleformat_hook * p_branch_neutral_hook, titleformat_branch_hook * p_branch_aware_hook, branch_point_callback * p_callback)
		: m_branch_neutral_hook(p_branch_neutral_hook), m_branch_aware_hook(p_branch_aware_hook)
	{
		if (m_branch_aware_hook)
		{
			if (p_callback)
			{
				m_branch_aware_hook->set_callback(p_callback);
			}
			else
			{
				uBugCheck();
			}
		}
	}

	titleformat_hook_impl_branch_splitter::~titleformat_hook_impl_branch_splitter()
	{
		if (m_branch_aware_hook)
		{
			m_branch_aware_hook->set_callback(0);
		}
	}

	void titleformat_hook_impl_branch_splitter::prepare_branch(t_size p_branch_point_limit)
	{
		if (m_branch_aware_hook)
		{
			m_branch_aware_hook->prepare_branch(p_branch_point_limit);
		}
	}

	bool titleformat_hook_impl_branch_splitter::process_field(titleformat_text_out * p_out,const char * p_name,t_size p_name_length,bool & p_found_flag)
	{
		p_found_flag = false;
		if (m_branch_neutral_hook && m_branch_neutral_hook->process_field(p_out,p_name,p_name_length,p_found_flag)) return true;
		p_found_flag = false;
		if (m_branch_aware_hook && m_branch_aware_hook->process_field(p_out,p_name,p_name_length,p_found_flag)) return true;
		p_found_flag = false;
		return false;
	}

	bool titleformat_hook_impl_branch_splitter::process_function(titleformat_text_out * p_out,const char * p_name,t_size p_name_length,titleformat_hook_function_params * p_params,bool & p_found_flag)
	{
		p_found_flag = false;
		if (m_branch_neutral_hook && m_branch_neutral_hook->process_function(p_out,p_name,p_name_length,p_params,p_found_flag)) return true;
		p_found_flag = false;
		if (m_branch_aware_hook && m_branch_aware_hook->process_function(p_out,p_name,p_name_length,p_params,p_found_flag)) return true;
		p_found_flag = false;
		return false;
	}
	
	titleformat_hook_impl_file_info_branch::titleformat_hook_impl_file_info_branch(const playable_location & p_location,const file_info * p_info)
		: m_callback(0), m_location(p_location), m_info(p_info)
	{
	}

	void titleformat_hook_impl_file_info_branch::set_callback(branch_point_callback * p_callback)
	{
		m_callback = p_callback;
	}

	void titleformat_hook_impl_file_info_branch::prepare_branch(t_size p_branch_point_limit)
	{
		remove_invalid_entries(m_meta_map, p_branch_point_limit);
		remove_invalid_entries(m_meta_remap_map, p_branch_point_limit);
	}

	void titleformat_hook_impl_file_info_branch::remove_invalid_entries(pfc::map_t<pfc::string8, t_size> & p_map, t_size p_branch_point_limit)
	{
		typedef pfc::map_t<pfc::string8, t_size>::iterator iterator;

		iterator current = p_map.first();
		while (current != p_map.last())
		{
			if (current->m_value >= p_branch_point_limit)
			{
				iterator obsolete = current;
				++current;
				p_map.remove(obsolete);
			}
			else
			{
				++current;
			}
		}
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

		pfc::map_t<pfc::string8, t_size> & map = p_remap ? m_meta_remap_map : m_meta_map;

		if (m_callback->next_branch_point())
		{
			meta_index = m_callback->get_current_branch_point().get_meta_index();
			value_index = m_callback->get_current_branch_point().get_value_index();
		}
		else
		{
			// Check for previous occurrence of this field.
			pfc::string8 name(p_name, p_name_length);
			// XXX stack index might point to branch point that was removed from the stack.
			if (map.exists(name) && (map[name] < m_callback->get_current_branch_point_index()))
			{
				// Use existing entry.
				t_size stack_index = map[name];
				meta_index = m_callback->get_branch_point(stack_index).get_meta_index();
				value_index = m_callback->get_branch_point(stack_index).get_value_index();
			}
			else
			{
				// Add new entry.
				bool found;
				if (p_remap)
				{
					found = remap_meta(meta_index, p_name, p_name_length);
				}
				else
				{
					meta_index = m_info->meta_find_ex(p_name, p_name_length);
					found = (meta_index != pfc::infinite_size);
				}

				if (found)
				{
					t_size value_count = m_info->meta_enum_value_count(meta_index);
					t_size branch_point_index = m_callback->add_branch_point(branch_point(pfc::max_t<t_size>(1, value_count), meta_index));
					value_index = 0;
					map[name] = branch_point_index;
				}
				else
				{
					m_callback->add_branch_point(branch_point(1));
					p_found_flag = false;
					return true;
				}
			}
		}

		p_out->write(titleformat_inputtypes::meta, m_info->meta_enum_value(meta_index, value_index));

		p_found_flag = true;
		return true;
	}

} // namespace multiformat