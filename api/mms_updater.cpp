// SYSTEM
#include <memory> // make_shared
// LOCAL
#include "mms_updater.h" // IMPLEMENTS
#include "err_codes.h" // USES
#include "sql_gen_helper.h" // USES
//#define TEST
namespace mms
{
	void updater::update( sql_query_string const & mms_sql_query_, std::string const & db_path )
	{
		mms_sql_query = mms_sql_query_;
		return update(db_path);
	}

	void updater::update( std::string const & db_path )
	{
		parse_query();
		return update( update_clause, db_path );
	}

	void updater::update( update_clause_container const & update_clause, std::string const & db_path )
	{
		auto code = update_compiler_impl();
		if ( code != SUCCESS ) throw code;
		code = update_db(sql_query, db_path);
		if ( code != SUCCESS ) throw code;
	}

	void updater::parse_query()
	{
		if ( !mms::parse_action<update_parser<action_str,cit>>( action, mms_sql_query.c_str(), update_clause, sql_query.size() ) ) throw PARSER;	
	}
	
	void updater::begin_query()
	{
		after_parse_query(); // may be redefined in 'descendants'
		sql_query = "update";
	}

	void updater::add_columns_and_values()
	{
		sql_query += space + "set";
		for ( auto idx = 0U; idx < update_clause.values.size(); ++idx )
		{
			append_column_name( sql_query, update_clause.columns.at(idx) ) += equality_ch;
			sql_query += update_clause.values.at(idx).apply_visitor( mms::value_visitor() );
			if ( idx + 1 != update_clause.values.size() ) sql_query += comma;
		}
	}

	void updater::add_table()
	{
		int const & code = mms::add_table( update_clause, sql_query );
		if ( code != SUCCESS ) throw code;
	}

	void updater::add_filters()
	{
		mms::add_filters( update_clause, sql_query );
	}

	int updater::update_compiler_impl()
	{
		try
		{
			if ( !validate_mms_sql_query() ) return MMS_SQL_QUERY_VALIDATION;
			begin_query();
			add_table();
			add_columns_and_values();
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

	bool updater::validate_mms_sql_query()
	{
		return update_clause.columns.size() == update_clause.values.size();
	}

} // namespace mms