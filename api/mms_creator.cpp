// SYSTEM
#include <memory> // make_shared
// LOCAL
#include "mms_creator.h" // IMPLEMENTS
#include "err_codes.h" // USES
#include "scope_guard.h" // USES
#define TEST
namespace mms
{
	void creator::create_table( sql_query_string const & mms_sql_query_, std::string const & db_path )
	{
		mms_sql_query = mms_sql_query_;
		return create_table(db_path);
	}

	void creator::create_table( std::string const & db_path )
	{
		parse_query();
		return create_table( create_clause, db_path );
	}

	void creator::create_table( create_clause_container const & update_clause, std::string const & db_path )
	{
		auto code = create_compiler_impl();
		if ( code != SUCCESS ) throw code;
		code = create_table_in_db(sql_query, db_path);
		if ( code != SUCCESS ) throw code;
	}

	void creator::parse_query()
	{
		if ( !mms::parse_action<create_parser<action_str,cit>>( action, mms_sql_query.c_str(), create_clause, sql_query.size() ) ) throw PARSER;	
	}
	
	void creator::begin_query()
	{
		after_parse_query(); // may be redefined in 'descendants'
		sql_query = "create table if not exists";
	}

	void creator::add_table()
	{
		int const & code = mms::add_table( create_clause.columns.at(0), sql_query );
		if ( code != SUCCESS ) throw code;
	}

	int creator::create_compiler_impl()
	{
		try
		{
			if ( !validate_mms_sql_query() ) return MMS_SQL_QUERY_VALIDATION;
			begin_query();
			add_table();
			sql_query += opened_parenthesis;
			add_columns();
			sql_query += closed_parenthesis + semicolon;
#ifdef TEST
			std::cout << sql_query;
#endif
		}
		catch ( std::runtime_error const & )
		{
			return BROKEN_CREATE_POLICY_BEHAVIOR;
		}
		catch ( int const & code )
		{
			return code;
		}
		return SUCCESS;
	}

	sql_query_string creator::add_constraints(  unsigned const & idx )
	{
		sql_query_string fk;
		try
		{
			if ( create_clause.constraints.size() > idx && create_clause.constraints.at(idx).primary ) sql_query += space + primary_key;
			if ( create_clause.constraints.size() > idx && !create_clause.constraints.at(idx).null ) sql_query += space + null;
			else sql_query += space + not + space + null;
			if ( create_clause.constraints.size() > idx ) sql_query += create_clause.constraints.at(idx).def_value.apply_visitor( ::mms::constraint_def_val_visitor() );
			if ( create_clause.constraints.size() > idx && create_clause.constraints.at(idx).foreign ) 
			{
				fk= comma;
				fk += space + foreign_key + opened_parenthesis + single_quote + create_clause.columns.at(idx) + single_quote + closed_parenthesis + space + references;
				auto const & citer = std::find_if( 
													create_clause.satellite_data.cbegin(), 
													create_clause.satellite_data.cend(), 
													[this,idx](mms::column_and_satellite_data_item_container const & item)->bool
													{
														return item.first == create_clause.columns.at(idx);
													}
												 );
				if ( citer == create_clause.satellite_data.cend() ) throw std::runtime_error("Failed to add the FOREIGN constraint : No constraint satellite data was found");
				auto split_col = mms::split_column( citer->second.second );
				if ( split_col.size() < 2 ) throw COLUMN_DELIMITER_WAS_NOT_FOUND;
				fk += space + split_col.at(0) + opened_parenthesis + single_quote + citer->second.second + single_quote + closed_parenthesis;
			}
		}
		catch ( std::out_of_range const & )
		{
			throw std::runtime_error( "Failed to add constraints : No correspondent column was found" );
		}
		catch ( int const & )
		{
			throw;
		}
		return fk;
	}

	void creator::add_type( unsigned const & idx )
	{
		sql_query += space;
		switch ( create_clause.types.at(idx).v )
		{
		case mms::sql_data_type::bool_: sql_query += boolean;
			break;
		case mms::sql_data_type::datetime: sql_query += datetime;
			break;
		case mms::sql_data_type::double_: sql_query += double_;
			break;
		case mms::sql_data_type::int_: sql_query += integer;
			break;
		case mms::sql_data_type::varchar: 
			auto const & cit = std::find_if( 
												create_clause.satellite_data.cbegin(), 
												create_clause.satellite_data.cend(), 
												[this,idx](mms::column_and_satellite_data_item_container const & item)->bool
												{
													return item.first == create_clause.columns.at(idx);
												}
										   );
			if ( cit != create_clause.satellite_data.cend() ) sql_query += varchar + opened_parenthesis + ::boost::lexical_cast<::std::string>(cit->second.first) + closed_parenthesis;
			else throw std::runtime_error( "Failed to add VARCHAR type : No constraint satellite data was found" );
			break;
		}
	}

	void creator::add_columns()
	{
		auto const sql_q = sql_query;
		auto sql_query_guard = make_guard_0( [sql_q,this]()
		{
			sql_query = sql_q;
		});
		std::vector<sql_query_string> foreign_keys;
		for ( auto idx = 0U; idx < create_clause.columns.size(); ++idx )
		{
			append_column_name( sql_query, create_clause.columns.at(idx) );
			try
			{
				add_type( idx );
				auto const & fk = add_constraints( idx );
				foreign_keys.push_back(fk);
			}
			catch ( std::runtime_error const &  )
			{
//				std::cout <<idx;
				throw;
			}
			catch ( int const & )
			{
//				std::cout <<idx;
				throw;
			}
			if ( idx + 1 != create_clause.columns.size() ) sql_query += comma;
		}
		for ( auto const & fk : foreign_keys ) sql_query += fk;
		sql_query_guard.dismiss();
	}

	bool creator::validate_mms_sql_query()
	{
		typedef rule::rule_ptr policy_ptr;
		policy_ptr create_clause_plop( new (std::nothrow) create_clause_policy );
		if ( create_clause_plop == nullptr ) throw MEM_ALLOC;
		try
		{
			create_clause_plop->add_rule( std::make_shared<non_empty_columns_list_rule>() );
			create_clause_plop->add_rule( std::make_shared<dif_column_names_list_rule>() );
			create_clause_plop->add_rule( std::make_shared<columns_list_equal_to_types_list_rule>() );
			create_clause_plop->add_rule( std::make_shared<varchar_satellite_data_rule>() );
			create_clause_plop->add_rule( std::make_shared<foreign_satellite_data_rule>() );
		}
		catch ( std::bad_alloc const & )
		{
			throw MEM_ALLOC;
		}
		return create_clause_plop->validate( create_clause );
	}
	

} // namespace mms