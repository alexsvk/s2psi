#ifndef mms_selector_h
#define mms_selector_h

#include "sql_gen_helper.h" // USES

namespace mms
{

class selector
{
public:
	typedef std::string data_atom;
	typedef std::deque<data_atom> data_deque_container;
	selector( action_str const & action_, sql_query_string const & mms_sql_query_ = ""): mms_sql_query( mms_sql_query_ ), action( action_ )
	{
	}
	/*!
	\brief Selects data using sql_query and returns a raw pointer to the data.
	\throws std::runtime_error.
	*/
	virtual data_deque_container select( sql_query_string const & sql_query_, std::string const & db_path );
	/*!
	\brief Selects data using internal sql_query and returns a raw pointer to the data.
	\throws std::runtime_error.
	*/
	virtual data_deque_container select( std::string const & db_path );
	data_deque_container & get_data() 
	{
		return data;
	}
protected:
	virtual bool validate_mms_sql_query();
	virtual void parse_query();
	virtual void after_parse_query()
	{
	}
	virtual void add_columns_and_aggregates();
	virtual void add_tables();
	virtual void add_filters();
	virtual void add_groups();
	virtual void add_groups_filters();
	virtual void add_order();
	virtual void add_limit_and_offset();
	// In a RDBMS it should have an implementation.
	virtual int select_from_db(sql_query_string const & sql_query, std::string const & db_path)
	{
		return 0;
	}
protected:
	sql_query_string sql_query, mms_sql_query;
	select_clause_container select_clause;
	data_deque_container data;
	action_str action;
private:
	/*!
	\brief Selects data using parsed MMS SQL query in the select_clause and returns a raw pointer to the data.
	\throws std::runtime_error.
	*/
	data_deque_container select( select_clause_container const & select_clause, std::string const & db_path );
	void begin_query();
	void add_distinct_expr();
	int select_compiler_impl();
};

} // namespace mms

#endif