#ifndef mms_inserter_h
#define mms_inserter_h

#include "sql_gen_helper.h" // USES
#include "db.h" // USES

namespace mms
{

class inserter
{
public:
	typedef std::string data_atom;
	inserter( action_str const & action_, sql_query_string const & mms_sql_query_ = ""): mms_sql_query( mms_sql_query_ ), action( action_ )
	{
	}
	/*!
	\brief Updates a data base located in db_path.
	\param sql_query_ The MMS SQL query.
	\param db_path The path to the data base to be updated.
	\throws std::runtime_error.
	*/
	virtual long long insert( sql_query_string const & sql_query_, std::string const & db_path );
	/*!
	\brief Updates a data base located in db_path.
	\throws std::runtime_error.
	*/
	virtual long long insert( std::string const & db_path );

protected:
	virtual bool validate_mms_sql_query();
	virtual void parse_query();
	virtual void after_parse_query()
	{
	}
	virtual void add_table();
	virtual void add_columns();
	virtual void add_values();
	// In a RDBMS it should have an implementation.
	virtual long long insert_into_db(sql_query_string const & sql_query, std::string const & db_path) = 0;
protected:
	sql_query_string sql_query, mms_sql_query;
	insert_clause_container insert_clause;
	action_str action;
private:
	/*!
	\brief Updates a data base located in db_path.
	\throws std::runtime_error.
	*/
	long long insert( insert_clause_container const & select_clause, std::string const & db_path );
	void begin_query();
	int insert_compiler_impl();
};

class sqlite3_inserter: public inserter
{
public:
	sqlite3_inserter( action_str const & action, sql_query_string const & sql_query_ = "" ): inserter( action, sql_query_ )
	{
	}
protected:
	long long insert_into_db(sql_query_string const & sql_query, std::string const & db_path) override
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
			db_.insert( sql_query );
		}
		catch ( std::runtime_error & )
		{
			return INSERT_TO_DB;
		}
		return db_.last_inserted_row_id();		
	}
};

} // namespace mms

#endif