#pragma comment(lib,"..\\Debug\\sqlite3")
// PROJECT
#include <boost/lexical_cast.hpp> // uses
// LOCAL
#include "../api/err_codes.h" // uses
#include "../api/sql_gen_helper.h" // select_impl, GENERATE_MMS_MEDIATOR, GENERATE_MMS_MEDIATOR_AND_FRIEND
#include "../api/mms_deleter.h" //uses

#define BUILDING_THE_DLL
#include "../api/sql_gen.h" // implements

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
