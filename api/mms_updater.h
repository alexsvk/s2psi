#ifndef mms_updater_h
#define mms_updater_h

#include "sql_gen_helper.h" // USES
#include "db.h" // USES

namespace mms
{

class updater
{
public:
	typedef std::string data_atom;
	updater( action_str const & action_, sql_query_string const & mms_sql_query_ = ""): mms_sql_query( mms_sql_query_ ), action( action_ )
	{
	}
	/*!
	\brief Updates a data base located in db_path.
	\param sql_query_ The MMS SQL query.
	\param db_path The path to the data base to be updated.
	\throws std::runtime_error.
	*/
	virtual void update( sql_query_string const & sql_query_, std::string const & db_path );
	/*!
	\brief Updates a data base located in db_path.
	\throws std::runtime_error.
	*/
	virtual void update( std::string const & db_path );

protected:
	virtual bool validate_mms_sql_query();
	virtual void parse_query();
	virtual void after_parse_query()
	{
	}
	virtual void add_table();
	virtual void add_columns_and_values();
	virtual void add_filters();
	// In a RDBMS it should have an implementation.
	virtual int update_db(sql_query_string const & sql_query, std::string const & db_path) = 0;
protected:
	sql_query_string sql_query, mms_sql_query;
	update_clause_container update_clause;
	action_str action;
private:
	/*!
	\brief Updates a data base located in db_path.
	\throws std::runtime_error.
	*/
	void update( update_clause_container const & select_clause, std::string const & db_path );
	void begin_query();
	int update_compiler_impl();
};

class sqlite3_updater: public updater
{
public:
	sqlite3_updater( action_str const & action, sql_query_string const & sql_query_ = "" ): updater( action, sql_query_ )
	{
	}
protected:
	int update_db(sql_query_string const & sql_query, std::string const & db_path) override
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
			db_.update( sql_query );
		}
		catch ( std::runtime_error & )
		{
			return UPDATE_DB_TABLE;
		}
		return SUCCESS;		
	}
};

} // namespace mms

#endif