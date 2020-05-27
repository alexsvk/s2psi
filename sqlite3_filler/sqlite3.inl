#include "mms_sqlite3_transaction.h"
#include "db.h"
#include <iostream>
#define DEBUG
namespace mms
{
	struct db::impl
	{
		impl(): db_ptr(nullptr)
		{
		}
		void open(source const & src_name);
		void insert(sql_q const & query);
		sqlite3_int64 last_inserted_row_id() const
		{
			return sqlite3_last_insert_rowid( db_ptr );
		}
		void select(sql_q const & query, data_reader_func const & data_reader, void *data_reader_arg = nullptr) const;
		void create_table(sql_q const & query);
		void delete_(sql_q const & query);
		void update(sql_q const & query);
		void close();
	private:
		void write_access_transaction( sql_q const & query );
		void throw_sqlite3_exception(std::string const & conseq) const
		{
			throw std::runtime_error( conseq + sqlite3_errmsg(db_ptr) );
		}
		sqlite3* db_ptr;
		static unsigned const conn_options = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |SQLITE_OPEN_FULLMUTEX;
		static unsigned const timeout = 20; // [ms]		
	};

	void db::impl::open(source const & src_name)
	{
		std::string const conseq = "Failed to open connection : ";
		if ( sqlite3_open_v2( 
					src_name.c_str( ), 
					&db_ptr, 
					conn_options, 
					NULL 
				    ) != SQLITE_OK 
		   ) 
		{
			db_ptr = nullptr;
			throw_sqlite3_exception(conseq);
		}
	}
	void db::impl::insert(sql_q const & query)
	{
		write_access_transaction( query );	
	}
	void db::impl::select(sql_q const & query, db::data_reader_func const & data_reader, void *data_reader_arg) const
	{
		std::string const conseq = "Failed to execute SQL select query";
		if ( !data_reader ) throw_sqlite3_exception(conseq);
		SetTimeout( timeout, db_ptr );
#if defined(__gnu_linux__)
		::scope_guard __attribute__((unused))timeout_guard = ::make_guard_1( &mms::ResetTimeout, db_ptr );
#elif defined(_WIN32) || defined(_WIN64) || defined(__WINDOWS__) || defined(__WIN32__) || defined(__TOS_WIN__)
		::scope_guard timeout_guard = ::make_guard_1( &mms::ResetTimeout, db_ptr );
		(timeout_guard);
#endif
		char* errmsg = nullptr;
		auto const & f_double_ptr = data_reader.target<data_reader_raw_func_ptr>();
		if ( f_double_ptr == nullptr ) throw std::runtime_error(conseq + " : Failed to extract callback function double pointer");
		auto const & f_ptr = *f_double_ptr;
		if ( f_ptr == nullptr ) throw std::runtime_error(conseq + " : Failed to extract callback function pointer");
		if ( sqlite3_exec( db_ptr, query.c_str(), f_ptr, data_reader_arg, &errmsg ) != SQLITE_OK ) 
		{
#ifdef DEBUG
			std::cout << query << '\n' << errmsg << '\n';
#endif
			auto conseq_ = errmsg != nullptr ? conseq + errmsg : conseq;
			if ( errmsg != nullptr ) sqlite3_free(errmsg);
			throw std::runtime_error(conseq_);
		}
	}
	void db::impl::create_table( sql_q const & query )
	{
		write_access_transaction( query );
	}
	void db::impl::delete_( sql_q const & query )
	{
		write_access_transaction( query );
	}
	void db::impl::update( sql_q const & query )
	{
		write_access_transaction( query );
	}
	void db::impl::close()
	{
		//sqlite3_close_v2(db_ptr);
		sqlite3_close(db_ptr);
	}
	void db::impl::write_access_transaction(sql_q const & query)
	{
		try
		{
			WriteTransactionSafeStart( db_ptr, timeout ); // timeout, save point, immediate transaction are used.
		}
		catch ( std::runtime_error & )
		{
			throw;
		}
		sqlite3_stmt* stmt_p = nullptr;
		WRITE_TRANSACTION_GUARDS( db_ptr, stmt_p )
		int errcode = SQLITE_OK;
		std::string const conseq = "Failed to execute SQL insert query";
start_insertion:
		do
		{
			// The nByte saves SQLite from having to make a copy of the input string.
			if ( sqlite3_prepare_v2( db_ptr, query.c_str( ), query.length( ) + 1, &stmt_p, NULL ) != SQLITE_OK )
				throw_sqlite3_exception( conseq );
		} while ( errcode == SQLITE_BUSY );
		do
		{
			if ( errcode == SQLITE_BUSY ) sqlite3_reset( stmt_p );
			errcode = sqlite3_step( stmt_p );
			if ( errcode == SQLITE_SCHEMA )	// statement-fatal error.
			{
				sqlite3_reset( stmt_p );
				sqlite3_finalize( stmt_p );	// SQLITE_OK is returned if recent statement evaluation encountered no errors. Hence, no error checking is present.
				goto start_insertion;
			}
			if ( errcode != SQLITE_DONE && errcode != SQLITE_BUSY ) throw_sqlite3_exception( conseq );
		} while ( errcode == SQLITE_BUSY );

		if ( ( errcode = sqlite3_reset( stmt_p ) ) != SQLITE_OK ) throw_sqlite3_exception( conseq );
		DISMISS_STMT_RESET_GUARD

		if ( ( errcode = sqlite3_finalize( stmt_p ) ) != SQLITE_OK ) throw_sqlite3_exception( conseq );
		DISMISS_STMT_FIN_GUARD

		stmt_p = nullptr;

		try
		{
			WriteTransactionSafeEnd( db_ptr ); // commits the transaction.
		}
		catch ( std::runtime_error & )
		{
			throw;
		}	
		DISMISS_TRANSACT_GUARD		
	}
}
