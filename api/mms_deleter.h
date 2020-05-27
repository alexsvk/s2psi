#ifndef mms_deleter_h
#define mms_deleter_h

#include "sql_gen_helper.h" // USES
#include "db.h" // USES

namespace mms
{

class deleter
{
public:
	typedef std::string data_atom;
	deleter( action_str const & action_, sql_query_string const & mms_sql_query_ = ""): mms_sql_query( mms_sql_query_ ), action( action_ )
	{
	}
	/*!
	\brief Updates a data base located in db_path.
	\param sql_query_ The MMS SQL query.
	\param db_path The path to the data base to be updated.
	\throws std::runtime_error.
	*/
	virtual void delete_( sql_query_string const & sql_query_, std::string const & db_path );
	/*!
	\brief Updates a data base located in db_path.
	\throws std::runtime_error.
	*/
	virtual void delete_( std::string const & db_path );

protected:
	virtual bool validate_mms_sql_query();
	virtual void parse_query();
	virtual void after_parse_query()
	{
	}
	virtual void add_table();
	virtual void add_filters();
	// In a RDBMS it should have an implementation.
	virtual int delete_from_db(sql_query_string const & sql_query, std::string const & db_path) = 0;
protected:
	sql_query_string sql_query, mms_sql_query;
	delete_clause_container delete_clause;
	action_str action;
private:
	/*!
	\brief Updates a data base located in db_path.
	\throws std::runtime_error.
	*/
	void delete_( delete_clause_container const & select_clause, std::string const & db_path );
	void begin_query();
	int delete_compiler_impl();
};

class sqlite3_deleter: public deleter
{
public:
	sqlite3_deleter( action_str const & action, sql_query_string const & sql_query_ = "" ): deleter( action, sql_query_ )
	{
	}
protected:
	int delete_from_db(sql_query_string const & sql_query, std::string const & db_path) override
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
			db_.delete_( sql_query );
		}
		catch ( std::runtime_error & )
		{
			return DEL_FROM_DB;
		}
		return SUCCESS;		
	}
};

} // namespace mms

#endif