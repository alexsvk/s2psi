#ifndef mms_creator_h
#define mms_creator_h

#include "sql_gen_helper.h" // USES
#include "db.h" // USES

namespace mms
{

class creator
{
public:
	typedef std::string data_atom;
	creator( action_str const & action_, sql_query_string const & mms_sql_query_ = ""): mms_sql_query( mms_sql_query_ ), action( action_ )
	{
	}
	/*!
	\brief Creates a table in a data base located in db_path.
	\param sql_query_ The MMS SQL query.
	\param db_path The path to the data base the table to be created in.
	\throws std::runtime_error.
	*/
	virtual void create_table( sql_query_string const & sql_query_, std::string const & db_path );
	virtual ~creator()
	{
	}
	/*!
	\brief Creates a table in a data base located in db_path.
	\throws std::runtime_error.
	*/
	virtual void create_table( std::string const & db_path );

protected:
	virtual bool validate_mms_sql_query();
	virtual void parse_query();
	virtual void after_parse_query()
	{
	}
	virtual void add_table();
	virtual void add_columns();
	virtual void add_type(unsigned const & idx);
	virtual sql_query_string add_constraints(unsigned const & idx);
	// In a RDBMS it should have an implementation.
	virtual int create_table_in_db(sql_query_string const & sql_query, std::string const & db_path) = 0;
protected:
	sql_query_string sql_query, mms_sql_query;
	create_clause_container create_clause;
	action_str action;
private:
	/*!
	\brief Creates a table in a data base located in db_path.
	\throws std::runtime_error.
	*/
	void create_table( create_clause_container const & create_clause, std::string const & db_path );
	void begin_query();
	int create_compiler_impl();
};

class sqlite3_creator: public creator
{
public:
	sqlite3_creator( action_str const & action, sql_query_string const & sql_query_ = "" ): creator( action, sql_query_ )
	{
	}
protected:
	int create_table_in_db(sql_query_string const & sql_query, std::string const & db_path) override
	{
		
		::mms::db db_;
		try
		{
			db_.open( db_path );
		}
		catch ( std::runtime_error const & )
		{
			return OPEN_DB;
		}
		try
		{
			db_.create_table( sql_query );
		}
		catch ( std::runtime_error const & )
		{
			return CREATE_TABLE;
		}
		return SUCCESS;		
	}
};

} // namespace mms

#endif