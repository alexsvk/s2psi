/*!
\brief MMS compilers tester.
\author Alexander Syvak
\date July 15 2013
*/
//#define TEST_ON
#ifdef TEST_ON

#pragma comment(lib,"..\\Debug\\sqlite3")
// SYSTEM
#include <stdexcept> // runtime_error
#include <sstream> // ostringstream
#include <algorithm> // mismatch, equal

// PROJECT
#define BOOST_TEST_MODULE mms_cpp_api
//#define BOOST_TEST_NO_MAIN
#if defined(_WIN32) || defined(_WIN64) || defined(__WINDOWS__) || defined(__WIN32__) || defined(__TOS_WIN__)
	#include <boost/test/unit_test.hpp> // Support of the auto-linking feature for MSVS 
										// http://www.boost.org/doc/libs/1_54_0/libs/test/doc/html/utf/compilation/auto-linking.html
#elif defined(__gnu_linux__)
	#include <boost/test/included/unit_test.hpp>
#endif
#include <boost/lexical_cast.hpp>
#include <boost/assign/list_of.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/spirit/include/qi.hpp>
// LOCAL
#include "sql_gen.h" // uses: functions to test
#include "db.h" // uses
#include "err_codes.h" // uses

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
// COMPILERS
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
// TYPES
typedef std::string::const_iterator it;
typedef ::boost::spirit::qi::expectation_failure<it> boost_qi_exception;
typedef ::boost::spirit::qi::space_type skipper;
typedef std::string mms_sql_query;

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
// FIXTURES
struct api_fxt
{
	api_fxt() : rcv(nullptr), db_path("t1.db")
	{ 
		reset();
		BOOST_TEST_MESSAGE( "Setup API fixture" ); 
	}
	~api_fxt()
	{ 
		BOOST_TEST_MESSAGE( "Teardown API fixture" ); 
		reset();
	}	
	void reset()
	{
		exp.resize(0);
		if ( 
				rcv != nullptr && 
				!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER)
		   ) 
		   deallocate_selected_data( rcv, ::boost::lexical_cast<unsigned>(rcv[0]) + 1 );
	}
	char const *db_path;
	mms_sql_query q;
	typedef std::vector<std::string> strings_container;
	strings_container exp;
	selection_double_raw_ptr rcv;
	int rcv_int;
};
struct create_clause_fxt : public api_fxt
{
	create_clause_fxt()
	{
		BOOST_TEST_MESSAGE( "Setup create_clause_fxt fixture." ); 
	}
	~create_clause_fxt()	
	{ 
		BOOST_TEST_MESSAGE( "Teardown create_clause_fxt fixture." ); 
	}
};
struct insert_clause_fxt : public api_fxt
{
	insert_clause_fxt()
	{
		BOOST_TEST_MESSAGE( "Setup insert_clause_fxt fixture." ); 
	}
	~insert_clause_fxt()	
	{ 
		BOOST_TEST_MESSAGE( "Teardown insert_clause_fxt fixture." ); 
	}
};
struct select_clause_fxt : public api_fxt
{
	select_clause_fxt()
	{
		BOOST_TEST_MESSAGE( "Setup select_clause_fxt fixture." ); 
	}
	~select_clause_fxt()	
	{ 
		BOOST_TEST_MESSAGE( "Teardown select_clause_fxt fixture." ); 
	}
};
struct update_clause_fxt : public api_fxt
{
	update_clause_fxt()
	{
		BOOST_TEST_MESSAGE( "Setup update_clause_fxt fixture." ); 
	}
	~update_clause_fxt()	
	{ 
		BOOST_TEST_MESSAGE( "Teardown update_clause_fxt fixture." ); 
	}
};
struct delete_clause_fxt : public api_fxt
{
	delete_clause_fxt()
	{
		BOOST_TEST_MESSAGE( "Setup delete_clause_fxt fixture." ); 
	}
	~delete_clause_fxt()	
	{ 
		BOOST_TEST_MESSAGE( "Teardown delete_clause_fxt fixture." ); 
	}
};
//
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
//// select_clause !!! TEST !!!
///////////////////////////////////////////////////////////////////////////////////
//BOOST_FIXTURE_TEST_SUITE( select_clause, select_clause_fxt )
//BOOST_AUTO_TEST_CASE(basic)
//{
//	q = "[\
//			\"action\" : \"select\",\
//			\"distinct\" : false,\
//			\"columns\" : [\"project@id\", \"project@name\"],\
//			\"filter\" : {\"project@name\":{\"like\":\"a\"}}\
//		]";
//	rcv = select_( q.data(), q.size(), db_path );
//}
//BOOST_AUTO_TEST_SUITE_END()
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
//// create_clause !!! TEST !!!
///////////////////////////////////////////////////////////////////////////////////
//BOOST_FIXTURE_TEST_SUITE( create_clause, create_clause_fxt )
//BOOST_AUTO_TEST_CASE(basic)
//{
//	q = "[\
//			\"action\" : \"create\",\
//			\"columns\" : [\
//							\"basic_test@int\",\
//							\"basic_test@varchar\",\
//							\"basic_test@double\",\
//							\"basic_test@bool\"\
//						  ],\
//			\"types\" : [\
//							0,\
//							3,\
//							1,\
//							2\
//						]\,\
//			\"constraints\" : [\
//									{\
//										\"primary\" : true\
//									},\
//									{},\
//									{\
//										\"default\" : 1.0\
//									}\
//							  ],\
//			\"constraints_sdata\" : [\
//										{\
//											\"basic_test@varchar\" :\
//																	[\
//																		256\
//																	]\
//										}\
//									]\
//		]";
//	rcv_int = create_( q.data(), q.size(), db_path );
//}
//BOOST_AUTO_TEST_SUITE_END()
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//// insert_clause !!! TEST !!!
/////////////////////////////////////////////////////////////////////////////////////
BOOST_FIXTURE_TEST_SUITE( insert_clause, insert_clause_fxt )
BOOST_AUTO_TEST_CASE(basic)
{
	//q = "[\
	//		\"action\" : \"insert\",\
	//		\"columns\" : [\"basic_test_varchar\", \"basic_test_bool\"],\
	//		\"values\" : [\"CZK\",true]\
	//	]";
	q = "[\
			\"action\" : \"insert\",\
			\"columns\" : [\"project@id\", \"project@name\", \"project@notes\"],\
			\"values\" : [1, \"tretji\", \"drugi notes\"],\
			\"default_all\" : false\
		]";
	rcv_int = insert_( q.data(), q.size(), db_path );
	q = "[\
			\"action\" : \"insert\",\
			\"columns\" : [\"project@id\", \"project@name\", \"project@notes\"],\
			\"values\" : [1, \"tretji\", \"drugi notes\"],\
			\"default_all\" : false\
		]";
	rcv_int = insert_( q.data(), q.size(), db_path );
}
BOOST_AUTO_TEST_SUITE_END()
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//// update_clause !!! TEST !!!
/////////////////////////////////////////////////////////////////////////////////////
//BOOST_FIXTURE_TEST_SUITE( update_clause, update_clause_fxt )
//BOOST_AUTO_TEST_CASE(basic)
//{
//	//q = "[\
//	//		\"action\" : \"update\",\
//	//		\"columns\" : [\"basic_test_varchar\", \"basic_test_bool\"],\
//	//		\"values\" : [\"EUR\", false],\
//	//		\"filter\" : {\
//	//						\"basic_test_varchar\" :\
//	//						{\
//	//							\"in\" : [\"CZK\"]\
//	//						}\
//	//					 }\
//	//	]";
//	q = "[\"action\" : \"update\",\"columns\" : [\"project@name\"],\"values\" : [\"aaa\"],\"filter\" : {\"project@id\" :{\"in\" : [1]}}]";
//	rcv_int = update( q.data(), q.size(), db_path );
//}
//BOOST_AUTO_TEST_SUITE_END()
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
// delete_clause !!! TEST !!!
///////////////////////////////////////////////////////////////////////////////////
//BOOST_FIXTURE_TEST_SUITE( delete_clause, delete_clause_fxt )
//BOOST_AUTO_TEST_CASE(basic)
//{
//	//q = "[\
//	//		\"action\" : \"delete\",\
//	//		\"filter\" : {\
//	//						\"basic_test_bool\" :\
//	//						{\
//	//							\"in\" : [false]\
//	//						}\
//	//					 }\
//	//	]";
//	q = "[\
//			\"action\" : \"delete\",\
//			\"filter\" : {\
//							\"project@id\" :\
//							{\
//								\"in\" : [3]\
//							}\
//						 }\
//		]";
//	rcv_int = delete_( q.data(), q.size(), db_path );
//}
//BOOST_AUTO_TEST_SUITE_END()
//BOOST_AUTO_TEST_SUITE(select_suite)
//BOOST_AUTO_TEST_CASE(s)
//{
//	std::string q = 
//		"[\
//			\"action\" : \"select\",\
//			\"distinct\" : false,\
//			\"columns\" : [\"project@id\", \"project@name\"]\
//		]";
//	auto rcv_int = select_( q.data(), q.size(), "t1.db" );
//}
//BOOST_AUTO_TEST_SUITE_END()
#endif