#ifndef sqlite3_selector_h
#define sqlite3_selector_h

// SYSTEM
#include <functional> // function
// LOCAL
#include "mms_selector.h" // inherits

namespace mms
{

	class sqlite3_selector: public selector
	{
	public:
		typedef int(data_handle_raw_ptr)(void*,int cols_cnt,char** columns,char**);
		typedef std::function<data_handle_raw_ptr> sqlite3_data_handle;
		sqlite3_selector( action_str const & action, sql_query_string const & sql_query_ = "" ): selector( action, sql_query_ ), data_handle_func( selected_data_handle )
		{
		}
		void set_selected_data_handle( sqlite3_data_handle const & data_handle_func_ = selected_data_handle )
		{
			data_handle_func = data_handle_func_;
		}
		
		static int selected_data_handle( void* buffer, int columns_cnt, char** columns, char** columns_names );
		data_deque_container const & get_data() const
		{
			return data;
		}
		data_deque_container & get_data()
		{
			return data;
		}
		int select_from_db(sql_query_string const & sql_query, std::string const & db_path) override;
	private:
		sqlite3_data_handle data_handle_func;
	};

} // namespace mms

#endif