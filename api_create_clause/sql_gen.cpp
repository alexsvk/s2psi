#pragma comment(lib,"..\\Debug\\sqlite3")
// PROJECT
//#include <boost/lexical_cast.hpp> // uses
// LOCAL
#include "../api/err_codes.h" // uses
#include "../api/sql_gen_helper.h" // select_impl, GENERATE_MMS_MEDIATOR, GENERATE_MMS_MEDIATOR_AND_FRIEND
//#include "../api/sqlite3_creator.h" //uses
#define BUILDING_THE_DLL
#include "../api/sql_gen.h" // implements
#include "../api/db.h" // uses
extern "C" EXPORTED void deallocate_selected_data(selection_double_raw_ptr double_raw_p, unsigned const & size)
{
	mms::deallocate( double_raw_p, size );
}
struct constraint_def_val_visitor : public ::boost::static_visitor<std::string>
{
	std::string operator()(std::string const & val) const
	{
		mms::sql_query_string const quote = "\"";
		return quote + val + quote;
	}
	std::string operator()(double const & val) const
	{
		return to_string(val);
	}
	std::string operator()(int const & val) const
	{
		return to_string(val);
	}
	std::string operator()(bool const & val) const
	{
		return to_string(val);
	}
	std::string operator()(mms::not_used_tag const & val) const
	{
		return " ";
	}
private:
	template <typename val_t>
	std::string to_string(val_t const & val) const
	{
		return " default " + ::boost::lexical_cast<std::string>(val);
	}
};

bool add_type( mms::create_clause_container const & create_clause, mms::sql_query_string & sql_q, unsigned const & idx )
{
	if ( create_clause.types.size() <= idx ) return false;
	switch ( create_clause.types.at(idx).v )
	{
	case mms::sql_data_type::bool_: sql_q += " BOOLEAN";
		break;
	case mms::sql_data_type::datetime: sql_q += " DATETIME";
		break;
	case mms::sql_data_type::double_: sql_q += " DOUBLE";
		break;
	case mms::sql_data_type::int_: sql_q += " INT";
		break;
	case mms::sql_data_type::varchar: 
		if ( create_clause.columns.size() <= idx ) return false;
		auto const citer = std::find_if( 
											create_clause.satellite_data.cbegin(), 
											create_clause.satellite_data.cend(), 
											[create_clause,idx](mms::column_and_satellite_data_item_container const & item)->bool
											{
												return item.first == create_clause.columns.at(idx);
											}
								   );
		if ( citer == create_clause.satellite_data.cend() ) return false;
		sql_q += " VARCHAR(" + ::boost::lexical_cast<::std::string>(citer->second.first) + ")";
		break;
	}
	return true;
}

bool add_constraints( mms::create_clause_container const & create_clause, mms::sql_query_string & sql_q, unsigned const & idx )
{
	if ( create_clause.constraints.at(idx).primary ) sql_q += " primary key";
	if ( create_clause.constraints.at(idx).null ) sql_q += " null";
	else sql_q += " not null";
	//if ( create_clause.constraints.at(idx).values.size() <= idx ) return false; // N.B. !!! There should be always all values set.
	sql_q += create_clause.constraints.at(idx).def_value.apply_visitor( constraint_def_val_visitor() );
	if ( create_clause.constraints.at(idx).foreign ) 
	{
		sql_q += ',';
		if ( create_clause.columns.size() <= idx ) return false;
		sql_q += " foreign key(" + create_clause.columns.at(idx) + ") references";
		auto const citer = std::find_if( 
											create_clause.satellite_data.cbegin(), 
											create_clause.satellite_data.cend(), 
											[create_clause,idx](mms::column_and_satellite_data_item_container const & item)->bool
											{
												return item.first == create_clause.columns.at(idx);
											}
										);
		if ( citer == create_clause.satellite_data.cend() ) return false;
		auto split_col = mms::split_column( citer->second.second );
		if ( split_col.size() != 2 ) return false;
		sql_q += " " + split_col.at(0) + "(" + citer->second.second + ")";
	}
	return true;
}

int create_db_table( mms::sql_query_string const & sql_q, char const* path )
{
	mms::db db_;
	try
	{
		db_.open(path);
	}
	catch ( ::std::runtime_error & )
	{
		return OPEN_DB;
	}
	try
	{
		db_.create_table( sql_q );
	}
	catch ( ::std::runtime_error & )
	{
		return CREATE_TABLE;
	}
	return SUCCESS;
}

template <typename clause>
bool add_table_name(clause const & clause_, mms::sql_query_string & sql_q )
{
	mms::sql_query_string const space(1, ' ');
	try
	{
		sql_q += space + mms::split_column( clause_.columns.at(0) ).at(0);
	}
	catch ( ::std::out_of_range const & )
	{
		return false;
	}
	return true;
}
extern "C" EXPORTED int create_(sql_spec_ptr const q_ptr, unsigned const & len, char const* path)
{
	if ( q_ptr != nullptr && path != nullptr && len != 0 )
	{
		mms::create_clause_container create_clause;
		if ( !mms::parse_action<mms::create_parser<mms::action_str,mms::cit>>( mms::create_action, q_ptr, create_clause, len ) ) return -1;
		if ( create_clause.columns.size() == 0 ) return MMS_SQL_QUERY_VALIDATION;
		mms::sql_query_string sql_q = "create table if not exists";
		if ( !add_table_name( create_clause, sql_q ) ) return CREATE_TABLE;
		sql_q += '(';
		for ( auto idx = 0U; idx < create_clause.columns.size(); ++idx )
		{
			sql_q += create_clause.columns.at(idx);
			if ( !add_type( create_clause, sql_q, idx ) ) return CREATE_COMPILER_ADD_COLUMN_TYPE;
			if ( !add_constraints( create_clause, sql_q, idx ) ) return CREATE_COMPILER_ADD_COLUMN_CONSTRAINT;
			if ( idx + 1 != create_clause.columns.size() ) sql_q += ',';
		}
		sql_q += ");";
		return create_db_table( sql_q, path );
	}
	return MALFORMED_PARAMS;
}