/*!
\brief MMS compilers tester.
\author Alexander Syvak
\date July 15 2013
*/
#define TEST_ON
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
#include "../api/sql_gen.h" // uses: functions to test
#include "../api/db.h" // uses
#include "../api/err_codes.h" // uses

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
	api_fxt() : rcv(nullptr), db_path("../api_create_clause/data_base.db")
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
	}
	char const *db_path;
	mms_sql_query q;
	typedef std::vector<std::string> strings_container;
	strings_container exp;
	selection_double_raw_ptr rcv;
	long long rcv_int;
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
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
//// insert_clause !!! TEST !!!
/////////////////////////////////////////////////////////////////////////////////////
BOOST_FIXTURE_TEST_SUITE( insert_clause, insert_clause_fxt )
BOOST_AUTO_TEST_CASE(basic)
{
	q = "[\
			\"action\" : \"insert\",\
			\"columns\" : [\"basic_test_int\",\"basic_test_varchar\",\"basic_test_bool\"],\
			\"values\" : [0,\"CZK\",true]\
		]";
	rcv_int = insert_( q.data(), q.size(), db_path );
}
BOOST_AUTO_TEST_SUITE_END()

#endif