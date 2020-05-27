/*!
\brief MMS parsers tester.
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

// LOCAL
#include "db.h"
#include "parsers.hpp"
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
// Data base
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct db_fixture 
{
	db_fixture() : db_name( "data_base.db" ) 
	{ 
		BOOST_TEST_MESSAGE( "setup DB" ); 
		db_.open( db_name );
	}
	~db_fixture()         
	{ 
		BOOST_TEST_MESSAGE( "teardown DB" ); 
		db_.close();
	}
	mms::db::sql_q q;
	mms::db db_;
    private:
		mms::db::source const db_name;
};
struct select_from_project_module_fxt : public db_fixture
{
	select_from_project_module_fxt() : all_selector(all_selector_impl)
	{ 
		BOOST_TEST_MESSAGE( "setup select_from_project_module_fixture" ); 
	}
	~select_from_project_module_fxt()         
	{ 
		BOOST_TEST_MESSAGE( "teardown select_from_project_module_fixture" ); 
	}
	static int all_selector_impl(void*,int,char** r,char**)
	{
		cnt = boost::lexical_cast<int>(r[0]);
		return 0;
	}
	static unsigned cnt;
	mms::db::data_reader_func all_selector;
};

unsigned select_from_project_module_fxt::cnt = 0U;
BOOST_AUTO_TEST_SUITE( db_api )

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
// INSERT
///////////////////////////////////////////////////////////////////////////////////
BOOST_FIXTURE_TEST_SUITE( insert_5000_into_project_module, db_fixture )
BOOST_AUTO_TEST_CASE( insert_project )
{
	std::string project_names [] = {"\"rally\"","\"street_race\"","\"mms\""};
	auto projects_cnt = sizeof(project_names)/sizeof(project_names[0]);
	std::string project_notes [] = {"\"NY\"","\"Paris\"","\"Moscow\"","\"London\""};
	auto project_notes_cnt = sizeof(project_names)/sizeof(project_names[0]);
	for ( auto a = 0U; a < 5000U; ++a )
	{
		q = "insert into project(project_name,project_notes) values(";
		q += project_names[a%projects_cnt] + ',';
		q += project_notes[(a+1)%project_notes_cnt] + ')';
		BOOST_REQUIRE_NO_THROW( db_.insert(q) );		
		BOOST_TEST_CHECKPOINT( "Calling db_.insert(q) with q=" << q );
	}
}
BOOST_AUTO_TEST_CASE( insert_module )
{
	std::string module_names [] = {"\"anchorman\"","\"street\"","\"weight\""};
	for ( auto a = 0U; a < 5000U; ++a )
	{
		q = "insert into module(module_name) values(";
		q += module_names[a%(sizeof(module_names)/sizeof(module_names[0]))] + ')';
		BOOST_REQUIRE_NO_THROW( db_.insert(q) );
		BOOST_TEST_CHECKPOINT( "Calling db_.insert(q) with q=" << q );
	}
}
BOOST_AUTO_TEST_CASE( insert_project_module )
{
	for ( auto a = 0U; a < 5000U; ++a )
	{
		q = "insert into project_module values(";
		auto v = boost::lexical_cast<std::string>(a+1);
		q += v + ',';
		q += v + ')';
		BOOST_CHECK_NO_THROW( db_.insert(q) );
		BOOST_TEST_CHECKPOINT( "Calling db_.insert(q) with q=" << q );
	}
	BOOST_CHECK_THROW( db_.insert(q), std::runtime_error );
}
BOOST_AUTO_TEST_SUITE_END() // insert_5000_into_project_module

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
// SELECT
///////////////////////////////////////////////////////////////////////////////////
BOOST_FIXTURE_TEST_SUITE( select_5000_from_project_module, select_from_project_module_fxt )
BOOST_AUTO_TEST_CASE( select_project )
{	
	cnt = 0U;
	q = "select count(*) from project";
	db_.select(q, all_selector);
	BOOST_CHECK_EQUAL(cnt,5000U);
}
BOOST_AUTO_TEST_CASE( select_module )
{
	cnt = 0U;
	q = "select count(*) from module";
	db_.select(q, all_selector);
	BOOST_CHECK_EQUAL(cnt,5000U);
}
BOOST_AUTO_TEST_SUITE_END() // select_5000_from_project_module

BOOST_AUTO_TEST_SUITE_END() // db_api
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
// PARSERS
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef ::std::string::const_iterator it;
typedef ::boost::spirit::qi::expectation_failure<it> boost_qi_exception;
typedef ::boost::spirit::qi::space_type skipper;
template<class clause_container, class parser>
struct parsers_fxt
{
	parsers_fxt()
	{ 
		reset();
		BOOST_TEST_MESSAGE( "setup parsers_fxt" ); 
	}
	~parsers_fxt()
	{ 
		BOOST_TEST_MESSAGE( "teardown parsers_fxt" ); 
	}	
	void reset()
	{
		exp = rcv = clause_container();
	}
	::std::string q;
	clause_container exp;
	clause_container rcv;
	parser p;
};

struct insert_fxt : public parsers_fxt<mms::insert_clause_container,mms::insert_parser<it,skipper>>
{
	insert_fxt()
	{
		BOOST_TEST_MESSAGE( "Setup INSERT clause fixture." ); 
	}
	~insert_fxt()	
	{ 
		BOOST_TEST_MESSAGE( "Teardown INSERT clause fixture." ); 
	}
};
struct create_fxt : public parsers_fxt<mms::create_clause_container,mms::create_parser<it,skipper>>
{
	create_fxt()
	{
		BOOST_TEST_MESSAGE( "Setup CREATE clause fixture." ); 
	}
	~create_fxt()	
	{ 
		BOOST_TEST_MESSAGE( "Teardown CREATE clause fixture." ); 
	}
};
struct update_fxt : public parsers_fxt<mms::update_clause_container,mms::update_parser<it,skipper>>
{
	update_fxt()
	{
		BOOST_TEST_MESSAGE( "Setup UPDATE clause fixture." ); 
	}
	~update_fxt()	
	{ 
		BOOST_TEST_MESSAGE( "Teardown UPDATE clause fixture." ); 
	}
};
struct delete_fxt : public parsers_fxt<mms::delete_clause_container,mms::delete_parser<it,skipper>>
{
	delete_fxt()
	{
		BOOST_TEST_MESSAGE( "Setup DELETE clause fixture." ); 
	}
	~delete_fxt()	
	{ 
		BOOST_TEST_MESSAGE( "Teardown DELETE clause fixture." ); 
	}
};
struct select_fxt : public parsers_fxt<mms::select_clause_container,mms::select_parser<it,skipper>>
{
	select_fxt()
	{
		BOOST_TEST_MESSAGE( "Setup SELECT clause fixture." ); 
	}
	~select_fxt()	
	{ 
		BOOST_TEST_MESSAGE( "Teardown SELECT clause fixture." ); 
	}
};
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
// INSERT clause
template<typename citer, typename val>
bool valid(citer const & cit, val const & val_)
{
	return cit != val_.cend();
}
template<typename out_stream, typename clause>
void chk_columns(out_stream & msg, clause const & exp, clause const & rcv)
{
	if ( 
			(
				exp.columns.size() == 0 || rcv.columns.size() == 0 
			)
			&&
			exp.columns.size() != rcv.columns.size()
		)
	{
		msg << "\nExpected columns size => " << exp.columns.size() << "\nReceived columns size => " << rcv.columns.size() << '\n';
		return;
	}
	if (exp.columns.size() != rcv.columns.size() || exp.columns.size() != 0 && !std::equal(exp.columns.cbegin(), exp.columns.cend(), rcv.columns.cbegin())) 
	{
		auto mismatch_pair = std::mismatch( exp.columns.cbegin(), exp.columns.cend(), rcv.columns.cbegin() );
		msg << "Difference in field columns: " << (valid(mismatch_pair.first, exp.columns) ? *mismatch_pair.first : "") << ", " << (valid(mismatch_pair.second, rcv.columns) ? *mismatch_pair.second : "");
	}
}
struct value_visitor : public boost::static_visitor<void>
{
    void operator()(std::string const & s) const
    {
        std::cout << "std::string was parsed => " << s << '\n';
    }
    void operator()(int const & i) const
    {
        std::cout << "int was parsed => " << i << '\n';
    }
    void operator()(double const & d) const
    {
        std::cout << "double was parsed => " << d << '\n';
    }
    void operator()(bool const & b) const
    {
        std::cout << "bool was parsed => " << b<< '\n';
    }
    void operator()(column_and_aggregate const & aggr) const
    {
	std::cout << "aggregate was parsed => " << aggr.first << ':' << static_cast<int>( aggr.second ) << '\n';
    }
};
template<typename out_stream, typename clause>
void chk_values(out_stream & msg, clause const & exp, clause const & rcv)
{
	if ( 
			(
				exp.values.size() == 0 || rcv.values.size() == 0 
			)
			&&
			exp.values.size() != rcv.values.size()
		)
	{
		msg << "\nExpected values size => " << exp.values.size() << "\nReceived values size => " << rcv.values.size() << '\n';
		return;
	}
	if (exp.values.size() != rcv.values.size() || exp.values.size() != 0 && !std::equal(exp.values.cbegin(), exp.values.cend(), rcv.values.cbegin())) 
	{
		auto mismatch_pair = std::mismatch( exp.values.cbegin(), exp.values.cend(), rcv.values.cbegin() );
		msg << "Difference in field values: ";
		if ( valid( mismatch_pair.first, exp.values ) ) ::boost::apply_visitor( value_visitor(), *mismatch_pair.first );
		msg << ", ";
		if ( valid( mismatch_pair.second, rcv.values ) ) ::boost::apply_visitor( value_visitor(), *mismatch_pair.second );
	}
}
template<typename out_stream>
void chk_default_all(out_stream & msg, insert_clause_container const & exp, insert_clause_container const & rcv)
{
	if (exp.default_all != rcv.default_all)  msg << "Difference in field default_all: " << std::boolalpha << exp.default_all << ", " << rcv.default_all;
}
boost::test_tools::predicate_result compare_insert_clauses(insert_clause_container const & exp, insert_clause_container const & rcv)
{
	boost::test_tools::predicate_result r( true );
	std::ostringstream msg;
	chk_columns(msg,exp,rcv);
	chk_default_all(msg,exp,rcv);
	chk_values(msg,exp,rcv);
	if ( msg.str().size() ) 
	{
		r.message() << msg.str();
		r = false;
	}
   	return r;
}
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
// SELECT clause
std::ostream & operator<<(std::ostream & os, mms::interval const & i)
{
	os << "Interval=>\n";
	os << "Relation with max => " << i.relation_with_max << " Max => "; 
	boost::apply_visitor(value_visitor(),i.max_val); // operator<<(std::basic_ostream<ElemType,Traits> & out, 
             						 // const variant<T1, T2, ..., TN> & rhs); is not used by way of more verbous output
	os << "Relation with min => " << i.relation_with_min << " Min => ";
	boost::apply_visitor(value_visitor(),i.min_val); // operator<<(std::basic_ostream<ElemType,Traits> & out, 
             						 // const variant<T1, T2, ..., TN> & rhs); is not used by way of more verbous output
	os << "Logic operator => " << i.concatenation_logic << '\n';
	return os;
}
std::ostream & operator<<(std::ostream & os, mms::set const & s)
{
	std::cout << "Set =>\n";
	for ( auto const & e : s ) ::boost::apply_visitor(value_visitor(), e); // operator<<(std::basic_ostream<ElemType,Traits> & out, 
             								     // const variant<T1, T2, ..., TN> & rhs); is not used by way of more verbous output
	return os;
}
struct filter_visitor : public ::boost::static_visitor<void>
{
    void operator()(mms::interval const & i) const
    {
		std::cout << i;
    }
    void operator()(mms::set const & s) const
    {
		std::cout << s;
    }
};

template<typename out_stream>
void chk_distinct(out_stream & msg, mms::select_clause_container const & exp, mms::select_clause_container const & rcv)
{
	if (exp.distinct != rcv.distinct)  msg << "Difference in field distinct: " << std::boolalpha << exp.distinct << ", " << rcv.distinct;
}

template<typename out_stream>
void chk_aggregates(out_stream & msg, mms::select_clause_container const & exp, mms::select_clause_container const & rcv)
{
	if ( 
			(
				exp.aggregates.size() == 0 || rcv.aggregates.size() == 0 
			)
			&&
			exp.aggregates.size() != rcv.aggregates.size()
		)
	{
		msg << "\nExpected aggregates size => " << exp.aggregates.size() << "\nReceived aggregates size => " << rcv.aggregates.size() << '\n';
		return;
	}
	if (exp.aggregates.size() != rcv.aggregates.size() || exp.aggregates.size() != 0 && !std::equal(exp.aggregates.cbegin(), exp.aggregates.cend(), rcv.aggregates.cbegin())) 
	{
		msg << "Difference in field aggregates: ";
		auto mismatch_pair = std::mismatch( exp.aggregates.cbegin(), exp.aggregates.cend(), rcv.aggregates.cbegin() );
		auto const & exp_aggr(valid(mismatch_pair.first, exp.aggregates) ? *mismatch_pair.first : column_and_aggregate()), rcv_aggr(valid(mismatch_pair.second, rcv.aggregates) ? *mismatch_pair.second : column_and_aggregate());
		msg << exp_aggr.first << " : " << exp_aggr.second << ", " << rcv_aggr.first << " : " << rcv_aggr.second;		
	}
}
bool operator==(mms::interval const & f1, mms::interval const & f2)
{
	if ( f1.relation_with_min == f2.relation_with_min && f1.min_val == f2.min_val && f1.relation_with_max == f2.relation_with_max && f1.max_val == f2.max_val && f1.concatenation_logic == f2.concatenation_logic ) return true;
	return false;
}
template<typename out_stream, typename clause_container>
void chk_filters(out_stream & msg, mms::clause_container const & exp, mms::clause_container const & rcv)
{
	if ( 
			(
				exp.filters.size() == 0 || rcv.filters.size() == 0 
			)
			&&
			exp.filters.size() != rcv.filters.size()
		)
	{
		msg << "\nExpected filters size => " << exp.filters.size() << "\nReceived filters size => " << rcv.filters.size() << '\n';
		return;
	}
	if (exp.filters.size() != rcv.filters.size() || exp.filters.size() != 0 && !std::equal(exp.filters.cbegin(), exp.filters.cend(), rcv.filters.cbegin())) 
	{
		msg << "Difference in field filters: ";
		auto mismatch_pair = std::mismatch( exp.filters.cbegin(), exp.filters.cend(), rcv.filters.cbegin() );
		auto const & exp_aggr(valid(mismatch_pair.first, exp.filters) ? *mismatch_pair.first : column_to_filter()), rcv_aggr(valid(mismatch_pair.second, rcv.filters) ? *mismatch_pair.second : column_to_filter());
		msg << exp_aggr.first << " : " << exp_aggr.second << ", " << rcv_aggr.first << " : " << rcv_aggr.second;		
	}
}
template<typename out_stream>
void chk_groups(out_stream & msg, mms::select_clause_container const & exp, mms::select_clause_container const & rcv)
{
	if ( 
			(
				exp.groups.size() == 0 || rcv.groups.size() == 0 
			)
			&&
			exp.groups.size() != rcv.groups.size()
		)
	{
		msg << "\nExpected groups filters size => " << exp.groups.size() << "\nReceived groups filters size => " << rcv.groups.size() << '\n';
		return;
	}
	if (exp.groups.size() != rcv.groups.size() || exp.groups.size() && !std::equal(exp.groups.cbegin(), exp.groups.cend(), rcv.groups.cbegin())) 
	{
		auto mismatch_pair = std::mismatch( exp.groups.cbegin(), exp.groups.cend(), rcv.groups.cbegin() );
		msg << "Difference in field groups: " << (valid(mismatch_pair.first, exp.groups) ? *mismatch_pair.first : "") << ", " << (valid(mismatch_pair.second, rcv.groups) ? *mismatch_pair.second : "");
	}
}
template<typename out_stream>
void chk_group_filters(out_stream & msg, mms::select_clause_container const & exp, mms::select_clause_container const & rcv)
{
	if ( 
			(
				exp.group_filters.size() == 0 || rcv.group_filters.size() == 0 
			)
			&&
			exp.group_filters.size() != rcv.group_filters.size()
		)
	{
		msg << "\nExpected group filters size => " << exp.group_filters.size() << "\nReceived group filters size => " << rcv.group_filters.size() << '\n';
		return;
	}
	if (exp.group_filters.size() != rcv.group_filters.size() || exp.group_filters.size() != 0 && !std::equal(exp.group_filters.cbegin(), exp.group_filters.cend(), rcv.group_filters.cbegin())) 
	{
		msg << "Difference in field group filters: ";
		auto mismatch_pair = std::mismatch( exp.group_filters.cbegin(), exp.group_filters.cend(), rcv.group_filters.cbegin() );
		auto const & exp_aggr(valid(mismatch_pair.first, exp.group_filters) ? *mismatch_pair.first : column_to_filter()), rcv_aggr(valid(mismatch_pair.second, rcv.group_filters) ? *mismatch_pair.second : column_to_filter());
		msg << exp_aggr.first << " : " << exp_aggr.second << ", " << rcv_aggr.first << " : " << rcv_aggr.second;		
	}
}
bool operator!=(mms::order_container const & lord, mms::order_container const & rord)
{
	if ( lord.order == rord.order && std::equal(lord.columns.cbegin(),lord.columns.cend(),rord.columns.cbegin()) ) return false;
	return true;
}
template<typename out_stream>
void chk_order_options(out_stream & msg, mms::select_clause_container const & exp, mms::select_clause_container const & rcv)
{
	if ( exp.order_options != rcv.order_options )
	{
		auto const & exp_(exp.order_options), rcv_(rcv.order_options);
		msg << "Difference in field order_options: ";
		if ( exp_.order != rcv_.order ) msg << exp_.order << ", " << rcv_.order;	
		if ( exp_.columns.size() != rcv_.columns.size() || exp_.columns.size() != 0 && !std::equal( exp_.columns.cbegin(),exp_.columns.cend(),rcv_.columns.cbegin() ) )
		{
			msg << '\n';
			auto mismatch_pair = std::mismatch( exp_.columns.cbegin(), exp_.columns.cend(), rcv_.columns.cbegin() );
			msg << (valid(mismatch_pair.first, exp_.columns) ? *mismatch_pair.first : "") << ", " << (valid(mismatch_pair.second, rcv_.columns) ? *mismatch_pair.second : "");
		}
	
	}
}
template<typename out_stream>
void chk_limit_options(out_stream & msg, mms::select_clause_container const & exp, mms::select_clause_container const & rcv)
{
	if ( exp.limit_options != rcv.limit_options )
	{
		msg << "Difference in field limit options: ";
		auto const & exp_(exp.limit_options), rcv_(rcv.limit_options);
		if ( exp_.first != rcv_.first ) msg << " (offset: " << exp_.first << ", " << rcv_.first << ')';
		if ( exp_.second != rcv_.second ) msg << " (count: " << exp_.second << ", " << rcv_.second << ')';
	}
}
boost::test_tools::predicate_result compare_select_clauses(mms::select_clause_container const & exp, mms::select_clause_container const & rcv)
{
	boost::test_tools::predicate_result r( true );
	std::ostringstream msg;
	chk_distinct(msg,exp,rcv);
	chk_columns(msg,exp,rcv);
	chk_aggregates(msg,exp,rcv);
	chk_filters(msg,exp,rcv);
	chk_groups(msg,exp,rcv);
	chk_group_filters(msg,exp,rcv);
	chk_order_options(msg,exp,rcv);
	chk_limit_options(msg,exp,rcv);
	if ( msg.str().size() ) 
	{
		r.message() << msg.str();
		r = false;
	}
   	return r;
}

std::ostream & operator<<(std::ostream & cout, mms::select_clause_container const & sel_clause)
{
	cout << "distinct = > " << std::boolalpha << sel_clause.distinct << '\n';
	cout << "columns = > ";
	for ( auto const & col : sel_clause.columns ) cout << '\n' << col;
	cout << "\naggregates = > \n";
	for ( auto const & col_and_aggr : sel_clause.aggregates ) cout << col_and_aggr.first << ':' << col_and_aggr.second << '\n';
	cout << "filter = > \n";
	for ( auto const & col_and_flt : sel_clause.filters )
	{
		cout << "Column: " << col_and_flt.first << '\n';
		::boost::apply_visitor(filter_visitor(), col_and_flt.second);
	}
	cout << "groups => \n";
	for ( auto const & g : sel_clause.groups ) cout << g << ',';
	cout << "\ngroup filters => \n";
	for ( auto const & g_and_flt : sel_clause.group_filters )
	{
		cout << "Group: " << g_and_flt.first << '\n';
		::boost::apply_visitor(filter_visitor(), g_and_flt.second);
	}
	cout << "order options => \n";
	for ( auto const & col : sel_clause.order_options.columns ) cout << col << ',';
	cout << "\nused order: " << sel_clause.order_options.order;
	cout << "\nlimit => " << sel_clause.limit_options.first << "\noffset => " << sel_clause.limit_options.second;
	return cout;
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
// CREATE clause
bool operator==(mms::sql_data_type_wrapper const & lhs_wrp, mms::sql_data_type_wrapper const & rhs_wrp)
{
	return lhs_wrp.v == rhs_wrp.v;
}
std::ostream & operator<<(std::ostream & os, sql_data_type const & sql_data_type_)
{
	switch ( sql_data_type_ )
	{
	case mms::sql_data_type::int_: os << "int";
		break;
	case mms::sql_data_type::datetime: os << "datetime";
		break;
	case mms::sql_data_type::double_: os << "double";
		break;
	case mms::sql_data_type::bool_: os << "bool";
		break;
	case mms::sql_data_type::varchar: os << "varchar";
		break;
	}
	return os;
}
template<typename out_stream, typename clause_container>
void chk_types(out_stream & msg, mms::clause_container const & exp, mms::clause_container const & rcv)
{
	if ( 
			(
				exp.types.size() == 0 || rcv.types.size() == 0 
			)
			&&
			exp.types.size() != rcv.types.size()
		)
	{
		msg << "\nExpected types size => " << exp.types.size() << "\nReceived types size => " << rcv.types.size() << '\n';
		return;
	}
	if (exp.types.size() != rcv.types.size() || exp.types.size() && !std::equal(exp.types.cbegin(), exp.types.cend(), rcv.types.cbegin())) 
	{
		auto mismatch_pair = std::mismatch( exp.types.cbegin(), exp.types.cend(), rcv.types.cbegin() );
		msg << "A difference in types: " <<		
			(
				valid(mismatch_pair.first, exp.types) ? mismatch_pair.first->v : sql_data_type_wrapper()
			) << ", " << 
			(
				valid(mismatch_pair.second, rcv.types) ? mismatch_pair.second->v : sql_data_type_wrapper()
			);
	}
}
bool operator==(mms::constraint_container const & lhs_cons, mms::constraint_container const & rhs_cons)
{
	auto const & l = lhs_cons, r = rhs_cons;
	return 
			l.def_value		==		r.def_value		&&
			l.null			==		r.null			&&
			l.primary		==		r.primary		&&
			l.foreign		==		r.foreign;
}
std::ostream & operator<<(std::ostream & os, mms::constraint_container const & constr)
{
	return 
		os 
			<< constr.def_value << '\n' << std::boolalpha 
			<< constr.null		<< '\n' 
			<< constr.primary	<< '\n' 
			<< constr.foreign;
}
template<typename out_stream, typename clause_container>
void chk_constraints(out_stream & msg, clause_container const & exp, clause_container const & rcv)
{
	if ( 
			(
				exp.constraints.size() == 0 || rcv.constraints.size() == 0 
			)
			&&
			exp.constraints.size() != rcv.constraints.size()
		)
	{
		msg << "\nExpected constraints size => " << exp.constraints.size() << "\nReceived constraints size => " << rcv.constraints.size() << '\n';
		return;
	}
	if (exp.constraints.size() != rcv.constraints.size() || exp.constraints.size() && !std::equal(exp.constraints.cbegin(), exp.constraints.cend(), rcv.constraints.cbegin())) 
	{
		auto mismatch_pair = std::mismatch( exp.constraints.cbegin(), exp.constraints.cend(), rcv.constraints.cbegin() );
		msg << "A difference in constraints.\nThe expected constraint:\n" <<		
			(
				valid(mismatch_pair.first, exp.constraints) ? *mismatch_pair.first : constraint_container()
			) << "\n, " << "the received constraint:\n" <<		
			(
				valid(mismatch_pair.second, rcv.constraints) ? *mismatch_pair.second : constraint_container()
			);
	}
}
std::ostream & operator<<(std::ostream & os, mms::satellite_data_item_container const & satellite_item)
{
	return os << satellite_item.first << ',' << satellite_item.second;
}
std::ostream & operator<<(std::ostream & os, mms::column_and_satellite_data_item_container const & satellite_col_and_item)
{
	return os << satellite_col_and_item.first << ",[" << satellite_col_and_item.second << ']';
}
template<typename out_stream, typename clause_container>
void chk_satellite_data(out_stream & msg, clause_container const & exp, clause_container const & rcv)
{
	if ( 
			(
				exp.satellite_data.size() == 0 || rcv.satellite_data.size() == 0 
			)
			&&
			exp.satellite_data.size() != rcv.satellite_data.size()
		)
	{
		msg << "Expected satellite data size => " << exp.satellite_data.size() << "\nReceived satellite data size => " << rcv.satellite_data.size() << '\n';
		return;
	}
	if (exp.satellite_data.size() != rcv.satellite_data.size() || exp.satellite_data.size() && !std::equal(exp.satellite_data.cbegin(), exp.satellite_data.cend(), rcv.satellite_data.cbegin())) 
	{
		if ( exp.satellite_data.size() == 0 || rcv.satellite_data.size() == 0 )
		{
			msg << "Expected satellite data size => " << exp.satellite_data.size() << "\nReceived satellite data size => " << rcv.satellite_data.size() << '\n';
			return;
		}
		auto mismatch_pair = std::mismatch( exp.satellite_data.cbegin(), exp.satellite_data.cend(), rcv.satellite_data.cbegin() );
		msg << "A difference in satellite data.\nThe expected data:\n" <<		
			(
				valid(mismatch_pair.first, exp.satellite_data) ? *mismatch_pair.first : column_and_satellite_data_item_container()
			) << "\n, " << "the received data:\n" <<		
			(
				valid(mismatch_pair.second, rcv.satellite_data) ? *mismatch_pair.second : column_and_satellite_data_item_container()
			);
	}
}
boost::test_tools::predicate_result compare_create_clauses(mms::create_clause_container const & exp, mms::create_clause_container const & rcv)
{
	boost::test_tools::predicate_result r( true );
	std::ostringstream msg;
	chk_columns(msg,exp,rcv);
	chk_types(msg,exp,rcv);
	chk_constraints(msg,exp,rcv);
	chk_satellite_data(msg,exp,rcv);
	if ( msg.str().size() ) 
	{
		r.message() << msg.str();
		r = false;
	}
   	return r;
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
// UPDATE clause
boost::test_tools::predicate_result compare_update_clauses(mms::update_clause_container const & exp, mms::update_clause_container const & rcv)
{
	boost::test_tools::predicate_result r( true );
	std::ostringstream msg;
	chk_columns(msg,exp,rcv);
	chk_values(msg,exp,rcv);
	chk_filters(msg,exp,rcv);
	if ( msg.str().size() ) 
	{
		r.message() << msg.str();
		r = false;
	}
   	return r;
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
// DELETE clause
boost::test_tools::predicate_result compare_delete_clauses(mms::delete_clause_container const & exp, mms::delete_clause_container const & rcv)
{
	boost::test_tools::predicate_result r( true );
	std::ostringstream msg;
	chk_filters(msg,exp,rcv);
	if ( msg.str().size() ) 
	{
		r.message() << msg.str();
		r = false;
	}
   	return r;
}


BOOST_AUTO_TEST_SUITE( parsers )

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
// SELECT !!! TEST !!!
///////////////////////////////////////////////////////////////////////////////////
BOOST_FIXTURE_TEST_SUITE( select, select_fxt )
BOOST_AUTO_TEST_CASE( basic )
{
	reset();
	q = "[\
		\"action\" : \"select\",\
		\"distinct\" : false,\
		\"columns\" : [ \"bosom\", \"Angelina\", \"Angela\", \"Mary\"],\
		\"aggregates\" : { \"Angelina\" : 3, \"Angela\" : 4, \"Mary\" : 0 },\
		\"filter\" : {\
						\"hip\" : { \"interval\" : { 2 : 87, 3 : 91.5, \"logic\" : 1 } },\
						\"waist\" : { \"in\" : [57.5, 62] }\
					 },\
		\"groups\" : [ \"bosom\" ],\
		\"group_filter\" : { \"bosom\" : { \"interval\" : { 2 : 87, 3 : 91.5, \"logic\" : 1 } } },\
		\"order\" : [ [ \"bosom\" ], 0 ],\
		\"limit\" : [ 0, 100 ],\
		\"map\" : [\"Angelina\",\"Angelina_data\"]\
	    ]";
	mms::interval intrv = {2,87,3,91.5,1};
	exp.distinct = false, 
	exp.columns = (
				boost::assign::list_of("bosom"),"Angelina","Angela","Mary"
		      ).operator columns_container(),
	exp.aggregates = (
				boost::assign::list_of<column_and_aggregate>
				(
					std::make_pair("Angelina",static_cast<sql_faggregate>(3))
				),
				std::make_pair("Angela",static_cast<sql_faggregate>(4)),
				std::make_pair("Mary",static_cast<sql_faggregate>(0))
			 ).operator column_and_aggregate_container(), 
	exp.filters = (
				boost::assign::list_of<column_to_filter>
				(
					std::make_pair("hip",intrv)
				),
				std::make_pair(
						"waist",
						(::boost::assign::list_of<value_t>(57.5),62).operator set()
					      )
		      ).operator column_to_filter_container(), 
	exp.groups = boost::assign::list_of("bosom").operator columns_container(), 
	exp.order_options.columns = boost::assign::list_of("bosom").operator columns_container(), 
	exp.order_options.order = 0,
	exp.group_filters = boost::assign::map_list_of<column_name_t,column_filter>("bosom",intrv).operator column_to_filter_container(),
	exp.limit_options = std::make_pair(0,100),
	exp.map = boost::assign::map_list_of<std::string,std::string>("Angelina","Angelina_data").operator col_and_ref_col_container();
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));	
	auto r = compare_select_clauses(exp,rcv);
	BOOST_CHECK(r);
	
}
BOOST_AUTO_TEST_CASE( invalid_action )
{
	reset();
	q = 	
	"[\
		\"action\" : \"select?\",\
		\"distinct\" : false,\
		\"columns\" : [ \"bosom\", \"Angelina\", \"Angela\", \"Mary\"],\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);	
}
BOOST_AUTO_TEST_CASE( invalid_columns )
{
	reset();
	q = 	
	"[\
		\"action\" : \"select?\",\
		\"distinct\" : false,\
		\"columns\" : [ \"\"bosom\", \"Angelina\", \"Angela\", \"Mary\"],\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);	
}
BOOST_AUTO_TEST_CASE( invalid_aggregates )
{
	reset();
	q = "[\
		\"action\" : \"select\",\
		\"distinct\" : false,\
		\"columns\" : [ \"bosom\", \"Angelina\", \"Angela\", \"Mary\"],\
		\"aggregates\" : { \"Angelina\" : 30, \"Angela\" : 4, \"Mary\" : 0 },\
		\"filter\" : {\
						\"hip\" : { \"interval\" : { 2 : 87, 3 : 91.5, \"logic\" : 1 } },\
						\"waist\" : { \"in\" : [57.5, 62] }\
					 },\
		\"groups\" : [ \"bosom\" ],\
		\"group_filter\" : { \"bosom\" : { \"interval\" : { 2 : 87, 3 : 91.5, \"logic\" : 1 } } },\
		\"order\" : [ [ \"bosom\" ], 0 ],\
		\"limit\" : [ 0, 100 ]\
	    ]";	
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
}
BOOST_AUTO_TEST_CASE( invalid_filter )
{
	reset();
	q = "[\
		\"action\" : \"select\",\
		\"distinct\" : false,\
		\"columns\" : [ \"bosom\", \"Angelina\", \"Angela\", \"Mary\"],\
		\"aggregates\" : { \"Angelina\" : 3, \"Angela\" : 4, \"Mary\" : 0 },\
		\"filter\" : {\
						\"hip\" : { \"interval\" :  2 : 87, 3 : 91.5, \"logic\" : 1 } },\
						\"waist\" : { \"in\" : [57.5, 62] }\
					 },\
		\"groups\" : [ \"bosom\" ],\
		\"group_filter\" : { \"bosom\" : { \"interval\" : { 2 : 87, 3 : 91.5, \"logic\" : 1 } } },\
		\"order\" : [ [ \"bosom\" ], 0 ],\
		\"limit\" : [ 0, 100 ]\
	    ]";	
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
}
BOOST_AUTO_TEST_CASE( invalid_groups )
{
	reset();
	q = "[\
		\"action\" : \"select\",\
		\"distinct\" : false,\
		\"columns\" : [ \"bosom\", \"Angelina\", \"Angela\", \"Mary\"],\
		\"aggregates\" : { \"Angelina\" : 3, \"Angela\" : 4, \"Mary\" : 0 },\
		\"filter\" : {\
						\"hip\" : { \"interval\" : { 2 : 87, 3 : 91.5, \"logic\" : 1 } },\
						\"waist\" : { \"in\" : [57.5, 62] }\
					 },\
		\"groups\" : [ \"bosom\", ],\
		\"group_filter\" : { \"bosom\" : { \"interval\" : { 2 : 87, 3 : 91.5, \"logic\" : 1 } } },\
		\"order\" : [ [ \"bosom\" ], 0 ],\
		\"limit\" : [ 0, 100 ]\
	    ]";	
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);	
}
BOOST_AUTO_TEST_CASE( invalid_groups_filter )
{
	reset();
	q = "[\
		\"action\" : \"select\",\
		\"distinct\" : false,\
		\"columns\" : [ \"bosom\", \"Angelina\", \"Angela\", \"Mary\"],\
		\"aggregates\" : { \"Angelina\" : 3, \"Angela\" : 4, \"Mary\" : 0 },\
		\"filter\" : {\
						\"hip\" : { \"interval\" : { 2 : 87, 3 : 91.5, \"logic\" : 1 } },\
						\"waist\" : { \"in\" : [57.5, 62] }\
					 },\
		\"groups\" : [ \"bosom\" ],\
		\"group_filter\" : { \"bosom\" : { \"interval\" : { 2 : peach, 3 : 91.5, \"logic\" : 1 } } },\
		\"order\" : [ [ \"bosom\" ], 0 ],\
		\"limit\" : [ 0, 100 ]\
	    ]";	
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);	
}
BOOST_AUTO_TEST_CASE( invalid_order )
{
	reset();
	q = "[\
		\"action\" : \"select\",\
		\"distinct\" : false,\
		\"columns\" : [ \"bosom\", \"Angelina\", \"Angela\", \"Mary\"],\
		\"aggregates\" : { \"Angelina\" : 3, \"Angela\" : 4, \"Mary\" : 0 },\
		\"filter\" : {\
						\"hip\" : { \"interval\" : { 2 : 87, 3 : 91.5, \"logic\" : 1 } },\
						\"waist\" : { \"in\" : [57.5, 62] }\
					 },\
		\"groups\" : [ \"bosom\" ],\
		\"group_filter\" : { \"bosom\" : { \"interval\" : { 2 : 87, 3 : 91.5, \"logic\" : 1 } } },\
		\"order\" : [ [ \"bosom\" ], 2 ],\
		\"limit\" : [ 0, 100 ]\
	    ]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
}
BOOST_AUTO_TEST_CASE( invalid_limit )
{
	reset();
	q = "[\
		\"action\" : \"select\",\
		\"distinct\" : false,\
		\"columns\" : [ \"bosom\", \"Angelina\", \"Angela\", \"Mary\"],\
		\"aggregates\" : { \"Angelina\" : 3, \"Angela\" : 4, \"Mary\" : 0 },\
		\"filter\" : {\
						\"hip\" : { \"interval\" : { 2 : 87, 3 : 91.5, \"logic\" : 1 } },\
						\"waist\" : { \"in\" : [57.5, 62] }\
					 },\
		\"groups\" : [ \"bosom\" ],\
		\"group_filter\" : { \"bosom\" : { \"interval\" : { 2 : 87, 3 : 91.5, \"logic\" : 1 } } },\
		\"order\" : [ [ \"bosom\" ], 0 ],\
		\"limit\" : [ 0 100 ]\
	    ]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
}
BOOST_AUTO_TEST_CASE( invalid_map )
{
	reset();
	q = "[\
		\"action\" : \"select\",\
		\"distinct\" : false,\
		\"columns\" : [ \"bosom\", \"Angelina\", \"Angela\", \"Mary\"],\
		\"aggregates\" : { \"Angelina\" : 3, \"Angela\" : 4, \"Mary\" : 0 },\
		\"filter\" : {\
						\"hip\" : { \"interval\" : { 2 : 87, 3 : 91.5, \"logic\" : 1 } },\
						\"waist\" : { \"in\" : [57.5, 62] }\
					 },\
		\"groups\" : [ \"bosom\" ],\
		\"group_filter\" : { \"bosom\" : { \"interval\" : { 2 : 87, 3 : 91.5, \"logic\" : 1 } } },\
		\"order\" : [ [ \"bosom\" ], 0 ],\
		\"limit\" : [ 0, 100 ],\
		\"map\" : [\"Angelina\";\"Angelina_data\"]\
	    ]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
}
BOOST_AUTO_TEST_CASE( required_options )
{
	reset();
	q = 	
	"[\
		\"action\" : \"select\",\
		\"distinct\" : false,\
		\"columns\" : [ \"bosom\", \"Angelina\", \"Angela\", \"Mary\"],\
	]";
	exp.distinct = false;
	exp.columns = boost::assign::list_of("bosom")("Angelina")("Angela")("Mary").operator columns_container();
	//auto f(cbegin(q)), l(cend(q));
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	auto r = compare_select_clauses(exp,rcv);
	BOOST_CHECK(r);
	
}
BOOST_AUTO_TEST_CASE( select_subject_type )
{
	reset();
	q = 
	"[\
		\"action\" : \"select\",\
		\"distinct\" : false,\
		 \"columns\" : [\"subject_type_id\", \"subject_type_name\"],\
		\"order\": [ [\"subject_type_name\"], 0],\
		\"limit\" : [0, 10]\
	]";
	exp.distinct = false, exp.columns = boost::assign::list_of("subject_type_id")("subject_type_name").operator mms::columns_container(),
		exp.order_options.columns = boost::assign::list_of("subject_type_name").operator mms::columns_container(), exp.order_options.order = 0,
		exp.limit_options = std::make_pair(0,10);
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	auto r = compare_select_clauses(exp,rcv);
	BOOST_CHECK(r);
	
}
BOOST_AUTO_TEST_CASE( select_subject_type_l1 )
{
	reset();
	q = 
	"[\
		\"action\" : \"select\",\
		\"distinct\" : false,\
		 \"columns\" : [\"subject_type_l1_id\", \"subject_type_l1_name\"],\
		\"order\": [ [\"subject_type_l1_name\"], 0],\
		\"limit\" : [0, 10]\
	]";
	exp.distinct = false, exp.columns = boost::assign::list_of("subject_type_l1_id")("subject_type_l1_name").operator mms::columns_container(),
		exp.order_options.columns = boost::assign::list_of("subject_type_l1_name").operator mms::columns_container(), exp.order_options.order = 0,
		exp.limit_options = std::make_pair(0,10);
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	auto r = compare_select_clauses(exp,rcv);
	BOOST_CHECK(r);
	
}
BOOST_AUTO_TEST_CASE( panjan_test )
{
#define PANJAN_TEST \
{\
	it f = q.cbegin(), l = q.cend();\
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));\
	auto r = compare_select_clauses(exp,rcv);\
	BOOST_CHECK(r);\
	\
	reset();\
}
	reset();
	q = "[\
		\"action\" : \"select\",\
		\"distinct\" : false,\
		 \"columns\" : [\"subject_id\", \"subject_first_name\", \"subject_country\", \"subject_dominant_arm\"],\
		\"order\": [ [\"subject_first_name\"], 0],\
		\"limit\" : [0, 10]\
	     ]";
	exp.distinct = false, exp.columns = (boost::assign::list_of("subject_id"),"subject_first_name","subject_country","subject_dominant_arm").operator columns_container(),
		exp.order_options.columns = boost::assign::list_of("subject_first_name").operator mms::columns_container(), exp.order_options.order = 0,
		exp.limit_options = std::make_pair(0,10);
	PANJAN_TEST
	q = "[\
		\"action\" : \"select\",\
		\"distinct\" : true,\
		 \"columns\" : [\"subject_id\"]\
		\"aggregates\" : { \"subject_id\" : 1}\
	     ]";
	exp.distinct = true, exp.columns = boost::assign::list_of("subject_id").operator columns_container(),
		exp.aggregates = boost::assign::map_list_of<mms::column_name_t, mms::sql_faggregate>("subject_id",static_cast<mms::sql_faggregate>(1)).operator mms::column_and_aggregate_container();
	PANJAN_TEST
	q = "[\
		\"action\" : \"select\",\
		\"distinct\" : false,\
		\"columns\" : [\"project_id\", \"visit_id\", \"subject_id\", \"subject_first_name\", \"subject_country\"], \
		\"order\": [ [\"subject_first_name\"], 0],\
		\"limit\" : [10, 100]\
	    ]";
	exp.distinct = false, exp.columns = (boost::assign::list_of("project_id"),"visit_id","subject_id","subject_first_name","subject_country").operator mms::columns_container(),
		exp.order_options.columns = boost::assign::list_of("subject_first_name").operator mms::columns_container(), exp.order_options.order = 0,
		exp.limit_options = std::make_pair(10,100);
	PANJAN_TEST
	q = "[\
		\"action\" : \"select\",\
		\"distinct\" : false,\
		 \"columns\" : [\"project_id\", \"visit_id\", \"subject_id\", \"subject_first_name\", \"subject_country\", \"subject_dominant_arm\"],\
		 \"filter\" : { \"visit_id\" : { \"IN\" : [1] } },\
		 \"limit\" : [0, 100]\
	    ]";
	exp.distinct = false, exp.columns = (boost::assign::list_of("project_id"),"visit_id","subject_id","subject_first_name","subject_country","subject_dominant_arm").operator mms::columns_container(),
		exp.filters = boost::assign::map_list_of("visit_id",boost::assign::list_of<mms::value_t>(1).operator mms::set()).operator mms::column_to_filter_container();
		exp.limit_options = std::make_pair(0,100);
	PANJAN_TEST
	q = "[\
		\"action\" : \"select\",\
		\"distinct\" : false,\
		 \"columns\" : [\"project_id\", \"subject_id\", \"subject_first_name\", \"subject_country\", \"subject_dominant_arm\"],\
		 \"filter\" : { \"project_id\" : { \"IN\" : [1] } },\
		 \"limit\" : [0, 100]\
	    ]";
	exp.distinct = false, exp.columns = (boost::assign::list_of("project_id"),"subject_id","subject_first_name","subject_country","subject_dominant_arm").operator mms::columns_container(),
		exp.filters = boost::assign::map_list_of("project_id",boost::assign::list_of<mms::value_t>(1).operator mms::set()).operator mms::column_to_filter_container(), 
		exp.limit_options = std::make_pair(0,100);
	PANJAN_TEST
	q = "[\
		\"action\" : \"select\",\
		\"distinct\" : false,\
		 \"columns\" : [\"project_id\", \"subject_id\"],\
		\"aggregates\" : { \"subject_id\" : 1},\
		\"filter\" : { \"project_id\" : { \"IN\" : [1, 2, 3] } },\
		\"groups\" : [\"project_id\"],\
		 \"limit\" : [0, 100]\
	    ]";
	exp.distinct = false, 
	exp.columns = (boost::assign::list_of("project_id"),"subject_id").operator mms::columns_container(),
	exp.aggregates = boost::assign::map_list_of<mms::column_name_t, mms::sql_faggregate>("subject_id",static_cast<mms::sql_faggregate>(1)).operator mms::column_and_aggregate_container(), 
	exp.filters = boost::assign::map_list_of("project_id",(boost::assign::list_of<mms::value_t>(1),2,3).operator mms::set()).operator mms::column_to_filter_container(), 
	exp.groups = boost::assign::list_of("project_id").operator mms::columns_container(), 
	exp.limit_options = std::make_pair(0,100);
	PANJAN_TEST
	q = "[\
		\"action\" : \"select\",\
		\"distinct\" : false,\
		\"columns\" : [\"country_id\", \"country_printable_name\"],\
		\"order\": [ [\"country_printable_name\"], 0],\
	     ]";
	exp.distinct = false, 
	exp.columns = (boost::assign::list_of("country_id"),"country_printable_name").operator mms::columns_container(),
	exp.order_options.columns = boost::assign::list_of("country_printable_name").operator mms::columns_container(), 
	exp.order_options.order = 0;
	PANJAN_TEST
}
BOOST_AUTO_TEST_SUITE_END() // select

////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
// INSERT !!! TEST !!!
///////////////////////////////////////////////////////////////////////////////////
BOOST_FIXTURE_TEST_SUITE( insert, insert_fxt )
BOOST_AUTO_TEST_CASE( basic )
{
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [\"subject_type_id\", \"subject_type_name\"],\
		\"values\": [ \"subject_type_name\", 0, false, 99.999],\
		\"default_all\" : true\
	]";
	exp.default_all = true, 
	exp.columns = boost::assign::list_of("subject_type_id")("subject_type_name").operator mms::columns_container(),
	exp.values = boost::assign::list_of<mms::insert_value_t>(std::string("subject_type_name"))(0)(false)(99.999).operator mms::insert_values_container();
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	auto r = compare_insert_clauses(exp,rcv);
	BOOST_CHECK(r);
}
BOOST_AUTO_TEST_SUITE( columns )
BOOST_AUTO_TEST_CASE( non_string )
{
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [1, \"subject_type_name\"],\
		\"values\": [ \"subject_type_name\", 0, false, 99.999],\
		\"default_all\" : true\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
} // non_string
BOOST_AUTO_TEST_CASE( empty )
{
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [],\
		\"values\": [ \"subject_type_name\", 0, false, 99.999],\
		\"default_all\" : true\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
} // empty
BOOST_AUTO_TEST_CASE( wrong_keyword )
{
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns!\" : [\"it_will_not_be_parsed\"],\
		\"values\": [ \"subject_type_name\", 0, false, 99.999],\
		\"default_all\" : true\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
} // wrong_keyword
BOOST_AUTO_TEST_SUITE_END() // columns
BOOST_AUTO_TEST_SUITE( values )
BOOST_AUTO_TEST_CASE( empty )
{
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [\"subject_type_name\"],\
		\"values\": [],\
		\"default_all\" : true\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	exp.default_all = true;
	exp.columns.push_back("subject_type_name");
	auto r = compare_insert_clauses(exp,rcv);
	BOOST_CHECK(r);
} // empty
BOOST_AUTO_TEST_CASE( no_values )
{
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [\"subject_type_id\", \"subject_type_name\"],\
		\"default_all\" : true\
	]";
	exp.default_all = true, 
	exp.columns = boost::assign::list_of("subject_type_id")("subject_type_name").operator mms::columns_container();
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	auto r = compare_insert_clauses(exp,rcv);
	BOOST_CHECK(r);
} // no_values
BOOST_AUTO_TEST_CASE( wrong_keyword )
{
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [\"\"],\
		\"values!\" : [\"it_will_not_be_parsed\"],\
		\"default_all\" : true\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
} // wrong_keyword
BOOST_AUTO_TEST_SUITE_END() // values
BOOST_AUTO_TEST_SUITE( default_all )
BOOST_AUTO_TEST_CASE( empty )
{
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [\"\"],\
		\"values\" : [\"it_will_not_be_parsed\"],\
		\"default_all\" : \
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
} // empty
BOOST_AUTO_TEST_CASE( no_default_all )
{
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [\"\"],\
		\"values\" : [\"it_will_not_be_parsed\"]\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	exp.columns.push_back( "" );
	exp.values.push_back( std::string( "it_will_not_be_parsed" ) );
	auto r = compare_insert_clauses(exp,rcv);
	BOOST_CHECK(r);
	rcv = mms::insert_clause_container();
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [\"\"]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	exp.values.pop_back();
	r = compare_insert_clauses(exp,rcv);
	BOOST_CHECK(r);
} // no_default_all
BOOST_AUTO_TEST_CASE( non_bool )
{
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [\"\"],\
		\"values\" : [\"it_will_not_be_parsed\"],\
		\"default_all\" : 1\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [\"\"],\
		\"values\" : [\"it_will_not_be_parsed\"],\
		\"default_all\" : 1.0\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [\"\"],\
		\"values\" : [\"it_will_not_be_parsed\"],\
		\"default_all\" : 1.0\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [\"\"],\
		\"values\" : [\"it_will_not_be_parsed\"],\
		\"default_all\" : \"argh\"\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
} // non_bool
BOOST_AUTO_TEST_SUITE_END() // default_all
BOOST_AUTO_TEST_CASE( invalid_grammar)
{
	// colon
	q = 
	"[\
		\"action\"  \"insert\",\
		 \"columns\" : [\"subject_type_id\", \"subject_type_name\"],\
		\"values\": [ \"subject_type_name\", 0, false, 99.999],\
		\"default_all\" : true\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\"  [\"subject_type_id\", \"subject_type_name\"],\
		\"values\": [ \"subject_type_name\", 0, false, 99.999],\
		\"default_all\" : true\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [\"subject_type_id\", \"subject_type_name\"],\
		\"values\" [ \"subject_type_name\", 0, false, 99.999],\
		\"default_all\" : true\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [\"subject_type_id\", \"subject_type_name\"],\
		\"values\": [ \"subject_type_name\", 0, false, 99.999],\
		\"default_all\"  true\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// opened square bracket
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : \"subject_type_id\", \"subject_type_name\"],\
		\"values\": [ \"subject_type_name\", 0, false, 99.999],\
		\"default_all\" : true\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [[\"subject_type_id\", \"subject_type_name\"],\
		\"values\": [ \"subject_type_name\", 0, false, 99.999],\
		\"default_all\" : true\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [\"subject_type_id\", \"subject_type_name\"],\
		\"values\":  \"subject_type_name\", 0, false, 99.999],\
		\"default_all\" : true\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [\"subject_type_id\", \"subject_type_name\"],\
		\"values\": [[ \"subject_type_name\", 0, false, 99.999],\
		\"default_all\" : true\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"\
		\"action\" : \"insert\",\
		 \"columns\" : [\"subject_type_id\", \"subject_type_name\"],\
		\"values\": [ \"subject_type_name\", 0, false, 99.999],\
		\"default_all\" : true\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// closed square bracket
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [\"subject_type_id\", \"subject_type_name\"],\
		\"values\": [ \"subject_type_name\", 0, false, 99.999],\
		\"default_all\" : true\
	";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [\"subject_type_id\", \"subject_type_name\",\
		\"values\": [ \"subject_type_name\", 0, false, 99.999],\
		\"default_all\" : true\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [\"subject_type_id\", \"subject_type_name\"],\
		\"values\": [ \"subject_type_name\", 0, false, 99.999,\
		\"default_all\" : true\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// 'after-action' comma
	q = 
	"[\
		\"action\" : \"insert\"\
		 \"columns\" : [\"subject_type_id\", \"subject_type_name\"],\
		 \"values\": [ \"subject_type_name\", 0, false, 99.999,\
		\"default_all\" : true\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception)
	// 'after-columns' comma
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [\"subject_type_id\", \"subject_type_name\"]\
		 \"values\": [ \"subject_type_name\", 0, false, 99.999],\
		\"default_all\" : true\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception)
	// 'after-values' comma
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [\"subject_type_id\", \"subject_type_name\"],\
		 \"values\": [ \"subject_type_name\", 0, false, 99.999]\
		\"default_all\" : true\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception)
	// action value
	q = 
	"[\
		\"action\" : \
		\"columns\"\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception)
	// no action
	q = 
	"[\
		 \"columns\" : [\"subject_type_id\", \"subject_type_name\"],\
		 \"values\": [ \"subject_type_name\", 0, false, 99.999],\
		\"default_all\" : true\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// action: excessive comma
	q = 
	"[\
		\"action\" : \"insert\",,\
		 \"columns\" : [\"subject_type_id\", \"subject_type_name\"],\
		 \"values\": [ \"subject_type_name\", 0, false, 99.999],\
		\"default_all\" : true\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// action: no comma
	q = 
	"[\
		\"action\" : \"insert\"\
		 \"columns\" : [\"subject_type_id\", \"subject_type_name\"],\
		 \"values\": [ \"subject_type_name\", 0, false, 99.999],\
		\"default_all\" : true\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// values: no comma
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [\"subject_type_id\", \"subject_type_name\"],\
		 \"values\": [ \"subject_type_name\", 0, false, 99.999]\
		\"default_all\" : true\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [\"subject_type_id\", \"subject_type_name\"],\
		 \"values\": [ \"subject_type_name\" 0, false, 99.999],\
		\"default_all\" : true\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// values: excessive comma
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [\"subject_type_id\", \"subject_type_name\"],\
		 \"values\": [ \"subject_type_name\", 0, false, 99.999],,\
		\"default_all\" : true\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [\"subject_type_id\", \"subject_type_name\"],\
		 \"values\": [ \"subject_type_name\",, 0, false, 99.999],\
		\"default_all\" : true\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// columns: excessive comma
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [\"subject_type_id\", \"subject_type_name\"],,\
		 \"values\": [ \"subject_type_name\", 0, false, 99.999],\
		\"default_all\" : true\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [\"subject_type_id\",, \"subject_type_name\"],\
		 \"values\": [ \"subject_type_name\", 0, false, 99.999],\
		\"default_all\" : true\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// columns: no comma
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [\"subject_type_id\", \"subject_type_name\"]\
		 \"values\": [ \"subject_type_name\", 0, false, 99.999],\
		\"default_all\" : true\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [\"subject_type_id\" \"subject_type_name\"],\
		 \"values\": [ \"subject_type_name\", 0, false, 99.999],\
		\"default_all\" : true\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
} // invalid grammar
BOOST_AUTO_TEST_CASE( valid_grammar )
{
	q = 
	"[\
		\"action\" : \"insert\",\
		\"columns\" : [\"subject_type_id\", \"subject_type_name\"]],\
		\"values\": [ \"subject_type_name\", 0, false, 99.999],\
		\"default_all\" : true\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	q = 
	"[\
		\"action\" : \"insert\",\
		 \"columns\" : [\"subject_type_id\", \"subject_type_name\"],\
		 \"values\": [ \"subject_type_name\", 0, false, 99.999]],\
		\"default_all\" : true\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
} // valid_grammar
BOOST_AUTO_TEST_SUITE_END() // insert

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
// CREATE !!! TEST !!!
///////////////////////////////////////////////////////////////////////////////////
BOOST_FIXTURE_TEST_SUITE( create, create_fxt )
BOOST_AUTO_TEST_CASE( basic )
{
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 2, 1, 3],\
		\"constraints\" : [{\"null\":false,\"primary\":true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	exp.columns = (
					::boost::assign::list_of
												("car_shorthand"),
												"car_class",
												"car_body",
												"car_engine",
												"car_chassis"
				 ).operator mms::columns_container(),
	exp.types = (::boost::assign::list_of<sql_data_type_wrapper>(3),0,2,1,3).operator mms::sql_types_container(),
	exp.constraints = 
						boost::assign::list_of<constraint_container>(
																		constraint_container(
																								false, // NULL
																								true // PRIMARY
																							) // DEFAULT value is not used
																	).operator mms::constraints_container(),
	exp.satellite_data = (
							::boost::assign::pair_list_of<std::string,mms::satellite_data_item_container>(
																										"car_shorthand",
																										mms::satellite_data_item_container(16,"")
																									)
																									(
																										"car_body",
																										mms::satellite_data_item_container(20,"")
																									)
																									(
																										"car_chassis",
																										mms::satellite_data_item_container(20,"")
																									)
						 ).operator mms::columns_satellite_data_container();
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	auto r = compare_create_clauses(exp,rcv);
	BOOST_CHECK(r);
	//
} // basic

BOOST_AUTO_TEST_SUITE( optionals )
BOOST_AUTO_TEST_CASE( with_no_optionals )
{
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 2, 0, 2, 3, 1]\
	]";
	exp.columns = (
					::boost::assign::list_of
												("car_shorthand"),
												"car_class",
												"car_body",
												"car_engine",
												"car_chassis"
				 ).operator mms::columns_container(),
	exp.types = (::boost::assign::list_of<mms::sql_data_type_wrapper>(2),0,2,3,1).operator mms::sql_types_container();
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	auto r = compare_create_clauses(exp,rcv);
	BOOST_CHECK(r); // logs a message if present
	//reset();
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 1, 0, 2, 3, 1],\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
} // with_no_optionals
BOOST_AUTO_TEST_CASE( only_varchar_is_present )
{
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	exp.columns = (
					::boost::assign::list_of
												("car_shorthand"),
												"car_class",
												"car_body",
												"car_engine",
												"car_chassis"
				 ).operator mms::columns_container(),
	exp.types = (::boost::assign::list_of<sql_data_type_wrapper>(3),0,3,1,3).operator mms::sql_types_container(),
	exp.satellite_data = (
							::boost::assign::pair_list_of<std::string,mms::satellite_data_item_container>(
																										"car_shorthand",
																										mms::satellite_data_item_container(16,"")
																									)
																									(
																										"car_body",
																										mms::satellite_data_item_container(20,"")
																									)
																									(
																										"car_chassis",
																										mms::satellite_data_item_container(20,"")
																									)
						 ).operator mms::columns_satellite_data_container();
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	auto r = compare_create_clauses(exp,rcv);
	BOOST_CHECK(r);
	//
} // only_varchar_is_present
BOOST_AUTO_TEST_CASE( foreign_is_present )
{
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 2, 0, 2, 1, 2],\
		\"constraints\" : [{\"foreign\":true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[\"synonym_car_shorthand\"]}]\
	]";
	exp.columns = (
					::boost::assign::list_of
												("car_shorthand"),
												"car_class",
												"car_body",
												"car_engine",
												"car_chassis"
				 ).operator mms::columns_container(),
	exp.types = (::boost::assign::list_of<mms::sql_data_type_wrapper>(2),0,2,1,2).operator mms::sql_types_container(),
	exp.constraints = 
						boost::assign::list_of<mms::constraint_container>(
																		mms::constraint_container(
																								false, // NULL
																								false, // PRIMARY
																								true // FOREIGN
																							) // DEFAULT value is not used,
																	).operator mms::constraints_container(),
	exp.satellite_data = ::boost::assign::pair_list_of<std::string,mms::satellite_data_item_container>(
																										"car_shorthand",
																										mms::satellite_data_item_container(0,"synonym_car_shorthand")
																								 ).operator mms::columns_satellite_data_container();
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	auto r = compare_create_clauses(exp,rcv);
	BOOST_CHECK(r);
	//
} // foreign_is_present
BOOST_AUTO_TEST_CASE( varchar_and_foreign_are_present )
{
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"foreign\":true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	exp.columns = (
					::boost::assign::list_of
												("car_shorthand"),
												"car_class",
												"car_body",
												"car_engine",
												"car_chassis"
				 ).operator mms::columns_container(),
	exp.types = (::boost::assign::list_of<mms::sql_data_type_wrapper>(3),0,3,1,3).operator mms::sql_types_container(),
	exp.constraints = 
						boost::assign::list_of<mms::constraint_container>(
																		mms::constraint_container(
																								false, // NULL
																								false, // PRIMARY
																								true // FOREIGN
																							) // DEFAULT value is not used,
																	).operator mms::constraints_container(),
	exp.satellite_data = (
							::boost::assign::pair_list_of<std::string,mms::satellite_data_item_container>(
																										"car_shorthand",
																										mms::satellite_data_item_container(16,"synonym_car_shorthand")
																									)
																									(
																										"car_body",
																										mms::satellite_data_item_container(20,"")
																									)
																									(
																										"car_chassis",
																										mms::satellite_data_item_container(20,"")
																									)
						 ).operator mms::columns_satellite_data_container();
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	auto r = compare_create_clauses(exp,rcv);
	BOOST_CHECK(r);
	//
} // varchar_and_foreign_are_present
BOOST_AUTO_TEST_SUITE_END() // optionals

// constraints' tests
BOOST_AUTO_TEST_SUITE( constraints )
BOOST_AUTO_TEST_CASE( constraints_default_set )
{
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"Default\" : 5.0,\"foreign\":true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	exp.columns = (
					::boost::assign::list_of
												("car_shorthand"),
												"car_class",
												"car_body",
												"car_engine",
												"car_chassis"
				 ).operator mms::columns_container(),
	exp.types = (::boost::assign::list_of<mms::sql_data_type_wrapper>(3),0,3,1,3).operator mms::sql_types_container(),
	exp.constraints = 
						boost::assign::list_of<mms::constraint_container>(
																		mms::constraint_container(
																								false, // NULL
																								false, // PRIMARY
																								true, // FOREIGN
																								5.0 // DEFAULT
																							) 
																	).operator mms::constraints_container(),
	exp.satellite_data = (
							::boost::assign::pair_list_of<std::string,mms::satellite_data_item_container>(
																										"car_shorthand",
																										mms::satellite_data_item_container(16,"synonym_car_shorthand")
																									)
																									(
																										"car_body",
																										mms::satellite_data_item_container(20,"")
																									)
																									(
																										"car_chassis",
																										mms::satellite_data_item_container(20,"")
																									)
						 ).operator mms::columns_satellite_data_container();
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	auto r = compare_create_clauses(exp,rcv);
	BOOST_CHECK(r);
	//
} // constraints_default_set
BOOST_AUTO_TEST_CASE( constraints_non_bool )
{
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"Default\" : 5.0,\"foreign\":1}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
} // constraints_non_bool
BOOST_AUTO_TEST_CASE( constraints_wrong_keyword )
{
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"Default!\" : 5.0,\"foreign\":true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"null!\" : true,\"foreign\":true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"primary!\" true,\"foreign\":true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"primary\" true,\"foreign!\":true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
} // constraints_wrong_keyword
BOOST_AUTO_TEST_CASE( all_valid_combinations )
{
	// default
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"Default\" : 5.0}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	exp.columns = (
					::boost::assign::list_of
												("car_shorthand"),
												"car_class",
												"car_body",
												"car_engine",
												"car_chassis"
				 ).operator mms::columns_container(),
	exp.types = (::boost::assign::list_of<mms::sql_data_type_wrapper>(3),0,3,1,3).operator mms::sql_types_container(),
	exp.constraints = 
						boost::assign::list_of<mms::constraint_container>(
																		mms::constraint_container(
																								false, // NULL
																								false, // PRIMARY
																								false, // FOREIGN
																								5.0 // DEFAULT
																							) 
																	).operator mms::constraints_container(),
	exp.satellite_data = (
							::boost::assign::pair_list_of<std::string,mms::satellite_data_item_container>(
																										"car_shorthand",
																										mms::satellite_data_item_container(16,"synonym_car_shorthand")
																									)
																									(
																										"car_body",
																										mms::satellite_data_item_container(20,"")
																									)
																									(
																										"car_chassis",
																										mms::satellite_data_item_container(20,"")
																									)
						 ).operator mms::columns_satellite_data_container();
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	auto r = compare_create_clauses(exp,rcv);
	BOOST_CHECK(r);
	// default,null
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"Default\" : 5.0,\"null\" : true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	rcv = mms::create_clause_container();
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	exp.constraints[0].null = true;
	r = compare_create_clauses(exp,rcv);
	BOOST_CHECK(r);
	// default, null, primary
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"Default\" : 5.0,\"null\" : true, \"primary\" : true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	rcv = create_clause_container();
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	exp.constraints[0].primary = true;
	r = compare_create_clauses(exp,rcv);
	BOOST_CHECK(r);
	// default, null, primary, foreign
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"Default\" : 5.0,\"null\" : true, \"primary\" : true, \"foreign\" : true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	rcv = create_clause_container();
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	exp.constraints[0].foreign = true;
	r = compare_create_clauses(exp,rcv);
	BOOST_CHECK(r);
	// default, null, primary
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"Default\" : 5.0,\"null\" : true, \"primary\" : true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	rcv = create_clause_container();
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	exp.constraints[0].foreign = false;
	r = compare_create_clauses(exp,rcv);
	BOOST_CHECK(r);
	// default, null, foreign
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"Default\" : 5.0,\"null\" : true, \"foreign\" : true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	rcv = create_clause_container();
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	exp.constraints[0].primary = false;
	exp.constraints[0].foreign = true;
	r = compare_create_clauses(exp,rcv);
	BOOST_CHECK(r);
	// default, primary
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"Default\" : 5.0, \"primary\" : true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	rcv = create_clause_container();
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	exp.constraints[0].primary = true;
	exp.constraints[0].null = exp.constraints[0].foreign = false;
	r = compare_create_clauses(exp,rcv);
	BOOST_CHECK(r);
	// default, primary, foreign
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"Default\" : 5.0, \"primary\" : true,\"foreign\" : true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	rcv = create_clause_container();
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	exp.constraints[0].foreign = true;
	r = compare_create_clauses(exp,rcv);
	BOOST_CHECK(r);
	// default, foreign
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"Default\" : 5.0,\"foreign\" : true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	rcv = create_clause_container();
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	exp.constraints[0].primary = false;
	r = compare_create_clauses(exp,rcv);
	BOOST_CHECK(r);
	// null
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"null\" : true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	rcv = create_clause_container();
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	exp.constraints[0].null = true;
	exp.constraints[0].foreign = false;
	exp.constraints[0].def_value = mms::not_used_tag::not_used;
	r = compare_create_clauses(exp,rcv);
	BOOST_CHECK(r);
	// null, primary
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"null\" : true, \"primary\" : true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	rcv = create_clause_container();
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	exp.constraints[0].primary = true;
	r = compare_create_clauses(exp,rcv);
	BOOST_CHECK(r);
	// null, primary, foreign
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"null\" : true, \"primary\" : true, \"foreign\" : true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	rcv = create_clause_container();
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	exp.constraints[0].foreign = true;
	r = compare_create_clauses(exp,rcv);
	BOOST_CHECK(r);
	// null, foreign
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"null\" : true, \"foreign\" : true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	rcv = create_clause_container();
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	exp.constraints[0].primary = false;
	r = compare_create_clauses(exp,rcv);
	BOOST_CHECK(r);
	// primary
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"primary\" : true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	rcv = create_clause_container();
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	exp.constraints[0].primary = true;
	exp.constraints[0].null = exp.constraints[0].foreign = false;
	r = compare_create_clauses(exp,rcv);
	BOOST_CHECK(r);
	// primary, foreign
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"primary\" : true,\"foreign\" : true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	rcv = create_clause_container();
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	exp.constraints[0].foreign = true;
	r = compare_create_clauses(exp,rcv);
	BOOST_CHECK(r);
	// foreign
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"foreign\" : true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	rcv = create_clause_container();
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	exp.constraints[0].primary = false;
	r = compare_create_clauses(exp,rcv);
	BOOST_CHECK(r);
	// empty
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	rcv = create_clause_container();
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	exp.constraints.pop_back();
	r = compare_create_clauses(exp,rcv);
	BOOST_CHECK(r);
} // all_valid_combinations
BOOST_AUTO_TEST_CASE( invalid_combinations )
{
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"foreign\":true, \"Default\" : 5.0}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"foreign\":true, \"primary\" : true, \"Default\" : 5.0}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"foreign\":true, \"primary\" : true, \"null\" : true, \"Default\" : 5.0}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"foreign\":true, \"null\" : true, \"Default\" : 5.0}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"foreign\":true, \"default\" : 5, \"primary\" : true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"foreign\":true, \"default\" : 5, \"null\" : true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"foreign\":true, \"null\" : true, \"primary\" : true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"foreign\":true, \"primary\" : true, \"null\" : true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"primary\":true, \"Default\" : 5.0}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"primary\":true, \"null\" : true, \"Default\" : 5.0}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"primary\":true, \"foreign\" : true, \"Default\" : 5.0}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"primary\":true, \"foreign\" : true, \"null\" : true, \"Default\" : 5.0}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"primary\":true, \"null\" : true, \"foreign\" : true, \"Default\" : 5.0}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"primary\":true, \"default\" : 5, \"foreign\" : true, \"null\" : true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"primary\":true, \"default\" : 5, \"null\" : true, \"foreign\" : true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"primary\":true, \"null\" : true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints\" : [{\"null\":true, \"default\" : 5}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
} // invalid_combinations
BOOST_AUTO_TEST_SUITE_END() // constraints

// types' tests
BOOST_AUTO_TEST_SUITE( types )
BOOST_AUTO_TEST_CASE( types_out_of_bound )
{
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 93, 0, 3, 1, 3],\
		\"constraints\" : [{\"Default\" : 5.0,\"foreign\":true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
} // types_out_of_bound
BOOST_AUTO_TEST_CASE( types_non_integer )
{
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 2, 0, 3, 1, 3.0],\
		\"constraints\" : [{\"Default\" : 5.0,\"foreign\":true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
} // types_non_integer
BOOST_AUTO_TEST_CASE( types_wrong_keyword )
{
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types!\": [ 2, 0, 3, 1, 3],\
		\"constraints\" : [{\"Default\" : 5.0,\"foreign\":true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
} // types_wrong_keyword
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( columns )
BOOST_AUTO_TEST_CASE( non_string )
{
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [3,\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 2, 0, 3, 1, 3],\
		\"constraints\" : [{\"Default\" : 5.0,\"foreign\":true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
} // non_string
BOOST_AUTO_TEST_CASE( wrong_keyword )
{
	q = 
	"[\
		\"action\" : \"create\",\
		\",columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 2, 0, 3, 1, 3],\
		\"constraints\" : [{\"Default\" : 5.0,\"foreign\":true}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]},{\"car_body\":[20]},{\"car_chassis\":[20]}]\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
} // wrong_keyword
BOOST_AUTO_TEST_SUITE_END() // types

// constraints satellite data tests
BOOST_AUTO_TEST_SUITE( constraints_sdata )
BOOST_AUTO_TEST_CASE( all_valid_combinations )
{
	// 2 parameteres
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"synonym_car_shorthand\"]}]\
	]";
	exp.columns = (
					::boost::assign::list_of
												("car_shorthand"),
												"car_class",
												"car_body",
												"car_engine",
												"car_chassis"
				 ).operator mms::columns_container(),
	exp.types = (::boost::assign::list_of<mms::sql_data_type_wrapper>(3),0,3,1,3).operator mms::sql_types_container(),
	exp.satellite_data = (
							::boost::assign::pair_list_of<std::string,mms::satellite_data_item_container>(
																										"car_shorthand",
																										mms::satellite_data_item_container(16,"synonym_car_shorthand")
																									)
						 ).operator mms::columns_satellite_data_container();
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	auto r = compare_create_clauses(exp,rcv);
	BOOST_CHECK(r);
	// VARCHAR length
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16]}]\
	]";
	f = q.cbegin(), l = q.cend();
	rcv = mms::create_clause_container();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	exp.satellite_data.at(0).second.second = "";
	r = compare_create_clauses(exp,rcv);
	BOOST_CHECK(r);
	// FOREIGN column name
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints_sdata\" : [{\"car_shorthand\":[\"synonym_car_shorthand\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	rcv = create_clause_container();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	exp.satellite_data.at(0).second.first = 0U;
	exp.satellite_data.at(0).second.second = "synonym_car_shorthand";
	r = compare_create_clauses(exp,rcv);
	BOOST_CHECK(r);
	// empty
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints_sdata\" : [{\"car_shorthand\":[]}]\
	]";
	f = q.cbegin(), l = q.cend();
	rcv = create_clause_container();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	exp.satellite_data.at(0).second.first = 0U;
	exp.satellite_data.at(0).second.second = "";
	r = compare_create_clauses(exp,rcv);
	BOOST_CHECK(r);
} // all_valid_combinations
BOOST_AUTO_TEST_CASE( invalid_combinations )
{
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints_sdata\" : [{\"car_shorthand\":[\"synonym_car_shorthand\",16]}]\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv),boost_qi_exception);
} // invalid_combinations
//BOOST_AUTO_TEST_CASE( first_item_non_integer )
//{
//	q = 
//	"[\
//		\"action\" : \"create\",\
//		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
//		\"types\": [ 3, 0, 3, 1, 3],\
//		\"constraints_sdata\" : [{\"car_shorthand\":[\"synonym_car_shorthand\",16]}]\
//	]";
//	it f = q.cbegin(), l = q.cend();
//	rcv = create_clause_container();
//	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv),boost_qi_exception);
//} // first_item_non_integer
BOOST_AUTO_TEST_CASE( second_item_non_string )
{
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,15]}]\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv),boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,1.0]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv),boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,true]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv),boost_qi_exception);
} // second_item_non_string
BOOST_AUTO_TEST_CASE( column_name_non_string )
{
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints_sdata\" : [{1:[16,true]}]\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv),boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints_sdata\" : [{1.0:[16,true]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv),boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints_sdata\" : [{true:[16,true]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv),boost_qi_exception);
} // column_name_non_string
BOOST_AUTO_TEST_CASE( constraints_sdata_wrong_keyword )
{
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints_sdata!\" : [{\"car_shorthand\":[16,true]}]\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv),boost_qi_exception);
} // constraints_sdata_wrong_keyword
BOOST_AUTO_TEST_CASE( not_two_params )
{
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 3, 0, 3, 1, 3],\
		\"constraints_sdata\" : [{\"car_shorthand\":[1,\"2d\",\"3d\"]}]\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv),boost_qi_exception);
} // not_two_params
BOOST_AUTO_TEST_SUITE_END() // constraints_sdata

BOOST_AUTO_TEST_CASE( invalid_grammar)
{
	// colon
	q = 
	"[\
		\"action\"  \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false,\"primary\":true},{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\"  [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false,\"primary\":true},{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\" [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false,\"primary\":true},{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\"  [{\"null\":false,\"primary\":true},{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false,\"primary\":true},{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}],\
		\"constraints_sdata\"  [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// opened curly bracket
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{{\"null\":false,\"primary\":true},{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false,\"primary\":true},{{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false,\"primary\":true},{\"null\":false},{\"null\":false},{\"null\":false},{{\"null\":false}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false,\"primary\":true},{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}],\
		\"constraints_sdata\" : [{{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false,\"primary\":true},{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// opened square bracket
	q = 
	"\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false,\"primary\":true},{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// closed square bracket
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false,\"primary\":true},{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// closed curly bracket
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false,\"primary\":true,{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false,\"primary\":true}},{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false,\"primary\":true},{\"null\":false,{\"null\":false},{\"null\":false},{\"null\":false}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false,\"primary\":true},{\"null\":false}},{\"null\":false},{\"null\":false},{\"null\":false}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false,\"primary\":true},{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false,\"primary\":true},{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false,\"primary\":true},{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"],{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false,\"primary\":true},{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]}},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false,\"primary\":true},{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false,\"primary\":true},{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// 'after-action' comma
	q = 
	"[\
		\"action\" : \"create\"\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false,\"primary\":true},{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception)
	// 'after-columns' comma
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"]\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false,\"primary\":true},{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception)
	// 'after-types' comma
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4]\
		\"constraints\" : [{\"null\":false,\"primary\":true},{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}],\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception)
	// 'after-constraints' comma
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false,\"primary\":true},{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}]\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception)
	// action value
	q = 
	"[\
		\"action\" : \
		\"columns\"\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception)
	// no action
	q = 
	"[\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false,\"primary\":true},{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}]\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// types: no comma
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false,\"primary\":true},{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}]\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// types: excessive comma
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4,, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false,\"primary\":true},{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}]\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// columns: excessive comma
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",,\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false,\"primary\":true},{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}]\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// columns: no comma
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\"\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false,\"primary\":true},{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}]\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// constraints: no comma
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false \"primary\":true},{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}]\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// constraints: excessive comma
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false,,\"primary\":true},{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}]\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// constraints_sdata: no comma
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false, \"primary\":true},{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}]\
		\"constraints_sdata\" : [{\"car_shorthand\":[16 \"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// constraints_sdata: excessive comma
	q = 
	"[\
		\"action\" : \"create\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"types\": [ 4, 0, 4, 2, 4],\
		\"constraints\" : [{\"null\":false,\"primary\":true},{\"null\":false},{\"null\":false},{\"null\":false},{\"null\":false}]\
		\"constraints_sdata\" : [{\"car_shorthand\":[16,,\"\"]},{\"car_body\":[20,\"\"]},{\"car_chassis\":[20,\"\"]}]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
} // invalid grammar
BOOST_AUTO_TEST_SUITE_END() // create

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
// UPDATE !!! TEST !!!
///////////////////////////////////////////////////////////////////////////////////
BOOST_FIXTURE_TEST_SUITE( update, update_fxt )
BOOST_AUTO_TEST_CASE( basic )
{
	reset();
	q = 
	"[\
		\"action\" : \"update\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"],\
		\"filter\" : { \"car_id\" : { \"IN\" : [1] } }\
	]";
	exp.columns = (
					::boost::assign::list_of
												("car_shorthand"),
												"car_class",
												"car_body",
												"car_engine",
												"car_chassis"
				 ).operator mms::columns_container(),
	exp.values = (
					::boost::assign::list_of<mms::value_t>
												(static_cast<std::string>("Motor")),
												5.0,
												static_cast<int>(1),
												2.0,
												static_cast<std::string>("Russian chassis")
				).operator mms::values_container(),
	exp.filters = boost::assign::map_list_of(
												"car_id",
												(
													boost::assign::list_of<mms::value_t>(static_cast<int>(1))
												).operator set()
											).operator mms::column_to_filter_container();
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	auto r = compare_update_clauses(exp,rcv);
	BOOST_CHECK(r);
	
	rcv = mms::update_clause_container();
	q = 
	"[\
		\"action\" : \"update\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"]\
	]";
	exp.filters = mms::column_to_filter_container();
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	r = compare_update_clauses(exp,rcv);
	BOOST_CHECK(r);
	
}
BOOST_AUTO_TEST_CASE( filter )
{
	// If a comma is present after 'values' expression a valid 'filter' expression should follow.
	reset();
	q = 
	"[\
		\"action\" : \"update\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"],\
		\"invalid_filter\" : { \"car_id\" : { \"IN\" : [1] } }\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"update\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"],\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"update\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"],\
		\"filter\" : { \"car_id\" : { \"IN\" : [1] } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
}

// for compiler testing
//////////////////////////////////////////////////////////////
//BOOST_AUTO_TEST_CASE( invalid_columns_values_map_in_values )
//{
//	reset();
//	q = 
//	"[\
//		\"action\" : \"update\",\
//		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
//		\"values\": [ 5.0, 1, 2.0, \"Russian chassis\"]\
//	]";
//	it f = q.cbegin(), l = q.cend();
//	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
//}
//BOOST_AUTO_TEST_CASE( invalid_columns_values_map_in_columns )
//{
//	reset();
//	q = 
//	"[\
//		\"action\" : \"update\",\
//		\"columns\" : [\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
//		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"]\
//	]";
//	it f = q.cbegin(), l = q.cend();
//	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
//}
//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( invalid_no_columns )
{
	reset();
	q = 
	"[\
		\"action\" : \"update\",\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"]\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
}
BOOST_AUTO_TEST_CASE( invalid_no_action )
{
	reset();
	q = 
	"[\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"]\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
}
BOOST_AUTO_TEST_CASE( invalid_column_name )
{
	reset();
	// No quote before car_class.
	q = 
	"[\
		\"action\" : \"update\",\
		\"columns\" : [\"car_shorthand\",car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"]\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// Excessive quote in car_class.
	q = 
	"[\
		\"action\" : \"update\",\
		\"columns\" : [\"car_shorthand\",\"\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
}
BOOST_AUTO_TEST_CASE( invalid_value )
{
	reset();
	// Excessive quote in Motor value.
	q = 
	"[\
		\"action\" : \"update\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\"\", 5.0, 1, 2.0, \"Russian chassis\"]\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// 5.0+5.
	q = 
	"[\
		\"action\" : \"update\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\", 5.0+5, 1, 2.0, \"Russian chassis\"]\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
}
BOOST_AUTO_TEST_CASE( invalid_action_name )
{
	reset();
	q = 
	"[\
		\"action\" : \"update!\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"]\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
}
BOOST_AUTO_TEST_CASE( invalid_grammar )
{
	reset();
	// colon
	q = 
	"[\
		\"action\"  \"update\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"],\
		\"filter\" : { \"car_id\" : { \"IN\" : [1] } }\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"update\",\
		\"columns\"  [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"],\
		\"filter\" : { \"car_id\" : { \"IN\" : [1] } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
	\"action\" : \"update\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\" [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"],\
		\"filter\" : { \"car_id\" : { \"IN\" : [1] } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"update\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"],\
		\"filter\"  { \"car_id\" : { \"IN\" : [1] } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// opened curly bracket
	q = 
	"[\
		\"action\" : \"update\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"],\
		\"filter\" : {{ \"car_id\" : { \"IN\" : [1] } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	q = 
	"[\
		\"action\" : \"update\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"],\
		\"filter\" : { \"car_id\" : {{ \"IN\" : [1] } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// open square bracket
	q = 
	"\
		\"action\" : \"update\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"],\
		\"filter\" : { \"car_id\" : { \"IN\" : [1] } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// closed square bracket
	q = 
	"[\
		\"action\" : \"update\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"],\
		\"filter\" : { \"car_id\" : { \"IN\" : [1] } }\
	";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// closed filter curly bracket
	q = 
	"[\
		\"action\" : \"update\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"],\
		\"filter\" : { \"car_id\" : { \"IN\" : [1] } \
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// opened filter curly bracket
	q = 
	"[\
		\"action\" : \"update\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"],\
		\"filter\" :  \"car_id\" : { \"IN\" : [1] } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// opened interval curly bracket
	q = 
	"[\
		\"action\" : \"update\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"],\
		\"filter\" :  { \"car_id\" : { \"IN\" : [1,4,5.0] }, \"car_engine\" :  \"interval\" : {0:1.8,3:3.2,\"LOGIC\":1} } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// closed interval curly bracket
	q = 
	"[\
		\"action\" : \"update\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"],\
		\"filter\" :  { \"car_id\" : { \"IN\" : [1,4,5.0] }, \"car_engine\" : { \"interval\" : {0:1.8,3:3.2,\"LOGIC\":1}  }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// opened interval value curly bracket
	q = 
	"[\
		\"action\" : \"update\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"],\
		\"filter\" :  { \"car_id\" : { \"IN\" : [1,4,5.0] }, \"car_engine\" : { \"interval\" : 0:1.8,3:3.2,\"LOGIC\":1} } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// closed interval value curly bracket
	q = 
	"[\
		\"action\" : \"update\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"],\
		\"filter\" : { \"car_id\" : { \"IN\" : [1,4,5.0] }, \"car_engine\" : { \"interval\" : {0:1.8,3:3.2,\"LOGIC\":1 } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// 'after-action' comma
	"[\
		\"action\" : \"update\"\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"],\
		\"filter\" : { \"car_id\" : { \"IN\" : [1,4,5.0] }, \"car_engine\" : { \"interval\" : {0:1.8,3:3.2,\"LOGIC\":1} } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception)
	// 'after-columns' comma
	q = 
	"[\
		\"action\" : \"update\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"]\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"],\
		\"filter\" : { \"car_id\" : { \"IN\" : [1,4,5.0] } \"car_engine\" : { \"interval\" : {0:1.8,3:3.2,\"LOGIC\":1} } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception)
	// 'after-values' comma
	q = 
	"[\
		\"action\" : \"update\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"]\
		\"filter\" : { \"car_id\" : { \"IN\" : [1,4,5.0] } \"car_engine\" : { \"interval\" : {0:1.8,3:3.2,\"LOGIC\":1} } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception)
	// 'interval' colon 
	q = 
	"[\
		\"action\" : \"update\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"],\
		\"filter\" : { \"car_id\" : { \"IN\" : [1,4,5.0] }, \"car_engine\" : { \"interval\"  {0:1.8,3:3.2,\"LOGIC\":1} } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception)
	// 'in' colon 
	q = 
	"[\
		\"action\" : \"update\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"],\
		\"filter\" : { \"car_id\" : { \"IN\"  [1,4,5.0] }, \"car_engine\" : { \"interval\" : {0:1.8,3:3.2,\"LOGIC\":1} } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception)
	// 'logic' colon 
	q = 
	"[\
		\"action\" : \"update\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"],\
		\"filter\" : { \"car_id\" : { \"IN\" : [1,4,5.0] }, \"car_engine\" : { \"interval\" : {0:1.8,3:3.2,\"LOGIC\"1} } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception)
	// filter value
	q = 
	"[\
		\"action\" : \"update\",\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"],\
		\"filter\" : \
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception)
	// action value
	q = 
	"[\
		\"action\" : ,\
		\"columns\" : [\"car_shorthand\",\"car_class\",\"car_body\", \"car_engine\", \"car_chassis\"],\
		\"values\": [ \"Motor\", 5.0, 1, 2.0, \"Russian chassis\"],\
		\"filter\" : { \"car_id\" : { \"IN\" : [1,4,5.0] }, \"car_engine\" : { \"interval\" : {0:1.8,3:3.2,\"LOGIC\":1} } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception)
	// no action
	q = 
	"[\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
} // invalid grammar
BOOST_AUTO_TEST_SUITE_END() // update

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
// DELETE !!! TEST !!!
///////////////////////////////////////////////////////////////////////////////////
BOOST_FIXTURE_TEST_SUITE( delete_, delete_fxt )
BOOST_AUTO_TEST_CASE( basics_set_and_interval )
{
	reset();
	q = 
	"[\
		\"action\" : \"delete\",\
		\"filter\" : { \"car_id\" : { \"IN\" : [1,4,5.0] } }\
	]";
	exp.filters = boost::assign::map_list_of(
												"car_id",
												(
													::boost::assign::list_of<mms::value_t>(1), 4, 5.0
												).operator mms::set()
											).operator mms::column_to_filter_container();
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	auto r = compare_delete_clauses(exp,rcv);
	BOOST_CHECK(r);
	
	rcv = mms::delete_clause_container();
	q = 
	"[\
		\"action\" : \"delete\",\
		\"filter\" : { \"car_id\" : { \"IN\" : [1,4,5.0] }, \"car_engine\" : { \"interval\" : {0:1.8,3:3.2,\"LOGIC\":1} } }\
	]";
	mms::interval intrv = { 0,1.8, 3,3.2, 1};
	exp.filters.push_back( std::make_pair( "car_engine", intrv ) );
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv));
	r = compare_delete_clauses(exp,rcv);
	BOOST_CHECK(r);
	
} // basics_set_and_interval
BOOST_AUTO_TEST_CASE( invalid_filter_set )
{
	reset();
	q = 
	"[\
		\"action\" : \"delete\",\
		\"filter\" : { \"car_id\" : { \"IN\" : [1,4,5.0,] }, \"car_engine\" : { \"interval\" : {0:1.8,3:3.2,\"LOGIC\":1} } }\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
} // invalid_filter_set
BOOST_AUTO_TEST_CASE( invalid_filter_interval )
{
	reset();
	// logic
	q = 
	"[\
		\"action\" : \"delete\",\
		\"filter\" : { \"car_id\" : { \"IN\" : [1,4,5.0] }, \"car_engine\" : { \"interval\" : {0:1.8,3:3.2,\"LOGIC\":2} } }\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// relation with max
	q = 
	"[\
		\"action\" : \"delete\",\
		\"filter\" : { \"car_id\" : { \"IN\" : [1,4,5.0] }, \"car_engine\" : { \"interval\" : {0:1.8,90:3.2,\"LOGIC\":1} } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// relation with min
	q = 
	"[\
		\"action\" : \"delete\",\
		\"filter\" : { \"car_id\" : { \"IN\" : [1,4,5.0] }, \"car_engine\" : { \"interval\" : {100:1.8,1:3.2,\"LOGIC\":1} } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// colon
	q = 
	"[\
		\"action\" : \"delete\",\
		\"filter\" : { \"car_id\" : { \"IN\" : [1,4,5.0] }, \"car_engine\" : { \"interval\" :? {0:1.8,1:3.2,\"LOGIC\":1} } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
} // invalid_filter_interval
BOOST_AUTO_TEST_CASE( invalid_column_name )
{
	//// Not alphanumeric character.
	reset();
	//q = 
	//"[\
	//	\"action\" : \"delete\",\
	//	\"filter\" : { \"car_id!\" : { \"IN\" : [1,4,5.0] } }\
	//]";
	//it f = q.cbegin(), l = q.cend();
	//BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// No quote.
	q = 
	"[\
		\"action\" : \"delete\",\
		\"filter\" : { car_id\" : { \"IN\" : [1,4,5.0] } }\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// Excessive quote.
	q = 
	"[\
		\"action\" : \"delete\",\
		\"filter\" : { \"car_\"id\" : { \"IN\" : [1,4,5.0] } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
} // invalid_column_name
BOOST_AUTO_TEST_CASE( invalid_action_name )
{
	reset();
	q = 
	"[\
		\"action\" : \"!delete\",\
		\"filter\" : { \"car_id\" : { \"IN\" : [1,4,5.0] } }\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
} // invalid_action_name
BOOST_AUTO_TEST_CASE( invalid_grammar)
{
	// colon
	q = 
	"[\
		\"action\" : \"delete\",\
		\"filter\"  { \"car_id\" : { \"IN\" : [1,4,5.0] }, \"car_engine\" : { \"interval\" : {0:1.8,3:3.2,\"LOGIC\":1} } }\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// opened curly bracket
	q = 
	"[\
		\"action\" : \"delete\",\
		\"filter\" : { \"car_id\" : {{ \"IN\" : [1,4,5.0] }, \"car_engine\" : { \"interval\" : {0:1.8,3:3.2,\"LOGIC\":1} } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// open square bracket
	q = 
	"\
		\"action\" : \"delete\",\
		\"filter\" : { \"car_id\" : { \"IN\" : [1,4,5.0] }, \"car_engine\" : { \"interval\" : {0:1.8,3:3.2,\"LOGIC\":1} } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// closed square bracket
	q = 
	"[\
		\"action\" : \"delete\",\
		\"filter\" : { \"car_id\" : { \"IN\" : [1,4,5.0] }, \"car_engine\" : { \"interval\" : {0:1.8,3:3.2,\"LOGIC\":1} } }\
	";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// closed filter curly bracket
	q = 
	"[\
		\"action\" : \"delete\",\
		\"filter\" : { \"car_id\" : { \"IN\" : [1,4,5.0] }, \"car_engine\" : { \"interval\" : {0:1.8,3:3.2,\"LOGIC\":1} } \
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// opened filter curly bracket
	q = 
	"[\
		\"action\" : \"delete\",\
		\"filter\" :  \"car_id\" : { \"IN\" : [1,4,5.0] }, \"car_engine\" : { \"interval\" : {0:1.8,3:3.2,\"LOGIC\":1} } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// opened interval curly bracket
	q = 
	"[\
		\"action\" : \"delete\",\
		\"filter\" :  { \"car_id\" : { \"IN\" : [1,4,5.0] }, \"car_engine\" :  \"interval\" : {0:1.8,3:3.2,\"LOGIC\":1} } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// closed interval curly bracket
	q = 
	"[\
		\"action\" : \"delete\",\
		\"filter\" : { \"car_id\" : { \"IN\" : [1,4,5.0] }, \"car_engine\" : { \"interval\" : {0:1.8,3:3.2,\"LOGIC\":1}  }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// opened interval value curly bracket
	q = 
	"[\
		\"action\" : \"delete\",\
		\"filter\" :  { \"car_id\" : { \"IN\" : [1,4,5.0] }, \"car_engine\" : { \"interval\" : 0:1.8,3:3.2,\"LOGIC\":1} } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// closed interval value curly bracket
	q = 
	"[\
		\"action\" : \"delete\",\
		\"filter\" : { \"car_id\" : { \"IN\" : [1,4,5.0] }, \"car_engine\" : { \"interval\" : {0:1.8,3:3.2,\"LOGIC\":1 } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
	// 'after-action' comma
	q = 
	"[\
		\"action\" : \"delete\"\
		\"filter\" : { \"car_id\" : { \"IN\" : [1,4,5.0] }, \"car_engine\" : { \"interval\" : {0:1.8,3:3.2,\"LOGIC\":1} } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception)
	// 'after-filter' comma
	q = 
	"[\
		\"action\" : \"delete\",\
		\"filter\" : { \"car_id\" : { \"IN\" : [1,4,5.0] } \"car_engine\" : { \"interval\" : {0:1.8,3:3.2,\"LOGIC\":1} } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception)
	// 'interval' colon 
	q = 
	"[\
		\"action\" : \"delete\",\
		\"filter\" : { \"car_id\" : { \"IN\" : [1,4,5.0] }, \"car_engine\" : { \"interval\"  {0:1.8,3:3.2,\"LOGIC\":1} } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception)
	// 'in' colon 
	q = 
	"[\
		\"action\" : \"delete\",\
		\"filter\" : { \"car_id\" : { \"IN\"  [1,4,5.0] }, \"car_engine\" : { \"interval\" : {0:1.8,3:3.2,\"LOGIC\":1} } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception)
	// 'logic' colon 
	q = 
	"[\
		\"action\" : \"delete\",\
		\"filter\" : { \"car_id\" : { \"IN\" : [1,4,5.0] }, \"car_engine\" : { \"interval\" : {0:1.8,3:3.2,\"LOGIC\"1} } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception)
	// filter value
	q = 
	"[\
		\"action\" : \"delete\",\
		\"filter\" : \
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception)
	// action value
	q = 
	"[\
		\"action\" : \
		\"filter\" : { \"car_id\" : { \"IN\" : [1,4,5.0] }, \"car_engine\" : { \"interval\" : {0:1.8,3:3.2,\"LOGIC\":1} } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception)
	// no action
	q = 
	"[\
		\"filter\" : { \"car_id\" : { \"IN\" : [1,4,5.0] } }\
	]";
	f = q.cbegin(), l = q.cend();
	BOOST_CHECK_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv), boost_qi_exception);
} // invalid grammar
BOOST_AUTO_TEST_CASE( valid_grammar )
{
	reset();
	// no filter
	q = 
	"[\
		\"action\" : \"delete\"\
	]";
	it f = q.cbegin(), l = q.cend();
	BOOST_CHECK_NO_THROW(boost::spirit::qi::phrase_parse(f,l,p,boost::spirit::qi::space,rcv))
	auto r = compare_delete_clauses(exp,rcv);
	BOOST_CHECK(r);
	
} // valid grammar
BOOST_AUTO_TEST_SUITE_END() // delete

BOOST_AUTO_TEST_SUITE_END() // parsers

#endif // ifdef TEST_ON