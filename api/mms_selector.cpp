// SYSTEM
#include <memory> // make_shared
// LOCAL
#include "mms_selector.h" // IMPLEMENTS
#include "err_codes.h" // USES
#define TEST
namespace mms
{
	selector::data_deque_container selector::select( sql_query_string const & mms_sql_query_, std::string const & db_path )
	{
		mms_sql_query = mms_sql_query_;
		return select(db_path);
	}

	selector::data_deque_container selector::select( std::string const & db_path )
	{
		parse_query();
		return select( select_clause, db_path );
	}

	selector::data_deque_container selector::select( select_clause_container const & select_clause, std::string const & db_path )
	{
		auto code = select_compiler_impl();
		if ( code != SUCCESS ) throw code;
		code = select_from_db(sql_query, db_path);
		if ( code != SUCCESS ) throw code;
		return data;
	}

	void selector::parse_query()
	{
		if ( !mms::parse_action<select_parser<action_str,cit>>( action, mms_sql_query.c_str(), select_clause, sql_query.size() ) ) throw PARSER;	
	}
	
	void selector::begin_query()
	{
		after_parse_query(); // may be redefined in 'descendants'
		sql_query = "select";
		add_distinct_expr();
	}

	void selector::add_distinct_expr()
	{
		sql_query_string distinct_str = "distinct", all_str = "all";
		sql_query_string const space(1,' '), comma(1,',');
		sql_query += space;
		sql_query += select_clause.distinct ? distinct_str : all_str;
	}

	void selector::add_columns_and_aggregates()
	{
		mms::add_columns_and_aggregates( select_clause, sql_query );
	}

	void selector::add_tables()
	{
		int code = mms::add_cross_joined_tables( select_clause, sql_query );
		if ( code != SUCCESS ) throw code;
	}

	void selector::add_filters()
	{
		mms::add_filters( select_clause, sql_query );
	}

	void selector::add_groups()
	{
		mms::add_groups( select_clause, sql_query );
	}

	void selector::add_groups_filters()
	{
		mms::add_defined_filters( select_clause, sql_query, true );
	}
	
	void selector::add_order()
	{
		mms::add_order( select_clause, sql_query );
	}
	
	void selector::add_limit_and_offset()
	{
		mms::add_limit_and_offset( select_clause, sql_query );
	}

	int selector::select_compiler_impl()
	{
		try
		{
			if ( !validate_mms_sql_query() ) return MMS_SQL_QUERY_VALIDATION;
			begin_query();
			add_columns_and_aggregates();
			add_tables();
			add_filters();
			add_groups();
			add_groups_filters();
			add_order();
			add_limit_and_offset();
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

	bool selector::validate_mms_sql_query()
	{
		typedef rule::rule_ptr policy_ptr;
		policy_ptr select_clause_plop( new (std::nothrow) select_clause_policy );
		if ( select_clause_plop == nullptr ) throw MEM_ALLOC;
		try
		{
			policy_ptr filter_names_policy_ptr = std::make_shared<filter_names_policy>();
			filter_names_policy_ptr->add_rule( std::make_shared<single_rule_1>() ), filter_names_policy_ptr->add_rule( std::make_shared<single_rule_2>() ),
				filter_names_policy_ptr->add_rule( std::make_shared<single_rule_3>() ), filter_names_policy_ptr->add_rule( std::make_shared<single_rule_4>() );
			select_clause_plop->add_rule( filter_names_policy_ptr );
			policy_ptr group_names_policy_ptr = std::make_shared<groups_names_policy>();
			group_names_policy_ptr->add_rules( filter_names_policy_ptr );
			//group_names_policy_ptr->delete_rule( 0 );
			select_clause_plop->add_rule( group_names_policy_ptr );
			policy_ptr group_flt_names_policy_ptr = std::make_shared<group_filters_names_policy>();
			group_flt_names_policy_ptr->add_rules( group_names_policy_ptr );
			select_clause_plop->add_rule( group_flt_names_policy_ptr );
			policy_ptr order_names_policy_ptr = std::make_shared<order_names_policy>();
			order_names_policy_ptr->add_rules( group_names_policy_ptr );
			select_clause_plop->add_rule( order_names_policy_ptr );
		}
		catch ( std::bad_alloc & )
		{
			throw MEM_ALLOC;
		}
		return select_clause_plop->validate( select_clause );
	}

} // namespace mms