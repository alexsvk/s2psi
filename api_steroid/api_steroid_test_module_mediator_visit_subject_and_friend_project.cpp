/*!
\brief MMS API steroid tester-module.
\author Alexander Syvak (alexander.svk@gmail.com)
\date August 23 2013, October 3, 2013(timing test suite)
*/
#define BOOST_SPIRIT_DEBUG
//#define TEST_ON
#ifdef TEST_ON

#define DATA_BASE_PATH "../db_filled_delim_@.db"

#pragma comment(lib,"..\\Debug\\sqlite3")
#define LEAN_AND_MEAN
#include <Windows.h>
// SYSTEM
#include <stdexcept> // runtime_error
#include <sstream> // ostringstream
#include <algorithm> // mismatch, equal
#include <vector>
#include <string>

// PROJECT
#define BOOST_TEST_MODULE mms_cpp_steroid_api
//#define BOOST_TEST_NO_MAIN
#if defined(_WIN32) || defined(_WIN64) || defined(__WINDOWS__) || defined(__WIN32__) || defined(__TOS_WIN__)
	#include <boost/test/unit_test.hpp> // Support of the auto-linking feature for MSVS 
										// http://www.boost.org/doc/libs/1_54_0/libs/test/doc/html/utf/compilation/auto-linking.html
#elif defined(__gnu_linux__)
	#include <boost/test/included/unit_test.hpp>
#endif
#include <boost/lexical_cast.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/spirit/include/qi.hpp>
// LOCAL
#include "sql_gen.h" // uses: functions to test
#include "db.h" // uses => data base interface
#include "err_codes.h" // uses => testing errors
//#include "measurement_auto_creator.h" // uses
#include "timer.h" // uses => timing test suite
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
	struct generator
	{
		generator(unsigned const & start = 0U):n(start)
		{
		}
		unsigned operator()()
		{
			return n++;
		}
		unsigned n;
	};
	api_fxt() : rcv(nullptr), db_path(DATA_BASE_PATH)
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
		{
		   deallocate_selected_data( rcv, ::boost::lexical_cast<unsigned>(rcv[0]) + 1 );
		   rcv = nullptr;
		}
	}
	void fill(unsigned const & n, mms_sql_query const & val)
	{
		exp.resize(n);
		std::fill_n( exp.begin(), n, val );
	}
	virtual void chk_q_exec()
	{
		rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
		BOOST_REQUIRE( rcv != nullptr );
		BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER), reinterpret_cast<int>(rcv));	
	}
	char const *db_path;
	mms_sql_query q;
	typedef std::vector<std::string> strings_container;
	strings_container exp;
	selection_double_raw_ptr rcv;
};
struct grp_filter_fxt:api_fxt
{
	grp_filter_fxt():q_base(
								"[\
									\"action\" : \"mediator_and_friend\",\
									\"distinct\" : false,\
									\"columns\" : [\"subject@height\",\"subject@sex\",\"subject@weight\"],\
									\"aggregates\" : {\
														\"subject@sex\" : 1,\
														\"subject@weight\" : 6\
													 },\
									\"as\" : [\
												{\"subject@sex\" : \"sex\"},\
												{\"subject@weight\" : \"weight\"},\
												{\"subject@height\" : \"height\"}\
											 ],\
									\"groups\" : [\
													\"subject@height\"\
												 ],\
									\"group_filter\" : {"
							)
	{
	}
	void set_grp_filter(mms_sql_query const & grp_filter_)
	{
		grp_filter = grp_filter_;
	}
	void chk_q_exec() override
	{
		q = q_base + grp_filter + "}]";
		rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
		BOOST_REQUIRE( rcv != nullptr );
		BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER), reinterpret_cast<int>(rcv));	
	}
	mms_sql_query const q_base;
	mms_sql_query grp_filter;
};
struct filter_fxt:api_fxt
{
	filter_fxt():q_base(
						"[\
							\"action\" : \"mediator_and_friend\",\
							\"distinct\" : false,\
							\"columns\" : [\"project@id\"],\
							\"as\" : [\
										{\"project@id\" : \"id\"},\
										{\"uknown\" : \"null\"}\
									 ],"
					   ), pre_filter( "\"filter\" : {\"" ), post_filter( "\" : {" ), filter_column( "project@id" )
	{
		BOOST_TEST_MESSAGE( "Setup filter_fxt fixture" ); 
	}
	~filter_fxt()
	{ 
		BOOST_TEST_MESSAGE( "Teardown filter_fxt fixture" ); 
	}	
	void set_filter_column(mms_sql_query const & column)
	{
		// N.B. !!!
		// Only for base SELECT test.
#ifdef PIVOTAL_SELECT
		filter_column = column;
#endif
	}
	void mk_q(mms_sql_query const & interval)
	{
		q = q_base + pre_filter + filter_column + post_filter + interval + "}]";	
	}
	//void chk_q_exec() override
	//{
	//	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	//	BOOST_REQUIRE( rcv != nullptr );
	//	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER), reinterpret_cast<int>(rcv));	
	//}

