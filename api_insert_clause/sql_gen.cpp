#pragma comment(lib,"..\\Debug\\sqlite3")
// PROJECT
//#include <boost/lexical_cast.hpp> // uses
// LOCAL
#include "../api/err_codes.h" // uses
#include "../api/sql_gen_helper.h" // select_impl, GENERATE_MMS_MEDIATOR, GENERATE_MMS_MEDIATOR_AND_FRIEND
#include "../api/mms_inserter.h" 
//#include "../api/sqlite3_creator.h" //uses
#define BUILDING_THE_DLL
#include "../api/sql_gen.h" // implements
#include "../api/db.h" // uses
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