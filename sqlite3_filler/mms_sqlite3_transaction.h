/*
 * write_transaction.h
 *
 *  Created on: July 12, 2013
 *      Author: Alexander Syvak
 *
 * Files defines functions to simplify work when avoding SQLITE_BUSY message and deadlocks.
 * To achieve that immediate transactions are used to avoid deadlock,
 * timeout - to avoid SQLITE_BUSY,
 * save points and rollbacks to bring reliability when getting unpredictable results
 * ( e.g. getting SQLITE_SCHEMA when stepping in query using v2 of preparation function ),
 * scope guards to pass responsibility onto PC to destroy object in a user defined way
 * ( or to not forget to call any function when next command is out of a current scope )
 * and to shorter amount of code lines,
 * macros to throw std::runtime_exception with defined consequence and retrieved reason from
 * entrails of SQLite3.
 */

#ifndef WRITE_TRANSACTION_H_
#define WRITE_TRANSACTION_H_

#include "scope_guard.h"	// USES
#include "sqlite3.h"	// USES
#include <stdexcept>
// Macros throws std::runtime exception, which consists of consequence and reason as result of sqlite3_errmsg function.
#define WRITE_TRANSACTION_THROW(rConseq, rDbConnection) throw std::runtime_error( rConseq + sqlite3_errmsg( rDbConnection ) );
#define READ_TRANSACTION_THROW(rConseq, rDbConnection) throw std::runtime_error( rConseq + sqlite3_errmsg( rDbConnection ) );
// Macros throws std::runtime_exception only if and only if errcode variable does not equal rOkMsg and does not equal SQLITE_BUSY.
// WARNING
// errcode integer variable must be predeclared before using this macros.
#define WRITE_TRANSACTION_ON_ERROR( rOkMsg, rConseq, rDbConnection )\
	if ( errcode != rOkMsg && errcode != SQLITE_BUSY )\
	{\
		char const * errmsg = sqlite3_errmsg( rDbConnection );\
		std::string msg = rConseq;\
		if ( errmsg != NULL ) msg += errmsg;\
		throw std::runtime_error( msg );\
	}

namespace mms
{
	std::string const sql_begin_imtransaction = "BEGIN IMMEDIATE TRANSACTION";	// No other database connection will be able
																				// to write to the database or execute BEGIN IMMEDIATE
																				// or BEGIN EXCLUSIVE. Other processes can continue to
																				// read from the database, however.
	std::string const sql_commit = "COMMIT";

	// Sets timeout to avoid SQLITE_BUSY messages.
	// @param rMsg Count of milliseconds.
	// @param rDbConnection Pointer to SQLite3 DB connection.
	// @return void.
	// @exception noexcept.
	static void SetTimeout( int const & rMs, sqlite3* rDbConnection )
	{
		sqlite3_busy_timeout( rDbConnection, rMs );
	}

	// Resets timeout set via SetTimeout() functions.
	// @param void.
	// @return void.
	// @exception noexcept.
	static void ResetTimeout( sqlite3* rDbConnection )
	{
		sqlite3_busy_timeout( rDbConnection, 0 );
	}

