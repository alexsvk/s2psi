#pragma comment(lib,"..\\Debug\\sqlite3")
// PROJECT
#include <boost/lexical_cast.hpp> // uses
// LOCAL
#include "err_codes.h" // uses
#include "sql_gen_helper.h" // select_impl, GENERATE_MMS_MEDIATOR, GENERATE_MMS_MEDIATOR_AND_FRIEND
#include "sqlite3_selector.h" //uses
#include "mms_inserter.h" //uses
#include "mms_updater.h" //uses
#include "mms_deleter.h" //uses
#include "mms_creator.h" //uses

#define BUILDING_THE_DLL
#include "sql_gen.h" // implements

#include "db.h"

extern "C" EXPORTED void deallocate_selected_data(selection_double_raw_ptr double_raw_p, unsigned const & size)
{
	mms::deallocate( double_raw_p, size );
}
#ifndef basic_selector_
#define basic_selector_
GENERATE_MMS_BASIC_SELECTOR(mms::sqlite3_selector)
#endif
extern "C" EXPORTED selection_double_raw_ptr select_(sql_spec_ptr const q_ptr, unsigned const & len, char const* db_path)
{
	if ( q_ptr == nullptr || db_path == nullptr || len == 0 ) return reinterpret_cast<selection_double_raw_ptr>( MALFORMED_PARAMS );
	return mms::select_impl<
								mms::basic_selector, // selector type 
								selection_double_raw_ptr // returned value type
						   >
						   ( 
								std::string( q_ptr, q_ptr + len ), // query
								std::string( db_path ) // DB path
						   );
}
#ifndef basic_inserter_
#define basic_inserter_
GENERATE_MMS_BASIC_INSERTER(mms::sqlite3_inserter)
#endif
extern "C" EXPORTED insert_return_val insert_(sql_spec_ptr const q_ptr, unsigned const & len, char const* db_path)
{
	if ( q_ptr == nullptr || db_path == nullptr || len == 0 ) return static_cast<insert_return_val>( MALFORMED_PARAMS );
	return mms::insert_impl<
								mms::basic_inserter, // selector type 
								insert_return_val // returned value type
						   >
						   ( 
								std::string( q_ptr, q_ptr + len ), // query
								std::string( db_path ) // DB path
						   );
}
#ifndef basic_creator_
#define basic_creator_
GENERATE_MMS_BASIC_CREATOR(mms::sqlite3_creator)
#endif
extern "C" EXPORTED create_return_val create_(sql_spec_ptr const q_ptr, unsigned const & len, char const* db_path)
{
	if ( q_ptr == nullptr || db_path == nullptr || len == 0 ) return static_cast<create_return_val>( MALFORMED_PARAMS );
	return mms::create_impl<
								mms::basic_creator, // selector type 
								create_return_val // returned value type
						   >
						   ( 
								std::string( q_ptr, q_ptr + len ), // query
								std::string( db_path ) // DB path
						   );
}
#ifndef basic_deleter_
#define basic_deleter_
GENERATE_MMS_BASIC_DELETER(mms::sqlite3_deleter)
#endif
extern "C" EXPORTED delete_return_val delete_(sql_spec_ptr const q_ptr, unsigned const & len, char const* db_path)
{
	if ( q_ptr == nullptr || db_path == nullptr || len == 0 ) return static_cast<delete_return_val>( MALFORMED_PARAMS );
	return mms::delete_impl<
								mms::basic_deleter, // selector type 
								delete_return_val // returned value type
						   >
						   ( 
								std::string( q_ptr, q_ptr + len ), // query
								std::string( db_path ) // DB path
						   );
}
#ifndef basic_updater_
#define basic_updater_
GENERATE_MMS_BASIC_UPDATER(mms::sqlite3_updater)
#endif
extern "C" EXPORTED update_return_val update(sql_spec_ptr const q_ptr, unsigned const & len, char const* db_path)
{
	if ( q_ptr == nullptr || db_path == nullptr || len == 0 ) return static_cast<update_return_val>( MALFORMED_PARAMS );
	return mms::update_impl<
								mms::basic_updater, // selector type 
								update_return_val // returned value type
						   >
						   ( 
								std::string( q_ptr, q_ptr + len ), // query
								std::string( db_path ) // DB path
						   );
}

extern "C" EXPORTED int drop_table(sql_spec_ptr table_name_raw_ptr, char const* db_path)
{
	mms::db db;
	std::string q = "drop table if exists ";
	q += table_name_raw_ptr;
	try
	{
		db.open( db_path );
	}
	catch ( std::runtime_error const & )
	{
		return OPEN_DB;
	}
	try
	{
		db.delete_( q );
	}
	catch ( std::runtime_error const & )
	{
		return DROP_TABLE;
	}
	return SUCCESS;		
}
//#define debug
#ifdef debug
int main()
{
	char* path = "../data_base.db", * sql_q_raw_ptr= 
		"[\"action\" : \"delete\",\
		\"filter\" : {\"subject@id\" : {\"in\":[1]}}]";
	sql_q_raw_ptr = "pragma foreign_keys = 'ON';";
	mms::db db_;
	db_.open(path);
	//db_.insert(sql_q_raw_ptr);
	db_.set_db_option(sql_q_raw_ptr);
	sql_q_raw_ptr = "pragma foreign_keys;";
	auto selector = mms::sqlite3_selector("");
	db_.select(sql_q_raw_ptr, selector.get_data_handle(), &selector.get_data());
	std::cout << selector.get_data().at(0) << '\n';
//	delete_(sql_q_raw_ptr, std::strlen(sql_q_raw_ptr), path);
}
#endif