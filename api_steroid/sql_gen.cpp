#pragma comment(lib,"..\\Debug\\sqlite3")
// PROJECT
#include <boost/lexical_cast.hpp> // uses
// LOCAL
#include "err_codes.h" // uses
#include "mediator_and_friend.h" // uses
#include "mediator.h" // uses
#include "sql_gen_helper.h" // uses => select_impl, GENERATE_MMS_MEDIATOR, GENERATE_MMS_MEDIATOR_AND_FRIEND
#include "sqlite3_selector.h" // uses
#include "measurement_auto_creator.h" // uses

//#define DEBUG
#ifdef DEBUG
#include "sql_gen_test.h"
#else
#define BUILDING_THE_DLL
#include "sql_gen.h" // implements
#endif

extern "C" EXPORTED void deallocate_selected_data(selection_double_raw_ptr double_raw_p, unsigned const & size)
{
	mms::deallocate( double_raw_p, size );
}

//////////////////////////////////////////////////////////////////////////////////
// MEDIATOR_AND_FRIEND
// :
// VISIT_SUBJECT, PROJECT
// TODO
// UTF-8
GENERATE_MMS_MEDIATOR_AND_FRIEND(
									mediator_visit_subject_and_friend_project, 
									sqlite3_selector,
									"project@id", 
									"from\
									(\
										(\
											visit join project on \"visit@project\" = \"project@id\"\
										)\
										join\
										visit_subject on \"visit@id\" = \"visit_subject@visit_id\"\
									)\
									join\
									subject on \"visit_subject@subject_id\" = \"subject@id\"" 
								)
extern "C" EXPORTED selection_double_raw_ptr mediator_visit_subject_and_friend_project(sql_spec_ptr const q_ptr, unsigned const & len, char const* db_path)
{	
	if ( q_ptr == nullptr || db_path == nullptr || len == 0 ) return reinterpret_cast<selection_double_raw_ptr>( MALFORMED_PARAMS );
	return mms::select_impl<
								mms::mediator_visit_subject_and_friend_project, // selector type 
								selection_double_raw_ptr // returned value type
						   >
						   ( 
								std::string( q_ptr, q_ptr + len ), // query
								std::string( db_path ) // DB path
						   );
}
//////////////////////////////////////////////////////////////////////////////////
// MEDIATOR
// :
// VISIT_SUBJECT
GENERATE_MMS_MEDIATOR( 
						mediator_visit_subject,
						sqlite3_selector,
						"visit@id",
						"from\
						(\
							visit\
							join\
							visit_subject on \"visit@id\" = \"visit_subject@visit_id\"\
						)\
						join\
						subject on \"visit_subject@subject_id\" = \"subject@id\""
					 )
extern "C" EXPORTED selection_double_raw_ptr mediator_visit_subject(sql_spec_ptr const q_ptr, unsigned const & len, char const* db_path)
{
	if ( q_ptr == nullptr || db_path == nullptr || len == 0 ) return reinterpret_cast<selection_double_raw_ptr>( MALFORMED_PARAMS );
	return mms::select_impl<
								mms::mediator_visit_subject, // selector type 
								selection_double_raw_ptr // returned value type
						   >
						   ( 
								std::string( q_ptr, q_ptr + len ), // query
								std::string( db_path ) // DB path
						   );
}
////////////////////////////////////////////////////////////////////////////////////
// MEDIATOR
// :
// PROJECT_MODULE
GENERATE_MMS_MEDIATOR( 
						mediator_project_module,
						sqlite3_selector,
						"project@id",
						"from\
						(\
							project\
							join\
							project_module on \"project@id\" = \"project_module@project_id\"\
						)\
						join\
						module on \"project_module@id\" = \"module@id\""
					 )
extern "C" EXPORTED selection_double_raw_ptr mediator_project_module(sql_spec_ptr const q_ptr, unsigned const & len, char const* db_path)
{
	if ( q_ptr == nullptr || db_path == nullptr || len == 0 ) return reinterpret_cast<selection_double_raw_ptr>( MALFORMED_PARAMS );
	return mms::select_impl<
								mms::mediator_project_module, // selector type 
								selection_double_raw_ptr // returned value type
							>
							( 
								std::string( q_ptr, q_ptr + len ), // query
								std::string( db_path ) // DB path
							);
}
//////////////////////////////////////////////////////////////////////////////////
// MEDIATOR
// :
// DEVICE_TEST_SET
GENERATE_MMS_MEDIATOR( 
						mediator_device_test_set,
						sqlite3_selector,
						"device@id",
						"from\
						(\
							device\
							join\
							device_test_set on \"device@id\" = \"device_test_set@device_id\"\
						)\
						join\
						test_set on \"device_test_set@test_set_id\" = \"test_set@id\""
					 )
extern "C" EXPORTED selection_double_raw_ptr mediator_device_test_set(sql_spec_ptr const q_ptr, unsigned const & len, char const* db_path)
{
	if ( q_ptr == nullptr || db_path == nullptr || len == 0 ) return reinterpret_cast<selection_double_raw_ptr>( MALFORMED_PARAMS );
	return mms::select_impl<
								mms::mediator_device_test_set, // selector type 
								selection_double_raw_ptr // returned value type
							>
							( 
								std::string( q_ptr, q_ptr + len ), // query
								std::string( db_path ) // DB path
							);
}