	// Begins immediate transaction.
	// @param rDbConnection Pointer to SQLite3 DB connection.
	// @return void.
	// @exception std::runtime_error.
	static void BeginImmediateTransaction(sqlite3* rDbConnection ) 
	{
		std::string const conseq = "Failed to begin immediate transaction : ";
		int errcode = SQLITE_OK;
		sqlite3_stmt* stmt = NULL;
		///////////////// Automatic cleanup.///////////////////////////////////////////
		scope_guard stmt_fin_guard = make_guard_1( &sqlite3_finalize, stmt );
		scope_guard stmt_reset_guard = make_guard_1( &sqlite3_reset, stmt );
		///////////////////////////////////////////////////////////////////////////////
	start_begin_immtransaction_again:
		do
		{
			errcode = sqlite3_prepare_v2( rDbConnection, sql_begin_imtransaction.c_str( ), sql_begin_imtransaction.length( ) + 1, &stmt, NULL );
            WRITE_TRANSACTION_ON_ERROR( SQLITE_OK, conseq, rDbConnection )
		} while ( errcode == SQLITE_BUSY );
		do
		{
			if ( errcode == SQLITE_BUSY ) sqlite3_reset( stmt );
			errcode = sqlite3_step( stmt );
			if ( errcode == SQLITE_SCHEMA )	// statement-fatal error.
			{
				sqlite3_reset( stmt );
				sqlite3_finalize( stmt );	// SQLITE_OK is returned if recent statement evaluation encountered no errors. Hence, no error-check.
				goto start_begin_immtransaction_again;
			}
            WRITE_TRANSACTION_ON_ERROR( SQLITE_DONE, conseq, rDbConnection )
		} while ( errcode == SQLITE_BUSY );
        if ( ( errcode = sqlite3_reset( stmt ) ) != SQLITE_OK )	WRITE_TRANSACTION_THROW(conseq, rDbConnection) // release locks.
		stmt_reset_guard.dismiss( );	// statement is reset.
        if ( ( errcode = sqlite3_finalize( stmt ) ) != SQLITE_OK ) WRITE_TRANSACTION_THROW(conseq, rDbConnection) // release memory.
		stmt_fin_guard.dismiss( );	// statement is finalized.
	}

	// Commits transaction.
	// @param rDbConnection Pointer to SQLite3 DB connection.
	// @return void.
	// @exception std::runtime_error.
	void Commit( sqlite3* rDbConnection ) 
	{
		std::string const conseq = "Failed to commit transaction : ";
		sqlite3_stmt* stmt = NULL;
		///////////////// Automatic cleanup.///////////////////////////////////////////
		scope_guard stmt_fin_guard = make_guard_1( &sqlite3_finalize, stmt );
		scope_guard stmt_reset_guard = make_guard_1( &sqlite3_reset, stmt );
		///////////////////////////////////////////////////////////////////////////////
		int errcode = SQLITE_OK;
	commit_again:
		do
		{
			errcode = sqlite3_prepare_v2( rDbConnection, sql_commit.c_str( ), sql_commit.length( ) + 1, &stmt, NULL );
            WRITE_TRANSACTION_ON_ERROR( SQLITE_OK, conseq, rDbConnection )
		} while ( errcode == SQLITE_BUSY );
		do
		{
			if ( errcode == SQLITE_BUSY ) sqlite3_reset( stmt );
			errcode = sqlite3_step( stmt );
			if ( errcode == SQLITE_SCHEMA )	// statement-fatal error.
			{
				sqlite3_reset( stmt );
				sqlite3_finalize( stmt );	// SQLITE_OK is returned if recent statement evaluation encountered no errors. Hence, no error-check.
				goto commit_again;
			}
            WRITE_TRANSACTION_ON_ERROR( SQLITE_DONE, conseq, rDbConnection )
		} while ( errcode == SQLITE_BUSY );
        if ( ( errcode = sqlite3_reset( stmt ) ) != SQLITE_OK )	WRITE_TRANSACTION_THROW(conseq, rDbConnection) // release locks.
		stmt_reset_guard.dismiss( );	// statement is reset.
        if ( ( errcode = sqlite3_finalize( stmt ) ) != SQLITE_OK ) WRITE_TRANSACTION_THROW(conseq, rDbConnection) // release memory.
		stmt_fin_guard.dismiss( );	// statement is finalized.
	}

	// Puts save point.
	// @param rDbConnection Pointer to SQLite3 DB connection.
	// @return void.
	// @exception std::runtime_error.
	// WARNING
	// No reason of an exception is set, as sqlite3_exec is a wrapper for a set of internal SQLite3 functions.
	static void PutSavePoint( sqlite3* rDbConnection ) 
	{
		if ( sqlite3_exec( rDbConnection, "SAVEPOINT point;", NULL, NULL, NULL ) != SQLITE_OK ) throw std::runtime_error( "Failed to set a savepoint" );
	}

