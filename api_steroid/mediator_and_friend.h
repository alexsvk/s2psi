#ifndef mediator_and_friend_h
#define mediator_and_friend_h

#include "err_codes.h" // uses
#include "sql_gen_helper.h" // uses sql_query_string

namespace mms
{

template<class selector_impl>
class mediator_and_friend: public selector_impl
{
public:
	mediator_and_friend(sql_query_string const & column_filter, sql_query_string const & select_from_expr_, sql_query_string const & mms_sql_query): 
		sqlite3_selector(mediator_and_friend_action, mms_sql_query), col_filter(column_filter), select_from_expr(select_from_expr_)
	{
	}
	void after_parse_query() override
	{
		for ( auto const & flt : select_clause.filters ) if ( flt.first != col_filter ) throw MEDIATOR_AND_FRIEND_MAIN_FILTER;
	}
	void add_tables() override
	{
		sql_query += space + select_from_expr;
	}
private:
	sql_query_string const col_filter, select_from_expr;

};

} // namespace mms
#endif