extern "C" EXPORTED int create_measurement(sql_spec_ptr module_name_raw_ptr, char const* db_path)
{
	if ( module_name_raw_ptr == nullptr || db_path == nullptr ) return MALFORMED_PARAMS;
	mms::sqlite3_measurement_auto_creator cr(module_name_raw_ptr, db_path);
	try
	{
		cr.do_();
	}
	catch ( int const & code )
	{
		return code;
	}	
	return SUCCESS;
}
// TODO
// UTF-8 code points
extern "C" EXPORTED selection_double_raw_ptr all_table_names(char const* db_path)
{
	if ( db_path == nullptr ) return reinterpret_cast<selection_double_raw_ptr>( MALFORMED_PARAMS );
	mms::sqlite3_selector selector("");
#ifdef SQLITE
	mms::sql_query_string const & sql_q = "select tbl_name from sqlite_master where type = 'table';";
#endif
	auto err_code = selector.select_from_db( sql_q, db_path );
	if ( err_code != SUCCESS ) return reinterpret_cast<selection_double_raw_ptr>(err_code);
	else return mms::convert_data_container_to_selection_raw_pointer<selection_double_raw_ptr>(selector.get_data());
}
// TODO
// UTF-8 code points
extern "C" EXPORTED selection_double_raw_ptr table_column_names(char const *table_name, char const *db_path)
{
	if ( table_name == nullptr && db_path == nullptr ) return reinterpret_cast<selection_double_raw_ptr>( MALFORMED_PARAMS );
	mms::sqlite3_selector selector("");
#ifdef SQLITE
	mms::sql_query_string const & sql_q = "pragma table_info('" + mms::sql_query_string(table_name) + "');";
#endif
	auto err_code = selector.select_from_db( sql_q, db_path );
	if ( err_code != SUCCESS ) return reinterpret_cast<selection_double_raw_ptr>(err_code);
	auto data_raw_ptr = mms::convert_data_container_to_selection_raw_pointer<selection_double_raw_ptr>(selector.get_data());
	if ( reinterpret_cast<int>(data_raw_ptr) < 0 ) return data_raw_ptr;
	try
	{
		auto const & columns_count = ::boost::lexical_cast<unsigned long>(data_raw_ptr[0]);
		auto const data_raw_ptr_ = data_raw_ptr;
		++data_raw_ptr;
		mms::selector::data_deque_container column_names;
		for ( auto idx = 0U; idx < columns_count; idx += 6, data_raw_ptr += 6 ) column_names.push_back( *(data_raw_ptr + 1) );
		deallocate_selected_data( data_raw_ptr_, columns_count + 1 );
		return mms::convert_data_container_to_selection_raw_pointer<selection_double_raw_ptr>( column_names );
	}
	catch ( ::boost::bad_lexical_cast const & )
	{
		return reinterpret_cast<selection_double_raw_ptr>(BOOST_LEXICAL_CAST);
	}
}
// TODO
// UTF-8 code points
extern "C" EXPORTED selection_double_raw_ptr table_column_names_and_types(char const *table_name, char const *db_path)
{
	if ( table_name == nullptr && db_path == nullptr ) return reinterpret_cast<selection_double_raw_ptr>( MALFORMED_PARAMS );
	mms::sqlite3_selector selector("");
#ifdef SQLITE
	mms::sql_query_string const & sql_q = "pragma table_info('" + mms::sql_query_string(table_name) + "');";
#endif
	auto err_code = selector.select_from_db( sql_q, db_path );
	if ( err_code != SUCCESS ) return reinterpret_cast<selection_double_raw_ptr>(err_code);
	auto data_raw_ptr = mms::convert_data_container_to_selection_raw_pointer<selection_double_raw_ptr>(selector.get_data());
	if ( reinterpret_cast<int>(data_raw_ptr) < 0 ) return data_raw_ptr;
	try
	{
		auto const & columns_count = ::boost::lexical_cast<unsigned long>(data_raw_ptr[0]);
		auto const data_raw_ptr_ = data_raw_ptr;
		++data_raw_ptr;
		mms::selector::data_deque_container column_names_and_types;
		for ( auto idx = 0U; idx < columns_count; idx += 6, data_raw_ptr += 6 ) column_names_and_types.push_back( *(data_raw_ptr + 1) ), column_names_and_types.push_back( *(data_raw_ptr + 2) );
		deallocate_selected_data( data_raw_ptr_, columns_count + 1 );
		return mms::convert_data_container_to_selection_raw_pointer<selection_double_raw_ptr>( column_names_and_types );
	}
	catch ( ::boost::bad_lexical_cast const & )
	{
		return reinterpret_cast<selection_double_raw_ptr>(BOOST_LEXICAL_CAST);
	}
}
//#define DEBUG
#ifdef DEBUG
int main()
{
	//char* path = "data_base.db", * sql_q_raw_ptr= 
	//	"[\"action\" : \"mediator_and_friend\",\
	//	\"distinct\" : false,\
	//	\"columns\" : [\"project_id\", \"project_name\"],\
	//	\"aggregates\" : {\"project_id\" : 1},\
	//	\"filter\" : {\"project_id\" : {\"interval\":{2:7,5:8,\"logic\" : 1}}},\
	//	\"groups\" : [\"project_name\"]]";
	//mediator_visit_subject_and_friend_project( sql_q_raw_ptr, std::strlen(sql_q_raw_ptr), path );
	//char* path = "data_base.db", *q = 
	//	"[\
	//	\"action\" : \"mediator_and_friend\",\
	//	\"distinct\" : false,\
	//	\"columns\" : [\"subject@first_name\"],\
	//	\"filter\" : {\"project@id\" : { \"IN\" : [1]}}\
	//	]";
	////mediator_visit_subject_and_friend_project( q, std::strlen(q), path );
	//table_column_names_and_types("country", path);
	create_measurement("miha","data_base.db");
}
#endif