	// Rollbacks to the point saved in the PutSavePoint method.
	// @param rDbConnection Pointer to SQLite3 DB connection.
	// @return void.
	// @exception std::runtime_error.
	// WARNING
	// No reason of an exception is set, as sqlite3_exec is a wrapper for a set of internal SQLite3 functions.
	static void Rollback( sqlite3* rDbConnection ) 
	{
		if ( sqlite3_exec( rDbConnection, "ROLLBACK TRANSACTION TO SAVEPOINT point;", NULL, NULL, NULL ) != SQLITE_OK ) throw std::runtime_error( "Failed to rollback" );
	}

	// Declares and initializes sqlite3_stmt as stmt set to NULL.
	// Should be used on WRITE operations to DB before query execution and value binding stage.
	// @param rDbConnection Connection to SQLite3 DB.
	// @param rTimeout Timeout.
	// @exception std::runtime_error.
	// @return void.
	inline void WriteTransactionSafeStart(sqlite3* rDbConnection, int const & rTimeout) 
	{
		SetTimeout( rTimeout, rDbConnection );\
		\
		try\
		{\
            mms::BeginImmediateTransaction( rDbConnection );\
            mms::PutSavePoint( rDbConnection );\
		}\
		catch ( std::runtime_error & )\
		{\
			throw;\
		}\
	}

	// Resets and finalizes statement stmt.
	// Commits immediate transaction.
	// @param rDbConnection Connection to SQLite3 DB.
	// @param rConseq Consequence of an exception.
	// @exception std::runtime_error.
	// @return void.
	inline void WriteTransactionSafeEnd(sqlite3* rDbConnection) 
	{
		try\
		{\
            mms::Commit( rDbConnection );\
		}\
		catch ( std::runtime_error & )\
		{\
			throw;\
		}\

	}

	// Sets timeout.
	// @param rDbConnection Connection to SQLite3 DB.
	// @param rTimeout Timeout.
	// @exception noexcept.
	// @return void.
	inline void ReadTransactionSafeStart( sqlite3* rDbConnection, int const & rTimeout )
	{
		SetTimeout( rTimeout, rDbConnection );
	}

}	// namespace mms

// Sets
//timeout;
//immediate transaction;
//guards:
//resettimeout;
//rollback;
//sqlite3_finalize;
//sqlite3_reset.
//Declares and initializes sqlite3_stmt as stmt set to NULL.
//Should be used on WRITE operations to DB before query execution and value binding stage.
//@param rDbConnection Connection to SQLite3 DB.
//@exception std::runtime_error.
#define WRITE_TRANSACTION_PREPARE_POSTCONDITIONS( rDbConnection ) \
	SetTimeout( timeout, rDbConnection );\
    scope_guard timeout_guard = make_guard_1( &mms::ResetTimeout, rDbConnection );\
	try\
	{\
        mms::BeginImmediateTransaction( rDbConnection );\
        mms::PutSavePoint( rDbConnection );\
	}\
	catch ( std::runtime_error & )\
	{\
		throw;\
	}\
    scope_guard transaction_guard = make_guard_1( &mms::Rollback, rDbConnection );\
	sqlite3_stmt* stmt = NULL;\
	scope_guard stmt_fin_guard = make_guard_1( &sqlite3_finalize, stmt );\
	scope_guard stmt_reset_guard = make_guard_1( &sqlite3_reset, stmt );

// Dismisses
// rollback;
// sqlite3_reset;
// sqlite3_finalize;
// guards.
// Resets and finalizes statement stmt.
// Commits immediate transaction.
// Should be used on WRITE operations to DB after query execution cycle.
#define WRITE_TRANSACTION_CLEANUP_POSTCONDITIONS( rDbConnection, rConseq )\
	if ( ( errcode = sqlite3_reset( stmt ) ) != SQLITE_OK )\
	{\
        WRITE_TRANSACTION_THROW(rConseq, rDbConnection)\
	}\
	stmt_reset_guard.dismiss( );\
	if ( ( errcode = sqlite3_finalize( stmt ) ) != SQLITE_OK )\
	{\
        WRITE_TRANSACTION_THROW(rConseq, rDbConnection)\
	}\
	stmt_fin_guard.dismiss( );\
	stmt = NULL;\
	try\
	{\
        mms::Commit( rDbConnection );\
	}\
	catch ( std::runtime_error & )\
	{\
		throw;\
	}\
	transaction_guard.dismiss( );

