#pragma comment(lib,"..\\Debug\\sqlite3")
// PROJECT
//#include <boost/lexical_cast.hpp> // uses
// LOCAL
#include "../api/err_codes.h" // uses
#include "../api/sql_gen_helper.h" // select_impl, GENERATE_MMS_MEDIATOR, GENERATE_MMS_MEDIATOR_AND_FRIEND
#include "../api/sqlite3_selector.h" 
//#include "../api/sqlite3_creator.h" //uses
#define BUILDING_THE_DLL
#include "../api/sql_gen.h" // implements
#include "../api/db.h" // uses
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