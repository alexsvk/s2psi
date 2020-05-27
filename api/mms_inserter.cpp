// SYSTEM
#include <memory> // make_shared
// LOCAL
#include "mms_inserter.h" // IMPLEMENTS
#include "err_codes.h" // USES
//#define TEST
namespace mms
{
	long long inserter::insert( sql_query_string const & mms_sql_query_, std::string const & db_path )
	{
		mms_sql_query = mms_sql_query_;
		return insert(db_path);
	}

	long long inserter::insert( std::string const & db_path )
	{
		parse_query();
		return insert( insert_clause, db_path );
	}

	long long inserter::insert( insert_clause_container const & update_clause, std::string const & db_path )
	{
		long long code = insert_compiler_impl();
		if ( code != SUCCESS ) throw code;
		code = insert_into_db(sql_query, db_path);
		if ( code == OPEN_DB || code == INSERT_TO_DB ) throw code;
		return code;
	}

	void inserter::parse_query()
	{
		if ( !mms::parse_action<insert_parser<action_str,cit>>( action, mms_sql_query.c_str(), insert_clause, sql_query.size() ) ) throw PARSER;	
	}
	
	void inserter::begin_query()
	{
		after_parse_query(); // may be redefined in 'descendants'
		sql_query = "insert into";
	}

	void inserter::add_columns()
	{
		sql_query += opened_parenthesis;
		for ( auto idx = 0U; idx < insert_clause.columns.size(); ++idx )
		{
			append_column_name( sql_query, insert_clause.columns.at(idx) );
			if ( idx + 1 != insert_clause.columns.size() ) sql_query += comma;
		}
		sql_query += closed_parenthesis;
	}

	void inserter::add_table()
	{
		int const & code = mms::add_table( insert_clause, sql_query );
		if ( code != SUCCESS ) throw code;
	}

	void inserter::add_values()
	{
		sql_query += space + "values" + opened_parenthesis;
		for ( auto idx = 0U; idx < insert_clause.values.size(); ++idx )
		{
			sql_query += insert_clause.values.at(idx).apply_visitor( value_visitor() );
			if ( idx + 1 != insert_clause.values.size() ) sql_query += comma;
		}
		sql_query += closed_parenthesis + semicolon;
	}

	int inserter::insert_compiler_impl()
	{
		try
		{
			if ( !validate_mms_sql_query() ) return MMS_SQL_QUERY_VALIDATION;
			begin_query();
			add_table();
			add_columns();
			add_values();
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

	bool inserter::validate_mms_sql_query()
	{
		return insert_clause.columns.size() == insert_clause.values.size();
	}

} // namespace mms