#define DISMISS_STMT_RESET_GUARD stmt_reset_guard.dismiss( );

#define DISMISS_STMT_FIN_GUARD stmt_fin_guard.dismiss( );

#define DISMISS_TRANSACT_GUARD transaction_guard.dismiss( );

#define DISMISS_STMT_TRANSACT_GUARDS\
		DISMISS_STMT_RESET_GUARD\
		DISMISS_STMT_FIN_GUARD\
		DISMISS_TRANSACT_GUARD

// Places timeout, sqlite3_reset, sqlite3_finalize, Rollback scope guards.
// @param rDbConnection SQLite3 DB connection pointer.
// @param rStmt Statement.
#if defined(_WIN32) || defined(_WIN64) || defined(__WINDOWS__) || defined(__WIN32__) || defined(__TOS_WIN__)
	#define WRITE_TRANSACTION_GUARDS( rDbConnection, rStmt )\
	    scope_guard timeout_guard = make_guard_1( &mms::ResetTimeout, rDbConnection );\
	    (timeout_guard);\
	    scope_guard transaction_guard = make_guard_1( &mms::Rollback, rDbConnection );\
		scope_guard stmt_fin_guard = make_guard_1( &sqlite3_finalize, rStmt );\
		scope_guard stmt_reset_guard = make_guard_1( &sqlite3_reset, rStmt );
#elif defined(__gnu_linux__)
	#define WRITE_TRANSACTION_GUARDS( rDbConnection, rStmt )\
	    scope_guard __attribute__((unused))timeout_guard = make_guard_1( &mms::ResetTimeout, rDbConnection );\
	    scope_guard transaction_guard = make_guard_1( &mms::Rollback, rDbConnection );\
		scope_guard stmt_fin_guard = make_guard_1( &sqlite3_finalize, rStmt );\
		scope_guard stmt_reset_guard = make_guard_1( &sqlite3_reset, rStmt );
#endif
// Places timeout, sqlite3_reset, sqlite3_finalize scope guards.
// @param rDbConnection SQLite3 DB connection pointer.
// @param rStmt Statement.
#if defined(_WIN32) || defined(_WIN64) || defined(__WINDOWS__) || defined(__WIN32__) || defined(__TOS_WIN__)
	#define READ_TRANSACTION_GUARDS( rDbConnection, rStmt)\
	    	scope_guard timeout_guard = make_guard_1( &mms::ResetTimeout, rDbConnection );\
		(timeout_guard);\
		scope_guard stmt_fin_guard = make_guard_1( &sqlite3_finalize, rStmt );\
		scope_guard stmt_reset_guard = make_guard_1( &sqlite3_reset, rStmt );
#elif defined(__gnu_linux__)
	#define READ_TRANSACTION_GUARDS( rDbConnection, rStmt)\
	    scope_guard __attribute__((unused))timeout_guard = make_guard_1( &mms::ResetTimeout, rDbConnection );\
		scope_guard stmt_fin_guard = make_guard_1( &sqlite3_finalize, rStmt );\
		scope_guard stmt_reset_guard = make_guard_1( &sqlite3_reset, rStmt );
#endif
// Finishes statement connected work and handles scope guards.
// @param rDbConnection SQLite3 DB connection pointer.
// @param rStmt Statement.
// @param rConseq Consequence in an exception.
#define READ_TRANSACTION_SAFE_END( rDbConnection, rStmt, rConseq )\
    if ( ( errcode = sqlite3_reset( rStmt ) ) != SQLITE_OK ) READ_TRANSACTION_THROW(rConseq, rDbConnection)\
	DISMISS_STMT_RESET_GUARD\
    if ( ( errcode = sqlite3_finalize( rStmt ) ) != SQLITE_OK ) READ_TRANSACTION_THROW(rConseq, rDbConnection)\
	DISMISS_STMT_FIN_GUARD

#endif /* WRITE_TRANSACTION_H_ */
