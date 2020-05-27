// SYSTEM
#include <memory> // make_shared
// LOCAL
#include "mms_deleter.h" // IMPLEMENTS
#include "err_codes.h" // USES
//#define TEST
namespace mms
{
	void deleter::delete_( sql_query_string const & mms_sql_query_, std::string const & db_path )
	{
		mms_sql_query = mms_sql_query_;
		return delete_(db_path);
	}

	void deleter::delete_( std::string const & db_path )
	{
		parse_query();
		return delete_( delete_clause, db_path );
	}

	void deleter::delete_( delete_clause_container const & update_clause, std::string const & db_path )
	{
		auto code = delete_compiler_impl();
		if ( code != SUCCESS ) throw code;
		code = delete_from_db(sql_query, db_path);
		if ( code != SUCCESS ) throw code;
	}

	void deleter::parse_query()
	{
		if ( !mms::parse_action<delete_parser<action_str,cit>>( action, mms_sql_query.c_str(), delete_clause, sql_query.size() ) ) throw PARSER;	
	}
	
	void deleter::begin_query()
	{
		after_parse_query(); // may be redefined in 'descendants'
		sql_query = "delete from";
	}

	void deleter::add_table()
	{
		int const & code = mms::add_table( delete_clause.filters.at(0).first, sql_query );
		if ( code != SUCCESS ) throw code;
	}

	void deleter::add_filters()
	{
		mms::add_filters( delete_clause, sql_query );
	}

	int deleter::delete_compiler_impl()
	{
		try
		{
			if ( !validate_mms_sql_query() ) return MMS_SQL_QUERY_VALIDATION;
			begin_query();
			add_table();
			add_filters();
#ifdef TEST
			std::cout << sql_query;
#endif
		}
		catch ( int const & code )
		{
			return code;
		}
		return SUCCESS;
	}

	bool deleter::validate_mms_sql_query()
	{
		return delete_clause.filters.size() > 0;
	}

} // namespace mms