	mms_sql_query q_base, pre_filter, post_filter;
	mms_sql_query q, filter_column;
};
struct mediator_visit_subject_and_friend_project_fxt : public api_fxt
{
	mediator_visit_subject_and_friend_project_fxt()
	{
		BOOST_TEST_MESSAGE( "Setup mediator_visit_subject_and_friend_project@fxt fixture." ); 
	}
	~mediator_visit_subject_and_friend_project_fxt()	
	{ 
		BOOST_TEST_MESSAGE( "Teardown mediator_visit_subject_and_friend_project@fxt fixture." ); 
	}
};
struct mediator_visit_subject_fxt : public api_fxt
{
	mediator_visit_subject_fxt()
	{
		BOOST_TEST_MESSAGE( "Setup mediator_visit_subject_fxt fixture." ); 
	}
	~mediator_visit_subject_fxt()	
	{ 
		BOOST_TEST_MESSAGE( "Teardown mediator_visit_subject_fxt fixture." ); 
	}
};
struct mediator_project_module_fxt : public api_fxt
{
	mediator_project_module_fxt()
	{
		BOOST_TEST_MESSAGE( "Setup mediator_project@module_fxt fixture." ); 
	}
	~mediator_project_module_fxt()	
	{ 
		BOOST_TEST_MESSAGE( "Teardown mediator_project@module_fxt fixture." ); 
	}
};
struct mediator_device_test_set_fxt : public api_fxt
{
	mediator_device_test_set_fxt()
	{
		BOOST_TEST_MESSAGE( "Setup mediator_device_test_set_fxt fixture." ); 
	}
	~mediator_device_test_set_fxt()	
	{ 
		BOOST_TEST_MESSAGE( "Teardown mediator_device_test_set_fxt fixture." ); 
	}
};
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
// mediator_visit_subject_and_friend_project !!! TEST !!!
///////////////////////////////////////////////////////////////////////////////////
BOOST_FIXTURE_TEST_SUITE( mediator_visit_subject_and_friend_project_suite, mediator_visit_subject_and_friend_project_fxt )
BOOST_AUTO_TEST_CASE( basic )
{
	// Test of a non-empty result container.
	q = 
		"[\"action\" : \"mediator_and_friend\",\
		\"distinct\" : false,\
		\"columns\" : [\"project@id\", \"project@name\"],\
		\"aggregates\" : {\"project@id\" : 1},\
		\"filter\" : {\"project@id\" : {\"interval\":{2:7,5:8,\"logic\" : 1}}},\
		\"groups\" : [\"project@name\"]]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER), reinterpret_cast<int>(rcv));
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 2U);
	exp.push_back( "2" ), exp.push_back( "7" ), exp.push_back( "ccdeadhadf" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + 3, exp.cbegin(), exp.cend() );
	reset();
	// Test of an empty result container.
	q = 
		"[\"action\" : \"mediator_and_friend\",\
		\"distinct\" : false,\
		\"columns\" : [\"project@id\", \"project@name\"],\
		\"aggregates\" : {\"project@id\" : 1},\
		\"filter\" : {\"project@id\" : {\"interval\":{2:7,5:8,\"logic\" : 0}}},\
		\"groups\" : [\"project@name\"]]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER), reinterpret_cast<int>(rcv));
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_SUITE(validation_rules)
BOOST_AUTO_TEST_CASE( valid_empty_filter_group_group_filter_order ) // 0 0 0 0
{
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\", \"project@name\"],\
			\"aggregates\" : {\"project@id\" : 1}\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER), reinterpret_cast<int>(rcv));
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 2U);
	exp.push_back( "2" ), exp.push_back( "7" ), exp.push_back( "ccdeadhadf" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + 3, exp.cbegin(), exp.cend() );
}
BOOST_AUTO_TEST_CASE( valid_filter ) // 0 0 0 1
{
	//, \"project@notes\" : {\"in\" : [\"dfdbaehgcb\"]}
	// The rule #1.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@name\"],\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 0\
															   }\
											}\
						}\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
	// The rule #2.
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\",\"project@name\"],\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 0\
															   }\
											}\
						}\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
	// The rule #3.
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\
							\"project@id\",\
							\"project@name\"\
						  ],\
			\"as\" : [\
						{\"project@id\":\"id\"},\
						{\"project@name\":\"project@id\"}\
					 ],\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 0\
															   }\
											}\
						}\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
	// The rule #4.
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\
							\"project@name\"\
						  ],\
			\"as\" : [\
						{\"project@name\":\"project@id\"}\
					 ],\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 0\
															   }\
											}\
						}\
		]";
	// N.B. !!!
	// project@id in the filter may be named also id.

	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE( valid_group ) // 0 0 1 0
{
	// The rule #1.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"groups\" : [\"project@name\"]]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 1U);
	exp.push_back( "1" ), exp.push_back( "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + 2, exp.cbegin(), exp.cend() );
	reset();
	// The rule #2.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\", \"project@name\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"groups\" : [\"project@name\"]]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 2U);
	exp.push_back( "2" ), exp.push_back( "7" ), exp.push_back( "ccdeadhadf" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + 3, exp.cbegin(), exp.cend() );
	reset();
	// The rule #3.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\", \"project@name\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"as\" :\
						[\
							{\
								\"project@name\" : \"name\"\
							}\
						],\
			\"groups\" : [\"project@name\"]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 2U);
	exp.push_back( "2" ), exp.push_back( "7" ), exp.push_back( "ccdeadhadf" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + 3, exp.cbegin(), exp.cend() );
	// The rule #4.
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [ \"project@id\", \"project@name\" ],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"as\" :\
						[\
							{\
								\"project@name\" : \"name\"\
							}\
						],\
			\"groups\" : [\"name\"]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 2U);
	exp.push_back( "2" ), exp.push_back( "7" ), exp.push_back( "ccdeadhadf" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + 3, exp.cbegin(), exp.cend() );
}
BOOST_AUTO_TEST_CASE( valid_filter_and_group ) // 0 0 1 1
{
	// The rule #1.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@notes\"],\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 0\
															   }\
											}\
						},\
			\"groups\" : [\"project@name\"]]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
	// The rule #2.
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\", \"project@name\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 0\
															   }\
											}\
						},\
			\"groups\" : [\"project@name\"]]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
	// The rule #3.
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\", \"project@name\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"as\" :\
						[\
							{\
								\"project@name\" : \"name\"\
							},\
							{\
								\"project@id\" : \"id\"\
							}\
						],\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 0\
															   }\
											}\
						},\
			\"groups\" : [\"project@name\"]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
	// The rule #4.
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [ \"project@notes\", \"project@name\" ],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"as\" :\
						[\
							{\
								\"project@name\" : \"project@id\"\
							},\
							{\
								\"project@notes\" : \"notes\"\
							}\
						],\
			\"filter\" : {\
							\"project@id\" : {\
												\"in\" : [\"abc\"]\
											}\
						},\
			\"groups\" : [\"notes\"]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE( valid_group_and_group_filter ) // 0 1 1 0
{
	// The rule #1.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@notes\"],\
			\"groups\" : [\"project@name\"],\
			\"group_filter\" : {\
									\"project@id\" : {\
														\"interval\" : {\
																			2:7,\
																			5:8,\
																			\"logic\" : 0\
																	   }\
													}\
								}\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
	// The rule #2.
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\",\"project@notes\"],\
			\"groups\" : [\"project@name\"],\
			\"group_filter\" : {\
									\"project@id\" : {\
														\"interval\" : {\
																			2:7,\
																			5:8,\
																			\"logic\" : 0\
																	   }\
													}\
								}\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
	// The rule #3.
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\", \"project@name\"],\
			\"as\" :\
						[\
							{\
								\"project@name\" : \"name\"\
							},\
							{\
								\"project@id\" : \"id\"\
							}\
						],\
			\"groups\" : [\"project@name\"],\
			\"group_filter\" : {\
									\"project@id\" : {\
														\"interval\" : {\
																			2:7,\
																			5:8,\
																			\"logic\" : 0\
																	   }\
													}\
								}\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
	// The rule #4.
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\", \"project@name\"],\
			\"as\" :\
						[\
							{\
								\"project@name\" : \"name\"\
							},\
							{\
								\"project@id\" : \"id\"\
							}\
						],\
			\"groups\" : [\"name\"],\
			\"group_filter\" : {\
									\"id\" : {\
														\"interval\" : {\
																			2:7,\
																			5:8,\
																			\"logic\" : 0\
																	   }\
													}\
								}\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE( valid_filter_and_group_and_group_filter ) // 0 1 1 1
{
	// The rule #1.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@notes\"],\
			\"filter\" : {\
							\"project@id\" : {\
													\"interval\" : {\
																		2:7,\
																		5:8,\
																		\"logic\" : 0\
																   }\
												}\
						},\
			\"groups\" : [\"project@name\"],\
			\"group_filter\" : {\
									\"project@id\" : {\
														\"interval\" : {\
																			2:7,\
																			5:8,\
																			\"logic\" : 0\
																	   }\
													}\
								}\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
	// The rule #2.
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\",\"project@notes\",\"project@name\"],\
			\"filter\" : {\
							\"project@id\" : {\
													\"interval\" : {\
																		2:7,\
																		5:8,\
																		\"logic\" : 0\
																   }\
												}\
						},\
			\"groups\" : [\"project@name\"],\
			\"group_filter\" : {\
									\"project@id\" : {\
														\"interval\" : {\
																			2:7,\
																			5:8,\
																			\"logic\" : 0\
																	   }\
													}\
								}\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
	// The rule #3.
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\", \"project@name\"],\
			\"as\" :\
						[\
							{\
								\"project@name\" : \"name\"\
							},\
							{\
								\"project@id\" : \"id\"\
							}\
						],\
			\"filter\" : {\
							\"project@id\" : {\
													\"interval\" : {\
																		2:7,\
																		5:8,\
																		\"logic\" : 0\
																   }\
												}\
						},\
			\"groups\" : [\"project@name\"],\
			\"group_filter\" : {\
									\"project@id\" : {\
														\"interval\" : {\
																			2:7,\
																			5:8,\
																			\"logic\" : 0\
																	   }\
													}\
								}\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
	// The rule #4.
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\", \"project@name\"],\
			\"as\" :\
						[\
							{\
								\"project@name\" : \"name\"\
							},\
							{\
								\"project@notes\" : \"project@id\"\
							},\
							{\
								\"project@id\" : \"id\"\
							}\
						],\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 0\
																}\
											 }\
						},\
			\"groups\" : [\"name\"],\
			\"group_filter\" : {\
									\"id\" : {\
													\"interval\" : {\
																		2:7,\
																		5:8,\
																		\"logic\" : 0\
																	}\
											 }\
								}\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE( valid_order ) // 1 0 0 0
{
	// The rule #1.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"order\" : [[\"visit_subject_subject_id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 1U);
	exp.push_back( "1" ), exp.push_back( "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + 2, exp.cbegin(), exp.cend() );
	reset();
	// The rule #2.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\",\"visit_subject_subject_id\"],\
			\"aggregates\" : {\
								\"project@id\" : 1,\
								\"visit_subject_subject_id\" : 6\
							 },\
			\"order\" : [[\"visit_subject_subject_id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 2U);
	exp.push_back( "2" ), exp.push_back( "7" ), exp.push_back( "28" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + 3, exp.cbegin(), exp.cend() );
	reset();
	// The rule #3.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\",\"visit_subject_subject_id\"],\
			\"aggregates\" : {\
								\"project@id\" : 1,\
								\"visit_subject_subject_id\" : 6\
							 },\
			\"as\" :\
						[\
							{\
								\"visit_subject_subject_id\" : \"id\"\
							}\
						],\
			\"order\" : [[\"visit_subject_subject_id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 2U);
	exp.push_back( "2" ), exp.push_back( "7" ), exp.push_back( "28" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + 3, exp.cbegin(), exp.cend() );
	// The rule #4.
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\",\"visit_subject_subject_id\"],\
			\"aggregates\" : {\
								\"project@id\" : 1,\
								\"visit_subject_subject_id\" : 6\
							 },\
			\"as\" :\
						[\
							{\
								\"visit_subject_subject_id\" : \"id\"\
							}\
						],\
			\"order\" : [[\"id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 2U);
	exp.push_back( "2" ), exp.push_back( "7" ), exp.push_back( "28" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + 3, exp.cbegin(), exp.cend() );
}
BOOST_AUTO_TEST_CASE( valid_order_and_filter ) // 1 0 0 1
{
	// The rule #1.

	// N.B. !!! The aggregates is omitted because the list of columns is empty.

	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 0\
																}\
											 }\
						},\
			\"order\" : [[\"visit_subject_subject_id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
	reset();
	// The rule #2.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\",\"visit_subject_subject_id\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 0\
																}\
											 }\
						},\
			\"order\" : [[\"visit_subject_subject_id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 2U);
	exp.push_back( "2" ), exp.push_back( "0" ), exp.push_back( "null" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + 3, exp.cbegin(), exp.cend() );
	reset();
	// The rule #3.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\",\"visit_subject_subject_id\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"as\" :\
						[\
							{\
								\"visit_subject_subject_id\" : \"id\"\
							},\
							{\
								\"project@id\" : \"id_\"\
							}\
						],\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 0\
																}\
											 }\
						},\
			\"order\" : [[\"visit_subject_subject_id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 2U);
	exp.push_back( "2" ), exp.push_back( "0" ), exp.push_back( "null" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + 3, exp.cbegin(), exp.cend() );
	// The rule #4.

	// N.B. !!! The project@id column in the data base is filtered in deed, but not the alias.

	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"visit_subject_subject_id\",\"subject_sex\"],\
			\"aggregates\" : {\
								\"subject_sex\" : 1\
							 },\
			\"as\" :\
						[\
							{\
								\"visit_subject_subject_id\" : \"id\"\
							},\
							{\
								\"subject_sex\" : \"project@id\"\
							}\
						],\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 0\
																}\
											 }\
						},\
			\"order\" : [[\"id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 2U);
	exp.push_back( "2" ), exp.push_back( "null" ), exp.push_back( "0" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + 3, exp.cbegin(), exp.cend() );
}
BOOST_AUTO_TEST_CASE( valid_order_and_group ) // 1 0 1 0
{
	// The rule #1.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"visit_notes\"],\
			\"groups\" : [\"project@id\"],\
			\"order\" : [[\"visit_subject_subject_id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 1U);
	exp.push_back( "1" ), exp.push_back( "ccadegbfhe" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + 2, exp.cbegin(), exp.cend() );
	reset();
	// The rule #2.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\",\"visit_subject_subject_id\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"groups\" : [\"project@id\"],\
			\"order\" : [[\"visit_subject_subject_id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 2U);
	exp.push_back( "2" ), exp.push_back( "7" ), exp.push_back( "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + 3, exp.cbegin(), exp.cend() );
	reset();
	// The rule #3.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\",\"visit_subject_subject_id\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"as\" :\
						[\
							{\
								\"visit_subject_subject_id\" : \"id\"\
							},\
							{\
								\"project@id\" : \"id_\"\
							}\
						],\
			\"groups\" : [\"project@id\"],\
			\"order\" : [[\"visit_subject_subject_id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 2U);
	exp.push_back( "2" ), exp.push_back( "7" ), exp.push_back( "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + 3, exp.cbegin(), exp.cend() );
	// The rule #4.
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\",\"visit_subject_subject_id\"],\
			\"as\" :\
						[\
							{\
								\"visit_subject_subject_id\" : \"id\"\
							},\
							{\
								\"project@id\" : \"id_\"\
							}\
						],\
			\"groups\" : [\"id_\"],\
			\"order\" : [[\"id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 2U);
	exp.push_back( "2" ), exp.push_back( "1" ), exp.push_back( "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + 3, exp.cbegin(), exp.cend() );
}
BOOST_AUTO_TEST_CASE( valid_order_and_group_and_filter ) // 1 0 1 1
{
	// The rule #1.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"visit_notes\"],\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 1\
																}\
											 }\
						},\
			\"groups\" : [\"project@id\"],\
			\"order\" : [[\"visit_subject_subject_id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 1U);
	exp.push_back( "1" ), exp.push_back( "ccadegbfhe" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + 2, exp.cbegin(), exp.cend() );
	reset();
	// The rule #2.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\",\"visit_subject_subject_id\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 1\
																}\
											 }\
						},\
			\"groups\" : [\"project@id\"],\
			\"order\" : [[\"visit_subject_subject_id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 2U);
	exp.push_back( "2" ), exp.push_back( "7" ), exp.push_back( "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + 3, exp.cbegin(), exp.cend() );
	reset();
	// The rule #3.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\",\"visit_subject_subject_id\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"as\" :\
						[\
							{\
								\"visit_subject_subject_id\" : \"vid\"\
							},\
							{\
								\"project@id\" : \"pid\"\
							}\
						],\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 1\
																}\
											 }\
						},\
			\"groups\" : [\"project@id\"],\
			\"order\" : [[\"visit_subject_subject_id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 2U);
	exp.push_back( "2" ), exp.push_back( "7" ), exp.push_back( "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + 3, exp.cbegin(), exp.cend() );
	// The rule #4.
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"visit_subject_subject_id\",\"subject_sex\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"as\" :\
						[\
							{\
								\"visit_subject_subject_id\" : \"vid\"\
							},\
							{\
								\"project@id\" : \"pid\"\
							},\
							{\
								\"subject_sex\" : \"project@id\"\
							}\
						],\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 0\
																}\
											 }\
						},\
			\"groups\" : [\"vid\"],\
			\"order\" : [[\"vid\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE( valid_order_and_group_filter_and_group ) // 1 1 1 0
{
	// The rule #1.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"visit_notes\"],\
			\"groups\" : [\"project@id\"],\
			\"group_filter\" : {\
									\"project@id\" : {\
														\"interval\" : {\
																			2:7,\
																			5:8,\
																			\"logic\" : 1\
																		}\
													 }\
						},\
			\"order\" : [[\"visit_subject_subject_id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 1U);
	exp.push_back( "1" ), exp.push_back( "ccadegbfhe" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + 2, exp.cbegin(), exp.cend() );
	reset();
	// The rule #2.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\",\"visit_subject_subject_id\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"groups\" : [\"project@id\"],\
			\"group_filter\" : {\
									\"project@id\" : {\
														\"interval\" : {\
																			2:7,\
																			5:8,\
																			\"logic\" : 1\
																		}\
													 }\
						},\
			\"order\" : [[\"visit_subject_subject_id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 2U);
	exp.push_back( "2" ), exp.push_back( "7" ), exp.push_back( "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + 3, exp.cbegin(), exp.cend() );
	reset();
	// The rule #3.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\",\"visit_subject_subject_id\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"as\" :\
						[\
							{\
								\"visit_subject_subject_id\" : \"vid\"\
							},\
							{\
								\"project@id\" : \"pid\"\
							}\
						],\
			\"groups\" : [\"project@id\"],\
			\"group_filter\" : {\
									\"project@id\" : {\
														\"interval\" : {\
																			2:7,\
																			5:8,\
																			\"logic\" : 1\
																		}\
													 }\
						},\
			\"order\" : [[\"visit_subject_subject_id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 2U);
	exp.push_back( "2" ), exp.push_back( "7" ), exp.push_back( "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + 3, exp.cbegin(), exp.cend() );
	// The rule #4.
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"visit_subject_subject_id\",\"subject_sex\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"as\" :\
						[\
							{\
								\"visit_subject_subject_id\" : \"vid\"\
							},\
							{\
								\"project@id\" : \"pid\"\
							},\
							{\
								\"subject_sex\" : \"project@id\"\
							}\
						],\
			\"groups\" : [\"vid\"],\
			\"group_filter\" : {\
									\"project@id\" : {\
														\"interval\" : {\
																			2:7,\
																			5:8,\
																			\"logic\" : 0\
																		}\
													 }\
								},\
			\"order\" : [[\"vid\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE( valid_filter_and_group_and_group_filter_and_order ) // 1 1 1 1
{
	// The rule #1.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"visit_notes\"],\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 1\
																}\
											 }\
						},\
			\"groups\" : [\"project@id\"],\
			\"group_filter\" : {\
									\"project@id\" : {\
														\"interval\" : {\
																			2:7,\
																			5:8,\
																			\"logic\" : 1\
																		}\
													 }\
						},\
			\"order\" : [[\"visit_subject_subject_id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 1U);
	exp.push_back( "1" ), exp.push_back( "ccadegbfhe" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + 2, exp.cbegin(), exp.cend() );
	reset();
	// The rule #2.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\",\"visit_subject_subject_id\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 1\
																}\
											 }\
						},\
			\"groups\" : [\"project@id\"],\
			\"group_filter\" : {\
									\"project@id\" : {\
														\"interval\" : {\
																			2:7,\
																			5:8,\
																			\"logic\" : 1\
																		}\
													 }\
						},\
			\"order\" : [[\"visit_subject_subject_id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 2U);
	exp.push_back( "2" ), exp.push_back( "7" ), exp.push_back( "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + 3, exp.cbegin(), exp.cend() );
	reset();
	// The rule #3.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\",\"visit_subject_subject_id\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"as\" :\
						[\
							{\
								\"visit_subject_subject_id\" : \"vid\"\
							},\
							{\
								\"project@id\" : \"pid\"\
							}\
						],\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 1\
																}\
											 }\
						},\
			\"groups\" : [\"project@id\"],\
			\"group_filter\" : {\
									\"project@id\" : {\
														\"interval\" : {\
																			2:7,\
																			5:8,\
																			\"logic\" : 1\
																		}\
													 }\
						},\
			\"order\" : [[\"visit_subject_subject_id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 2U);
	exp.push_back( "2" ), exp.push_back( "7" ), exp.push_back( "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + 3, exp.cbegin(), exp.cend() );
	// The rule #4.
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"visit_subject_subject_id\",\"subject_sex\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"as\" :\
						[\
							{\
								\"visit_subject_subject_id\" : \"vid\"\
							},\
							{\
								\"project@id\" : \"pid\"\
							},\
							{\
								\"subject_sex\" : \"project@id\"\
							}\
						],\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 0\
																}\
											 }\
						},\
			\"groups\" : [\"vid\"],\
			\"group_filter\" : {\
									\"project@id\" : {\
														\"interval\" : {\
																			2:7,\
																			5:8,\
																			\"logic\" : 0\
																		}\
													 }\
								},\
			\"order\" : [[\"vid\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_SUITE_END() // validation_rules
BOOST_AUTO_TEST_SUITE(invalid_policy)
BOOST_AUTO_TEST_CASE( filter ) // 0 0 0 1
{
	// The rule #4.
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\
							\"project@id\",\
							\"project@name\"\
						  ],\
			\"as\" : [\
						{\"project@id\":\"project@id\"},\
						{\"project@name\":\"project@id\"}\
					 ],\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 0\
															   }\
											}\
						}\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE( reinterpret_cast<int>(rcv) == MMS_SQL_QUERY_VALIDATION, reinterpret_cast<int>(rcv) );
	// The rule #3.
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\
						  ],\
			\"as\" : [\
						{\"project@id\":\"id\"}\
					 ],\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 0\
															   }\
											}\
						}\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == MMS_SQL_QUERY_VALIDATION, reinterpret_cast<int>(rcv));
}
BOOST_AUTO_TEST_CASE( valid_group ) // 0 0 1 0
{
	// The rule #4.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"as\" : [\
						{\"project@name\":\"id\"},\
						{\"project@name\":\"id\"}\
					 ],\
			\"groups\" : [\"id\"]]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == MMS_SQL_QUERY_VALIDATION , reinterpret_cast<int>(rcv));
	// The rule #3.
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"as\" : [\
						{\"project@name\":\"name\"}\
					 ],\
			\"groups\" : [\"project@name\"]]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == MMS_SQL_QUERY_VALIDATION , reinterpret_cast<int>(rcv));
}
BOOST_AUTO_TEST_CASE( valid_filter_and_group ) // 0 0 1 1
{
	// The rule #4.
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\", \"project@name\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"as\" :\
						[\
							{\
								\"project@name\" : \"project@name\"\
							},\
							{\
								\"project@id\" : \"project@name\"\
							}\
						],\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 0\
															   }\
											}\
						},\
			\"groups\" : [\"project@name\"]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == MMS_SQL_QUERY_VALIDATION , reinterpret_cast<int>(rcv));
	// The rule #3.
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"as\" :\
						[\
							{\
								\"project@name\" : \"project@id\"\
							},\
							{\
								\"project@notes\" : \"notes\"\
							}\
						],\
			\"filter\" : {\
							\"project@name\" : {\
												\"in\" : [\"abc\"]\
											}\
						},\
			\"groups\" : [\"project@notes\"]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == MMS_SQL_QUERY_VALIDATION , reinterpret_cast<int>(rcv));
}
BOOST_AUTO_TEST_CASE( valid_group_and_group_filter ) // 0 1 1 0
{
	// The rule #4.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\", \"project@name\"],\
			\"as\" :\
						[\
							{\
								\"project@name\" : \"project@name\"\
							},\
							{\
								\"project@id\" : \"project@name\"\
							}\
						],\
			\"groups\" : [\"project@name\"],\
			\"group_filter\" : {\
									\"project@id\" : {\
														\"interval\" : {\
																			2:7,\
																			5:8,\
																			\"logic\" : 0\
																	   }\
													}\
								}\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == MMS_SQL_QUERY_VALIDATION , reinterpret_cast<int>(rcv));
	// The rule #3.
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [],\
			\"as\" :\
						[\
							{\
								\"project@name\" : \"name\"\
							},\
							{\
								\"project@id\" : \"id\"\
							}\
						],\
			\"groups\" : [\"project@name\"],\
			\"group_filter\" : {\
									\"project@id\" : {\
														\"interval\" : {\
																			2:7,\
																			5:8,\
																			\"logic\" : 0\
																	   }\
													}\
								}\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == MMS_SQL_QUERY_VALIDATION , reinterpret_cast<int>(rcv));
}
BOOST_AUTO_TEST_CASE( valid_filter_and_group_and_group_filter ) // 0 1 1 1
{
	// The rule #4.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\", \"project@name\",\"project@notes\"],\
			\"as\" :\
						[\
							{\
								\"project@name\" : \"project@name\"\
							},\
							{\
								\"project@id\" : \"project@name\"\
							},\
							{\
								\"project@notes\" : \"project@name\"\
							}\
						],\
			\"filter\" : {\
							\"project@id\" : {\
													\"interval\" : {\
																		2:7,\
																		5:8,\
																		\"logic\" : 0\
																   }\
												}\
						},\
			\"groups\" : [\"project@name\"],\
			\"group_filter\" : {\
									\"project@notes\" : {\
														\"interval\" : {\
																			2:7,\
																			5:8,\
																			\"logic\" : 0\
																	   }\
													}\
								}\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == MMS_SQL_QUERY_VALIDATION , reinterpret_cast<int>(rcv));
	// The rule #3.
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [],\
			\"as\" :\
						[\
							{\
								\"project@name\" : \"project@name\"\
							},\
							{\
								\"project@id\" : \"project@id\"\
							},\
							{\
								\"project@notes\" : \"notes\"\
							}\
						],\
			\"filter\" : {\
							\"project@id\" : {\
													\"interval\" : {\
																		2:7,\
																		5:8,\
																		\"logic\" : 0\
																   }\
												}\
						},\
			\"groups\" : [\"project@name\"],\
			\"group_filter\" : {\
									\"project@notes\" : {\
														\"interval\" : {\
																			2:7,\
																			5:8,\
																			\"logic\" : 0\
																	   }\
													}\
								}\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == MMS_SQL_QUERY_VALIDATION , reinterpret_cast<int>(rcv));
}
BOOST_AUTO_TEST_CASE( valid_order ) // 1 0 0 0
{
	// The rule #4.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\",\"visit_subject_subject_id\"],\
			\"aggregates\" : {\
								\"project@id\" : 1,\
								\"visit_subject_subject_id\" : 6\
							 },\
			\"as\" :\
						[\
							{\
								\"visit_subject_subject_id\" : \"visit_subject_subject_id\"\
							},\
							{\
								\"project@id\" : \"visit_subject_subject_id\"\
							}\
						],\
			\"order\" : [[\"visit_subject_subject_id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == MMS_SQL_QUERY_VALIDATION , reinterpret_cast<int>(rcv));
	// The rule #3.
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\"],\
			\"aggregates\" : {\
								\"project@id\" : 1,\
								\"visit_subject_subject_id\" : 6\
							 },\
			\"as\" :\
						[\
							{\
								\"visit_subject_subject_id\" : \"id\"\
							}\
						],\
			\"order\" : [[\"visit_subject_subject_id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == MMS_SQL_QUERY_VALIDATION , reinterpret_cast<int>(rcv));
}
BOOST_AUTO_TEST_CASE( valid_order_and_filter ) // 1 0 0 1
{
	// The rule #4.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\",\"visit_subject_subject_id\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"as\" :\
						[\
							{\
								\"visit_subject_subject_id\" : \"visit_subject_subject_id\"\
							},\
							{\
								\"project@id\" : \"visit_subject_subject_id\"\
							}\
						],\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 0\
																}\
											 }\
						},\
			\"order\" : [[\"visit_subject_subject_id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == MMS_SQL_QUERY_VALIDATION , reinterpret_cast<int>(rcv));
	// The rule #3.
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [],\
			\"aggregates\" : {\
								\"subject_sex\" : 1\
							 },\
			\"as\" :\
						[\
							{\
								\"visit_subject_subject_id\" : \"id\"\
							},\
							{\
								\"subject_sex\" : \"project@id\"\
							}\
						],\
			\"filter\" : {\
							\"subject_sex\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 0\
																}\
											 }\
						},\
			\"order\" : [[\"visit_subject_subject_id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == MMS_SQL_QUERY_VALIDATION , reinterpret_cast<int>(rcv));
}
BOOST_AUTO_TEST_CASE( valid_order_and_group ) // 1 0 1 0
{
	// The rule #4.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\",\"visit_subject_subject_id\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"as\" :\
						[\
							{\
								\"visit_subject_subject_id\" : \"visit_subject_subject_id\"\
							},\
							{\
								\"project@id\" : \"visit_subject_subject_id\"\
							}\
						],\
			\"groups\" : [\"project@id\"],\
			\"order\" : [[\"visit_subject_subject_id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == MMS_SQL_QUERY_VALIDATION , reinterpret_cast<int>(rcv));
	// The rule #3.W
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [],\
			\"as\" :\
						[\
							{\
								\"visit_subject_subject_id\" : \"id\"\
							},\
							{\
								\"project@id\" : \"id_\"\
							}\
						],\
			\"groups\" : [\"project@id\"],\
			\"order\" : [[\"visit_subject_subject_id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == MMS_SQL_QUERY_VALIDATION , reinterpret_cast<int>(rcv));
}
BOOST_AUTO_TEST_CASE( valid_order_and_group_and_filter ) // 1 0 1 1
{
	// The rule #4.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\",\"visit_subject_subject_id\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"as\" :\
						[\
							{\
								\"visit_subject_subject_id\" : \"visit_subject_subject_id\"\
							},\
							{\
								\"project@id\" : \"visit_subject_subject_id\"\
							}\
						],\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 1\
																}\
											 }\
						},\
			\"groups\" : [\"project@id\"],\
			\"order\" : [[\"visit_subject_subject_id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == MMS_SQL_QUERY_VALIDATION , reinterpret_cast<int>(rcv));
	// The rule #3.
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"as\" :\
						[\
							{\
								\"visit_subject_subject_id\" : \"vid\"\
							},\
							{\
								\"project@id\" : \"pid\"\
							},\
							{\
								\"subject_sex\" : \"project@id\"\
							}\
						],\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 0\
																}\
											 }\
						},\
			\"groups\" : [\"visit_subject_subject_id\"],\
			\"order\" : [[\"subject_sex\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == MMS_SQL_QUERY_VALIDATION , reinterpret_cast<int>(rcv));
}
BOOST_AUTO_TEST_CASE( valid_order_and_group_filter_and_group ) // 1 1 1 0
{
	// The rule #4.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\",\"visit_subject_subject_id\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"as\" :\
						[\
							{\
								\"visit_subject_subject_id\" : \"visit_subject_subject_id\"\
							},\
							{\
								\"project@id\" : \"visit_subject_subject_id\"\
							}\
						],\
			\"groups\" : [\"project@id\"],\
			\"group_filter\" : {\
									\"project@id\" : {\
														\"interval\" : {\
																			2:7,\
																			5:8,\
																			\"logic\" : 1\
																		}\
													 }\
						},\
			\"order\" : [[\"visit_subject_subject_id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == MMS_SQL_QUERY_VALIDATION , reinterpret_cast<int>(rcv));
	// The rule #3.
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"as\" :\
						[\
							{\
								\"visit_subject_subject_id\" : \"vid\"\
							},\
							{\
								\"project@id\" : \"pid\"\
							},\
							{\
								\"subject_sex\" : \"project@id\"\
							}\
						],\
			\"groups\" : [\"visit_subject_subject_id\"],\
			\"group_filter\" : {\
									\"project@id\" : {\
														\"interval\" : {\
																			2:7,\
																			5:8,\
																			\"logic\" : 0\
																		}\
													 }\
								},\
			\"order\" : [[\"subject_sex\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == MMS_SQL_QUERY_VALIDATION , reinterpret_cast<int>(rcv));
}
BOOST_AUTO_TEST_CASE( valid_filter_and_group_and_group_filter_and_order ) // 1 1 1 1
{
	// The rule #4.
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\",\"visit_subject_subject_id\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"as\" :\
						[\
							{\
								\"visit_subject_subject_id\" : \"visit_subject_subject_id\"\
							},\
							{\
								\"project@id\" : \"visit_subject_subject_id\"\
							}\
						],\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 1\
																}\
											 }\
						},\
			\"groups\" : [\"project@id\"],\
			\"group_filter\" : {\
									\"project@id\" : {\
														\"interval\" : {\
																			2:7,\
																			5:8,\
																			\"logic\" : 1\
																		}\
													 }\
						},\
			\"order\" : [[\"visit_subject_subject_id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == MMS_SQL_QUERY_VALIDATION , reinterpret_cast<int>(rcv));
	// The rule #3.
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"as\" :\
						[\
							{\
								\"visit_subject_subject_id\" : \"vid\"\
							},\
							{\
								\"project@id\" : \"pid\"\
							},\
							{\
								\"subject_sex\" : \"project@id\"\
							}\
						],\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 0\
																}\
											 }\
						},\
			\"groups\" : [\"visit_subject_subject_id\"],\
			\"group_filter\" : {\
									\"subject_sex\" : {\
														\"interval\" : {\
																			2:7,\
																			5:8,\
																			\"logic\" : 0\
																		}\
													 }\
								},\
			\"order\" : [[\"visit_subject_subject_id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == MMS_SQL_QUERY_VALIDATION , reinterpret_cast<int>(rcv));
}
BOOST_AUTO_TEST_SUITE_END() // invalid_policy
BOOST_AUTO_TEST_SUITE(all_combinations_of_optional_expressions)
BOOST_AUTO_TEST_CASE(no_optionals)
{
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\"]\
		 ]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER), reinterpret_cast<int>(rcv));
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_SUITE(aggregates)
BOOST_AUTO_TEST_CASE(avg)
{
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\"],\
			\"aggregates\" : {\
								\"project@id\" : 0\
							 }\
		 ]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER), reinterpret_cast<int>(rcv));
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 1U);
	exp = ::boost::assign::list_of(std::string("1")),"25.5";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(count)
{
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 }\
		 ]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER), reinterpret_cast<int>(rcv));
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 1U);
	exp = ::boost::assign::list_of(std::string("1")),"50";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(max)
{
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\"],\
			\"aggregates\" : {\
								\"project@id\" : 4\
							 }\
		 ]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER), reinterpret_cast<int>(rcv));
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 1U);
	exp = ::boost::assign::list_of(std::string("1")),"50";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(min)
{
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\"],\
			\"aggregates\" : {\
								\"project@id\" : 5\
							 }\
		 ]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER), reinterpret_cast<int>(rcv));
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 1U);
	exp = ::boost::assign::list_of(std::string("1")),"1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(sum)
{
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\"],\
			\"aggregates\" : {\
								\"project@id\" : 6\
							 }\
		 ]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER), reinterpret_cast<int>(rcv));
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 1U);
	exp = ::boost::assign::list_of(std::string("1")),"1275";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_SUITE_END() // aggregates
BOOST_AUTO_TEST_CASE(as)
{
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\"],\
			\"as\" : [\
						{\"project@id\" : \"id\"},\
						{\"uknown\" : \"null\"}\
					 ]\
		 ]";	
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER), reinterpret_cast<int>(rcv));
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL( size, 7U );
	exp.reserve(7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_FIXTURE_TEST_SUITE(filter, filter_fxt)
BOOST_AUTO_TEST_CASE(basic)
{
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\"],\
			\"as\" : [\
						{\"project@id\" : \"id\"},\
						{\"uknown\" : \"null\"}\
					 ],\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2 : 7,\
																	5 : 8,\
																	\"logic\" : 0\
															   }\
											}\
						}\
		 ]";	
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER), reinterpret_cast<int>(rcv));
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE(like)
{
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\"],\
			\"as\" : [\
						{\"project@id\" : \"id\"},\
						{\"uknown\" : \"null\"}\
					 ],\
			\"filter\" : {\
							\"project@id\" : {\"like\":\"%3%\"}\
						}\
		 ]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER), reinterpret_cast<int>(rcv));
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL( size, 0U );
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL( size, 14U );
	exp = ::boost::assign::list_of(std::string("14")),"3","13","23","30","31","32","34","34","35","36","37","38","39","43";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\"],\
			\"as\" : [\
						{\"project@id\" : \"id\"},\
						{\"uknown\" : \"null\"}\
					 ],\
			\"filter\" : {\
							\"project@id\" : {\"in\":[6,1,4,8,14,25,30]},\
							\"project@name\" : {\
												  \"like\" : \"%cc%\"\
											   }\
						}\
		 ]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER), reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL( size, 6U );
	exp = ::boost::assign::list_of(std::string("6")),"1","4","8","14","25","30";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
} 
BOOST_AUTO_TEST_CASE(interval_grt_grt_and)
{
	mk_q( "\"interval\" : {0 : 7, 0 : 8, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE(interval_grt_grt_or)
{
	mk_q( "\"interval\" : {0 : 0, 0 : 1, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 14U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_grt_ls_and)
{
	mk_q( "\"interval\" : {0 : 0, 1 : 5, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 14U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_grt_ls_or)
{
	mk_q( "\"interval\" : {0 : 1, 1 : 5, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 14U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_grt_geq_and)
{
	mk_q( "\"interval\" : {0 : 1, 2 : 5, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE(interval_grt_geq_or)
{
	mk_q( "\"interval\" : {0 : 1, 2 : 5, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE(interval_grt_leq_and)
{
	mk_q( "\"interval\" : {0 : 1, 3 : 5, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE(interval_grt_leq_or)
{
	mk_q( "\"interval\" : {0 : 1, 3 : 5, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 14U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_grt_eq_and)
{
	mk_q( "\"interval\" : {0 : 0, 4 : 1, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 14U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_grt_eq_or)
{
	mk_q( "\"interval\" : {0 : 10, 4 : 2, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE(interval_grt_neq_and)
{
	mk_q( "\"interval\" : {0 : 10, 5 : 2, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE(interval_grt_neq_or)
{
	mk_q( "\"interval\" : {0 : 10, 5 : 2, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 14U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_ls_grt_and)
{
	mk_q( "\"interval\" : {1 : 10, 0 : 2, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE(interval_ls_grt_or)
{
	mk_q( "\"interval\" : {1 : 10, 0 : 2, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 14U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_ls_ls_and)
{
	mk_q( "\"interval\" : {1 : 10, 1 : 2, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 14U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_ls_ls_or)
{
	mk_q( "\"interval\" : {1 : 10, 1 : 2, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 14U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_ls_geq_and)
{
	mk_q( "\"interval\" : {1 : 10, 2 : 2, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE(interval_ls_geq_or)
{
	mk_q( "\"interval\" : {1 : 10, 2 : 2, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 14U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_ls_leq_and)
{
	mk_q( "\"interval\" : {1 : 10, 3 : 2, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 14U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_ls_leq_or)
{
	mk_q( "\"interval\" : {1 : 10, 3 : 2, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 14U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_ls_eq_and)
{
	mk_q( "\"interval\" : {1 : 10, 4 : 2, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE(interval_ls_eq_or)
{
	mk_q( "\"interval\" : {1 : 10, 4 : 2, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 14U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_ls_neq_and)
{
	mk_q( "\"interval\" : {1 : 10, 5 : 2, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 14U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_ls_neq_or)
{
	mk_q( "\"interval\" : {1 : 10, 5 : 2, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 14U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_geq_grt_and)
{
	mk_q( "\"interval\" : {2 : 10.5, 0 : 0, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE(interval_geq_grt_or)
{
	mk_q( "\"interval\" : {2 : 10.5, 0 : 0, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 14U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_geq_ls_and)
{
	mk_q( "\"interval\" : {2 : -0.5, 1 : -5, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE(interval_geq_ls_or)
{
	mk_q( "\"interval\" : {2 : -0.5, 1 : -5, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 14U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_geq_geq_and)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {2 : 2.5, 2 : 3.2, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 10U);
	exp = ::boost::assign::list_of(std::string("10")),"1","3.2","1","3.3","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_geq_geq_or)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {2 : 20.5, 2 : 4.2, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 4U);
	exp = ::boost::assign::list_of(std::string("10")),"1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_geq_leq_and)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {2 : 20.5, 3 : 4.2, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE(interval_geq_leq_or)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {2 : 20.5, 3 : 4.2, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 10U);
	exp = ::boost::assign::list_of(std::string("10")),"1","1.5","1","2.3","1","3.2","1","3.3","1","4";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_geq_eq_and)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {2 : 3.5, 4 : -4.2, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE(interval_geq_eq_or)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {2 : 3.5, 4 : -4.2, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 6U);
	exp = ::boost::assign::list_of(std::string("6")),"1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_geq_neq_and)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {2 : 3.5, 5 : -4.2, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 6U);
	exp = ::boost::assign::list_of(std::string("6")),"1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_geq_neq_or)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {2 : 3.5, 5 : -4.2, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 14U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_leq_grt_and)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {3 : 3.5, 0 : -4.2, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 8U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_leq_grt_or)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {3 : 3.5, 0 : -4.2, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 14U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_leq_ls_and)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {3 : -3.5, 1 : -4.2, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE(interval_leq_ls_or)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {3 : -3.5, 1 : -4.2, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE(interval_leq_geq_and)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {3 : -3.5, 2 : -4.2, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE(interval_leq_geq_or)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {3 : -3.5, 2 : -4.2, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 14U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_leq_leq_and)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {3 : 3.5, 3 : -4.2, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE(interval_leq_leq_or)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {3 : 3.5, 3 : -4.2, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 8U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_leq_eq_and)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {3 : 3.5, 4 : -4.2, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE(interval_leq_eq_or)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {3 : 3.5, 4 : -4.2, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 8U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_leq_neq_and)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {3 : 3.5, 5 : -4.2, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 8U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_leq_neq_or)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {3 : 3.5, 5 : -4.2, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 14U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_eq_grt_and)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {4 : 3.5, 0 : -4.2, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]))
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE(interval_eq_grt_or)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {4 : 3.5, 0 : -4.2, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 14U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_eq_ls_and)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {4 : 3.5, 1 : -4.2, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE(interval_eq_ls_or)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {4 : 3.5, 1 : -4.2, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE(interval_eq_geq_and)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {4 : 3.5, 2 : -4.2, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE(interval_eq_geq_or)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {4 : 3.5, 2 : -4.2, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 14U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_eq_leq_and)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {4 : 3.5, 3 : -4.2, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE(interval_eq_leq_or)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {4 : 3.5, 3 : -4.2, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE(interval_eq_eq_and)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {4 : 3.2, 4 : 3.2, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 2U);
	exp = ::boost::assign::list_of(std::string("2")),"1","3.2";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_eq_eq_or)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {4 : 3.2, 5 : 3.2, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 2U);
	exp = ::boost::assign::list_of(std::string("2")),"1","3.2";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_eq_neq_and)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {4 : 3.2, 5 : 3.2, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE(interval_eq_neq_or)
{
	set_filter_column( "height" );
	mk_q( "\"interval\" : {4 : 3.2, 5 : 3.2, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 14U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_neq_grt_and)
{
	set_filter_column( "subject_sex" );
	mk_q( "\"interval\" : {5 : 2, 0 : 3, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 6U);
	exp = ::boost::assign::list_of(std::string("6")),"1","1.5","1","3.2","1","4.4";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_neq_grt_or)
{
	set_filter_column( "subject_sex" );
	mk_q( "\"interval\" : {5 : 2, 0 : 3, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 10U);
	exp = ::boost::assign::list_of(std::string("10")),"1","1.5","1","3.2","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_neq_ls_and)
{
	set_filter_column( "subject_sex" );
	mk_q( "\"interval\" : {5 : 2, 1 : 3, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 4U);
	exp = ::boost::assign::list_of(std::string("4")),"1","4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_neq_ls_or)
{
	set_filter_column( "subject_sex" );
	mk_q( "\"interval\" : {5 : 2, 1 : 3, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 14U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_neq_geq_and)
{
	set_filter_column( "subject_sex" );
	mk_q( "\"interval\" : {5 : 2, 2 : 3, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 6U);
	exp = ::boost::assign::list_of(std::string("6")),"1","1.5","1","3.2","1","4.4";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_neq_geq_or)
{
	set_filter_column( "subject_sex" );
	mk_q( "\"interval\" : {5 : 2, 2 : 3, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 10U);
	exp = ::boost::assign::list_of(std::string("10")),"1","1.5","1","3.2","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_neq_leq_and)
{
	set_filter_column( "subject_sex" );
	mk_q( "\"interval\" : {5 : 2, 3 : 3, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 4U);
	exp = ::boost::assign::list_of(std::string("4")),"1","4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_neq_leq_or)
{
	set_filter_column( "subject_sex" );
	mk_q( "\"interval\" : {5 : 2, 3 : 3, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 14U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_neq_eq_and)
{
	set_filter_column( "subject_sex" );
	mk_q( "\"interval\" : {5 : 2, 4 : 3, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE(interval_neq_eq_or)
{
	set_filter_column( "subject_sex" );
	mk_q( "\"interval\" : {5 : 2, 4 : 3, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 10U);
	exp = ::boost::assign::list_of(std::string("10")),"1","1.5","1","3.2","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_neq_neq_and)
{
	set_filter_column( "subject_sex" );
	mk_q( "\"interval\" : {5 : 2, 5 : 3, \"logic\" : 0}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 10U);
	exp = ::boost::assign::list_of(std::string("10")),"1","1.5","1","3.2","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_neq_neq_or)
{
	set_filter_column( "subject_sex" );
	mk_q( "\"interval\" : {5 : 2, 5 : 3, \"logic\" : 1}" );
	chk_q_exec(); // checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
	BOOST_REQUIRE_EQUAL(size, 14U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#else
	BOOST_REQUIRE_EQUAL(size, 7U);
	fill( 7U, "1" );
	exp.insert( exp.cbegin(), "7" );
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_SUITE_END() // filter
BOOST_AUTO_TEST_CASE(groups)
{
#ifdef PIVOTAL_SELECT
#else
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"subject_height\",\"project@id\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"as\" : [\
						{\"project@id\" : \"id\"},\
						{\"unknown\" : \"null\"},\
						{\"subject_height\" : \"height\"}\
					 ],\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2 : 7,\
																	5 : 8,\
																	\"logic\" : 1\
															   }\
											 }\
						},\
			\"groups\" : [\"height\"]\
		 ]";	
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER), reinterpret_cast<int>(rcv));
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 14U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );	
	reset();
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"subject_height\",\"project@id\"],\
			\"groups\" : [\"subject_height\"]\
		 ]";	
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER), reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 14U);
	exp = ::boost::assign::list_of(std::string("14")),"1","1.5","1","2.3","1","3.2","1","3.3","1","4","1","4.4","1","8.1";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );	
#endif
}
BOOST_FIXTURE_TEST_SUITE(group_filter,grp_filter_fxt)
BOOST_AUTO_TEST_CASE(basic)
{
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\"],\
			\"as\" : [\
						{\"project@id\" : \"id\"},\
						{\"uknown\" : \"null\"}\
					 ],\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2 : 7,\
																	5 : 8,\
																	\"logic\" : 0\
															   }\
											}\
						}\
		 ]";	
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER), reinterpret_cast<int>(rcv));
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_CASE(interval_grt_grt_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {0 : 0, 0 : 8, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_grt_grt_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {0 : 0, 0 : 8, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 15U);
	exp = ::boost::assign::list_of(std::string("15")),"1.5","1","1.6","2.3","1","0.3","3.3","1","2","4","1","3","4.4","1","6.6";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_grt_ls_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {0 : 0, 1 : 5, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 12U);
	exp = ::boost::assign::list_of(std::string("12")),"1.5","1","1.6","2.3","1","0.3","3.3","1","2","4","1","3";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_grt_ls_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {0 : 0, 1 : 5, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 21U);
	exp = ::boost::assign::list_of(std::string("21")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","3.3","1","2","4","1","3","4.4","1","6.6","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_grt_geq_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {0 : 1, 2 : 5, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 3U);
	exp = ::boost::assign::list_of(std::string("3")),"4.4","1","6.6";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_grt_geq_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {0 : 1, 2 : 5, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 12U);
	exp = ::boost::assign::list_of(std::string("12")),"1.5","1","1.6","3.3","1","2","4","1","3","4.4","1","6.6";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_grt_leq_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {0 : 1, 3 : 5, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 9U);
	exp = ::boost::assign::list_of(std::string("9")),"1.5","1","1.6","3.3","1","2","4","1","3";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_grt_leq_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {0 : 1, 3 : 5, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 21U);
	exp = ::boost::assign::list_of(std::string("21")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","3.3","1","2","4","1","3","4.4","1","6.6","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_grt_eq_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {0 : 0, 4 : 1, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_grt_eq_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {0 : 10, 4 : 2, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 3U);
	exp = ::boost::assign::list_of(std::string("3")),"3.3","1","2";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_grt_neq_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {0 : 10, 5 : 2, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_grt_neq_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {0 : 10, 5 : 2, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 18U);
	exp = ::boost::assign::list_of(std::string("18")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","4","1","3","4.4","1","6.6","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_ls_grt_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {1 : 10, 0 : 2, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 6U);
	exp = ::boost::assign::list_of(std::string("6")),"4","1","3","4.4","1","6.6";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_ls_grt_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {1 : 10, 0 : 2, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 21U);
	exp = ::boost::assign::list_of(std::string("21")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","3.3","1","2","4","1","3","4.4","1","6.6","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_ls_ls_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {1 : 10, 1 : 2, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 12U);
	exp = ::boost::assign::list_of(std::string("12")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_ls_ls_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {1 : 10, 1 : 2, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 21U);
	exp = ::boost::assign::list_of(std::string("21")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","3.3","1","2","4","1","3","4.4","1","6.6","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_ls_geq_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {1 : 10, 2 : 2, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 9U);
	exp = ::boost::assign::list_of(std::string("9")),"3.3","1","2","4","1","3","4.4","1","6.6";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_ls_geq_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {1 : 10, 2 : 2, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 21U);
	exp = ::boost::assign::list_of(std::string("21")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","3.3","1","2","4","1","3","4.4","1","6.6","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_ls_leq_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {1 : 10, 3 : 2, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 15U);
	exp = ::boost::assign::list_of(std::string("15")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","3.3","1","2","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_ls_leq_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {1 : 10, 3 : 2, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 21U);
	exp = ::boost::assign::list_of(std::string("21")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","3.3","1","2","4","1","3","4.4","1","6.6","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_ls_eq_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {1 : 10, 4 : 2, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 3U);
	exp = ::boost::assign::list_of(std::string("3")),"3.3","1","2";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_ls_eq_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {1 : 10, 4 : 2, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 21U);
	exp = ::boost::assign::list_of(std::string("21")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","3.3","1","2","4","1","3","4.4","1","6.6","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_ls_neq_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {1 : 10, 5 : 2, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 18U);
	exp = ::boost::assign::list_of(std::string("18")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","4","1","3","4.4","1","6.6","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_ls_neq_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {1 : 10, 5 : 2, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 21U);
	exp = ::boost::assign::list_of(std::string("21")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","3.3","1","2","4","1","3","4.4","1","6.6","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_geq_grt_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {2 : 10.5, 0 : 0, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_geq_grt_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {2 : 10.5, 0 : 0, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 15U);
	exp = ::boost::assign::list_of(std::string("15")),"1.5","1","1.6","2.3","1","0.3","3.3","1","2","4","1","3","4.4","1","6.6";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_geq_ls_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {2 : -0.5, 1 : -5, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_geq_ls_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {2 : -0.5, 1 : -5, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 15U);
	exp = ::boost::assign::list_of(std::string("15")),"1.5","1","1.6","2.3","1","0.3","3.3","1","2","4","1","3","4.4","1","6.6";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_geq_geq_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {2 : -0.5, 2 : -5, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 15U);
	exp = ::boost::assign::list_of(std::string("15")),"1.5","1","1.6","2.3","1","0.3","3.3","1","2","4","1","3","4.4","1","6.6";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_geq_geq_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {2 : -0.5, 2 : -5, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 21U);
	exp = ::boost::assign::list_of(std::string("21")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","3.3","1","2","4","1","3","4.4","1","6.6","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_geq_leq_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {2 : 20.5, 3 : 4.2, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_geq_leq_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {2 : 20.5, 3 : 4.2, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 18U);
	exp = ::boost::assign::list_of(std::string("18")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","3.3","1","2","4","1","3","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_geq_eq_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {2 : 3.5, 4 : -4.2, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_geq_eq_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {2 : 3.5, 4 : -4.2, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 3U);
	exp = ::boost::assign::list_of(std::string("3")),"4.4","1","6.6";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_geq_neq_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {2 : 3.5, 5 : -4.2, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 3U);
	exp = ::boost::assign::list_of(std::string("3")),"4.4","1","6.6";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_geq_neq_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {2 : 3.5, 5 : -4.2, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 21U);
	exp = ::boost::assign::list_of(std::string("21")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","3.3","1","2","4","1","3","4.4","1","6.6","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_leq_grt_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {3 : 3.5, 0 : -4.2, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 18U);
	exp = ::boost::assign::list_of(std::string("18")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","3.3","1","2","4","1","3","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_leq_grt_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {3 : 3.5, 0 : -4.2, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 21U);
	exp = ::boost::assign::list_of(std::string("21")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","3.3","1","2","4","1","3","4.4","1","6.6","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_leq_ls_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {3 : -3.5, 1 : -4.2, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_leq_ls_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {3 : -3.5, 1 : -4.2, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_leq_geq_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {3 : -3.5, 2 : -4.2, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_leq_geq_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {3 : -3.5, 2 : -4.2, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 21U);
	exp = ::boost::assign::list_of(std::string("21")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","3.3","1","2","4","1","3","4.4","1","6.6","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_leq_leq_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {3 : -3.5, 3 : -4.2, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_leq_leq_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {3 : -3.5, 3 : -4.2, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_leq_eq_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {3 : -3.5, 4 : -4.2, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_leq_eq_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {3 : -3.5, 4 : -4.2, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_leq_neq_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {3 : -3.5, 5 : -4.2, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_leq_neq_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {3 : -3.5, 5 : -4.2, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 21U);
	exp = ::boost::assign::list_of(std::string("21")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","3.3","1","2","4","1","3","4.4","1","6.6","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_eq_grt_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {4 : 3.5, 0 : -4.2, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_eq_grt_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {4 : 3.5, 0 : -4.2, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 21U);
	exp = ::boost::assign::list_of(std::string("21")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","3.3","1","2","4","1","3","4.4","1","6.6","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_eq_ls_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {4 : 3.5, 1 : -4.2, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_eq_ls_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {4 : 3.5, 1 : -4.2, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_eq_geq_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {4 : 3.5, 2 : -4.2, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_eq_geq_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {4 : 3.5, 2 : -4.2, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 21U);
	exp = ::boost::assign::list_of(std::string("21")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","3.3","1","2","4","1","3","4.4","1","6.6","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_eq_leq_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {4 : 3.5, 3 : -4.2, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_eq_leq_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {4 : 3.5, 3 : -4.2, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_eq_eq_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {4 : 3.2, 4 : 3.2, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_eq_eq_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {4 : 3.2, 4 : 3.2, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_eq_neq_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {4 : 3.2, 5 : 3.2, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 0U);
#endif
}
BOOST_AUTO_TEST_CASE(interval_eq_neq_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {4 : 3.2, 5 : 3.2, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 21U);
	exp = ::boost::assign::list_of(std::string("21")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","3.3","1","2","4","1","3","4.4","1","6.6","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_neq_grt_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {5 : 2, 0 : 3, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 3U);
	exp = ::boost::assign::list_of(std::string("3")),"4.4","1","6.6";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_neq_grt_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {5 : 2, 0 : 3, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 18U);
	exp = ::boost::assign::list_of(std::string("18")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","4","1","3","4.4","1","6.6","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_neq_ls_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {5 : 2, 1 : 3, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 12U);
	exp = ::boost::assign::list_of(std::string("12")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_neq_ls_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {5 : 2, 1 : 3, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 21U);
	exp = ::boost::assign::list_of(std::string("21")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","3.3","1","2","4","1","3","4.4","1","6.6","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_neq_geq_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {5 : 2, 2 : 3, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 6U);
	exp = ::boost::assign::list_of(std::string("6")),"4","1","3","4.4","1","6.6";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_neq_geq_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {5 : 2, 2 : 3, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 18U);
	exp = ::boost::assign::list_of(std::string("18")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","4","1","3","4.4","1","6.6","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_neq_leq_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {5 : 2, 3 : 3, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 15U);
	exp = ::boost::assign::list_of(std::string("15")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","4","1","3","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_neq_leq_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {5 : 2, 3 : 3, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 21U);
	exp = ::boost::assign::list_of(std::string("21")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","3.3","1","2","4","1","3","4.4","1","6.6","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_neq_eq_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {5 : 2, 4 : 3, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 3U);
	exp = ::boost::assign::list_of(std::string("3")),"4","1","3";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_neq_eq_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {5 : 2, 4 : 3, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 18U);
	exp = ::boost::assign::list_of(std::string("18")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","4","1","3","4.4","1","6.6","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_neq_neq_and)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {5 : 2, 5 : 3, \"logic\" : 0}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 15U);
	exp = ::boost::assign::list_of(std::string("15")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","4.4","1","6.6","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_CASE(interval_neq_neq_or)
{
	set_grp_filter( "\"weight\" : {\"interval\" : {5 : 2, 5 : 3, \"logic\" : 1}}" );
	chk_q_exec(); // compiles the query, checks whether the rcv is a valid pointer and no error occured.
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
#ifdef PIVOTAL_SELECT
#else
	BOOST_REQUIRE_EQUAL(size, 21U);
	exp = ::boost::assign::list_of(std::string("21")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","3.3","1","2","4","1","3","4.4","1","6.6","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
#endif
}
BOOST_AUTO_TEST_SUITE_END() // group_filter
BOOST_AUTO_TEST_CASE(order)
{
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"visit_subject_subject_id\",\"project@id\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"as\" :\
						[\
							{\
								\"visit_subject_subject_id\" : \"id\"\
							},\
							{\
								\"project@id\" : \"id_\"\
							}\
						],\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 0\
																}\
											 }\
						},\
			\"order\" : [[\"visit_subject_subject_id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	unsigned size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	exp.push_back("2");
	exp.push_back("null");
	exp.push_back("0");
	BOOST_REQUIRE_EQUAL(size, 2U);
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
	reset();	
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"project@id\",\"visit_subject_subject_id\"],\
			\"aggregates\" : {\
								\"project@id\" : 1\
							 },\
			\"as\" :\
						[\
							{\
								\"visit_subject_subject_id\" : \"id\"\
							},\
							{\
								\"project@id\" : \"id_\"\
							}\
						],\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 0\
																}\
											 }\
						},\
			\"order\" : [[\"id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	exp.push_back("2");
	exp.push_back("0");
	exp.push_back("null");
	BOOST_REQUIRE_EQUAL(size, 2U);
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
	reset();	
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [],\
			\"filter\" : {\
							\"project@id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 0\
																}\
											 }\
						},\
			\"order\" : [[\"visit_subject_subject_id\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
	reset();
	q =							"[\
									\"action\" : \"mediator_and_friend\",\
									\"distinct\" : false,\
									\"columns\" : [\"subject_height\",\"subject_sex\",\"subject_weight\"],\
									\"aggregates\" : {\
														\"subject_sex\" : 1,\
														\"subject_weight\" : 6\
													 },\
									\"as\" : [\
												{\"subject_sex\" : \"sex\"},\
												{\"subject_weight\" : \"weight\"},\
												{\"subject_height\" : \"height\"}\
											 ],\
									\"groups\" : [\
													\"subject_height\"\
												 ],\
									\"group_filter\" : {""\"weight\" : {\"interval\" : {5 : 2, 5 : 3, \"logic\" : 1}}},\
									\"order\" : [[\"height\"],0]]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 21U);
	exp = ::boost::assign::list_of(std::string("21")),"1.5","1","1.6","2.3","1","0.3","3.2","1","-1","3.3","1","2","4","1","3","4.4","1","6.6","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
}
BOOST_AUTO_TEST_CASE(limit_and_offset)
{
	q =
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"subject_height\",\"subject_sex\",\"subject_weight\"],\
			\"aggregates\" : {\
								\"subject_sex\" : 1,\
								\"subject_weight\" : 6\
								},\
			\"as\" : [\
						{\"subject_sex\" : \"sex\"},\
						{\"subject_weight\" : \"weight\"},\
						{\"subject_height\" : \"height\"}\
						],\
			\"groups\" : [\
							\"subject_height\"\
							],\
			\"group_filter\" : {""\"weight\" : {\"interval\" : {5 : 2, 5 : 3, \"logic\" : 1}}},\
			\"order\" : [[\"height\"],0],\
			\"limit\" : [5,5]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	auto size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 6U);
	exp = ::boost::assign::list_of(std::string("6")),"4.4","1","6.6","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
	reset();
	q =
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"subject_height\",\"subject_sex\",\"subject_weight\"],\
			\"limit\" : [5,5]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 6U);
	exp = ::boost::assign::list_of(std::string("6")),"8.1","0","-0.7","1.5","6","1.6";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
	reset();
	q =
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"subject_height\",\"subject_sex\",\"subject_weight\"],\
			\"aggregates\" : {\
								\"subject_sex\" : 1,\
								\"subject_weight\" : 6\
								},\
			\"as\" : [\
						{\"subject_sex\" : \"sex\"},\
						{\"subject_weight\" : \"weight\"},\
						{\"subject_height\" : \"height\"}\
						],\
			\"groups\" : [\
							\"subject_height\"\
						],\
			\"limit\" : [3,5]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 12U);
	exp = ::boost::assign::list_of(std::string("12")),"3,3","1","2","4","1","3","4.4","1","6.6","8.1","1","-0.7";
	BOOST_CHECK_EQUAL_COLLECTIONS( rcv, rcv + exp.size(), exp.cbegin(), exp.cend() );
	reset();
	q =
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"subject_height\",\"subject_sex\",\"subject_weight\"],\
			\"aggregates\" : {\
								\"subject_sex\" : 1,\
								\"subject_weight\" : 6\
								},\
			\"as\" : [\
						{\"subject_sex\" : \"sex\"},\
						{\"subject_weight\" : \"weight\"},\
						{\"subject_height\" : \"height\"}\
					 ],\
			\"limit\" : [3,5]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
	reset();
	q =
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"subject_height\",\"subject_sex\",\"subject_weight\"],\
			\"aggregates\" : {\
								\"subject_sex\" : 1,\
								\"subject_weight\" : 6\
								},\
			\"limit\" : [3,5]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_SUITE_END() // all_combinations_of_optional_expressions
BOOST_AUTO_TEST_SUITE(errors)
BOOST_AUTO_TEST_CASE(malformed_params)
{
	rcv = mediator_visit_subject_and_friend_project( nullptr, 1, db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == MALFORMED_PARAMS, reinterpret_cast<int>(rcv));
	q = "void*";
	rcv = mediator_visit_subject_and_friend_project( q.data(), 0, db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == MALFORMED_PARAMS, reinterpret_cast<int>(rcv));
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), nullptr );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == MALFORMED_PARAMS, reinterpret_cast<int>(rcv));
}
BOOST_AUTO_TEST_CASE(parser)
{
	q =
		"\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"subject_height\",\"subject_sex\",\"subject_weight\"]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == PARSER, reinterpret_cast<int>(rcv));
}
BOOST_AUTO_TEST_CASE(values_size_is_zero)
{

}
BOOST_AUTO_TEST_CASE(column_delimeter_was_not_found)
{
	q =
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"subjectheight\",\"subject_sex\",\"subject_weight\"]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == COLUMN_DELIMITER_WAS_NOT_FOUND, reinterpret_cast<int>(rcv));
}
BOOST_AUTO_TEST_CASE(open_db)
{
	q =
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"subject_height\",\"subject_sex\",\"subject_weight\"]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), "non-existent db" );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == OPEN_DB, reinterpret_cast<int>(rcv));
}
BOOST_AUTO_TEST_CASE(select_from_db)
{
	q =
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [\"subject_h\",\"subject_sex\",\"subject_weight\"]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == SELECT_FROM_DB, reinterpret_cast<int>(rcv));
}
//BOOST_AUTO_TEST_CASE(mem_alloc)
//{
//	q =
//		"[\
//			\"action\" : \"mediator_and_friend\",\
//			\"distinct\" : false,\
//			\"columns\" : []\
//		]";
//    MEMORYSTATUSEX status;
//    status.dwLength = sizeof(status);
//    GlobalMemoryStatusEx(&status);
//    std::cout << status.ullAvailPhys << '\n';
//	auto const* unused_mem_ptr = new char[status.ullAvailPhys+200000000-5000000];
//	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
//	delete [] unused_mem_ptr;
//	BOOST_REQUIRE( rcv != nullptr );
//	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == MEM_ALLOC, reinterpret_cast<int>(rcv));
//}
BOOST_AUTO_TEST_CASE(boost_lexical_cast)
{

}
BOOST_AUTO_TEST_CASE(std_push_back_mem_alloc)
{

}
BOOST_AUTO_TEST_CASE(no_sqlite3_data_handle_is_set)
{

}
BOOST_AUTO_TEST_CASE(mms_sql_query_validation)
{
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [],\
			\"as\" : [\
						{\"subject_sex\" : \"sex\"},\
						{\"subject_weight\" : \"weight\"},\
						{\"subject_height\" : \"height\"}\
					 ],\
			\"order\" : [[\"subject_sex\"],1]\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == MMS_SQL_QUERY_VALIDATION, reinterpret_cast<int>(rcv));
}
BOOST_AUTO_TEST_CASE(mediator_and_friend_main_filter)
{
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"distinct\" : false,\
			\"columns\" : [],\
			\"filter\" : {\
							\"id\" : {\
												\"interval\" : {\
																	2:7,\
																	5:8,\
																	\"logic\" : 0\
																}\
											 }\
						}\
		]";
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(reinterpret_cast<int>(rcv) == MEDIATOR_AND_FRIEND_MAIN_FILTER, reinterpret_cast<int>(rcv));
}
BOOST_AUTO_TEST_SUITE_END() // errors
BOOST_AUTO_TEST_SUITE(timing)
BOOST_AUTO_TEST_CASE(all_columns_no_filter)
{
	q = 
		"[\
			\"action\" : \"mediator_and_friend\",\
			\"columns\" : []\
		]";	
	timer tmr;
	tmr.start();
	rcv = mediator_visit_subject_and_friend_project( q.data(), q.size(), db_path );
	BOOST_MESSAGE( "\nTime of execution => " << std::fixed << tmr.get_time() << " [ms]." );
	BOOST_REQUIRE( rcv != nullptr );
	BOOST_REQUIRE_MESSAGE(!(reinterpret_cast<int>(rcv) <= FIRST_CODE_NUMBER && reinterpret_cast<int>(rcv) >= LAST_CODE_NUMBER) , reinterpret_cast<int>(rcv));
	auto size = 0U;
	BOOST_REQUIRE_NO_THROW(size = ::boost::lexical_cast<unsigned>(rcv[0]));
	BOOST_REQUIRE_EQUAL(size, 0U);
}
BOOST_AUTO_TEST_SUITE_END() // timing
BOOST_AUTO_TEST_SUITE_END() // mediator_visit_subject_and_friend_project
//BOOST_AUTO_TEST_CASE(creator)
//{
//	mms::sqlite3_measurement_auto_creator cr("doeifdeegg","data_base.db");
//	try
//	{
//		cr.do_();
//	}
//	catch ( int e )
//	{
//		std::cout << e;
//	}
//}
// 1. Invalid policy.
// 2. All combinations of SELECT clause.
// 3. All errors.
// 4. Time of selecting bulks of data.
#endif // ifdef TEST_ON

