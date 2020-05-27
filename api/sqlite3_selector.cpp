#include "sqlite3_selector.h" // implements
#include "db.h" // uses

namespace mms
{

	int sqlite3_selector::select_from_db(sql_query_string const & sql_query, std::string const & db_path)
	{
		mms::db db_;
		try
		{
			db_.open( db_path );
		}
		catch ( std::runtime_error & )
		{
			return OPEN_DB;
		}
		try
		{
			data_deque_container data_;
			auto const & data_handle_double_raw_ptr = data_handle_func.target<data_handle_raw_ptr*>();
			if ( data_handle_double_raw_ptr == nullptr ) return NO_SQLITE3_DATA_HANDLE_IS_SET;
			db_.select( sql_query, *data_handle_double_raw_ptr, &data_ );
			data = data_;
		}
		catch ( std::runtime_error & )
		{
			return SELECT_FROM_DB;
		}
		return SUCCESS;	
	}

	int sqlite3_selector::selected_data_handle( void* buffer, int columns_cnt, char** columns, char** columns_names )
	{
		if ( buffer == nullptr ) return 1; // SQLITE_ABORT is returned
		char const* null = "null";
		for ( auto a = 0; a < columns_cnt; ++a ) 
			if ( columns[a] == nullptr ) static_cast<data_deque_container*>(buffer)->push_back( null );
			else static_cast<data_deque_container*>(buffer)->push_back( columns[a] );
		return 0;
	}

} // namespace mms