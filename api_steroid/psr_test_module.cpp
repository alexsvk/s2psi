/*!
\brief MMS parsers tester.
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

// LOCAL
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
// PARSERS
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef std::string::const_iterator it;
typedef ::boost::spirit::qi::expectation_failure<it> boost_qi_exception;
typedef boost::spirit::qi::space_type skipper;
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
	std::string q;
	clause_container exp;
	clause_container rcv;
	parser p;
};

struct insert_fxt : public parsers_fxt<insert_clause_container,insert_parser<it,skipper>>
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
std::ostream & operator<<(std::ostream & os, interval const & i)
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
std::ostream & operator<<(std::ostream & os, set const & s)
{
	std::cout << "Set =>\n";
	for ( auto const & e : s ) ::boost::apply_visitor(value_visitor(), e); // operator<<(std::basic_ostream<ElemType,Traits> & out, 
             								     // const variant<T1, T2, ..., TN> & rhs); is not used by way of more verbous output
	return os;
}
struct filter_visitor : public boost::static_visitor<void>
{
    void operator()(interval const & i) const
    {
	std::cout << i;
    }
    void operator()(set const & s) const
    {
	std::cout << s;
    }
};

template<typename out_stream>
void chk_distinct(out_stream & msg, select_clause_container const & exp, select_clause_container const & rcv)
{
	if (exp.distinct != rcv.distinct)  msg << "Difference in field distinct: " << std::boolalpha << exp.distinct << ", " << rcv.distinct;
}

template<typename out_stream>
void chk_aggregates(out_stream & msg, select_clause_container const & exp, select_clause_container const & rcv)
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
bool operator==(interval const & f1, interval const & f2)
{
	if ( f1.relation_with_min == f2.relation_with_min && f1.min_val == f2.min_val && f1.relation_with_max == f2.relation_with_max && f1.max_val == f2.max_val && f1.concatenation_logic == f2.concatenation_logic ) return true;
	return false;
}
template<typename out_stream, typename clause_container>
void chk_filters(out_stream & msg, clause_container const & exp, clause_container const & rcv)
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
void chk_groups(out_stream & msg, select_clause_container const & exp, select_clause_container const & rcv)
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
void chk_group_filters(out_stream & msg, select_clause_container const & exp, select_clause_container const & rcv)
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
bool operator!=(order_container const & lord, order_container const & rord)
{
	if ( lord.order == rord.order && std::equal(lord.columns.cbegin(),lord.columns.cend(),rord.columns.cbegin()) ) return false;
	return true;
}
template<typename out_stream>
void chk_order_options(out_stream & msg, select_clause_container const & exp, select_clause_container const & rcv)
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
void chk_limit_options(out_stream & msg, select_clause_container const & exp, select_clause_container const & rcv)
{
	if ( exp.limit_options != rcv.limit_options )
	{
		msg << "Difference in field limit options: ";
		auto const & exp_(exp.limit_options), rcv_(rcv.limit_options);
		if ( exp_.first != rcv_.first ) msg << " (offset: " << exp_.first << ", " << rcv_.first << ')';
		if ( exp_.second != rcv_.second ) msg << " (count: " << exp_.second << ", " << rcv_.second << ')';
	}
}
boost::test_tools::predicate_result compare_select_clauses(select_clause_container const & exp, select_clause_container const & rcv)
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

std::ostream & operator<<(std::ostream & cout, select_clause_container const & sel_clause)
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
bool operator==(sql_data_type_wrapper const & lhs_wrp, sql_data_type_wrapper const & rhs_wrp)
{
	return lhs_wrp.v == rhs_wrp.v;
}
std::ostream & operator<<(std::ostream & os, sql_data_type const & sql_data_type_)
{
	switch ( sql_data_type_ )
	{
	case sql_data_type::int_: os << "int";
		break;
	case sql_data_type::datetime: os << "datetime";
		break;
	case sql_data_type::double_: os << "double";
		break;
	case sql_data_type::bool_: os << "bool";
		break;
	case sql_data_type::varchar: os << "varchar";
		break;
	}
	return os;
}
template<typename out_stream, typename clause_container>
void chk_types(out_stream & msg, clause_container const & exp, clause_container const & rcv)
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
bool operator==(constraint_container const & lhs_cons, constraint_container const & rhs_cons)
{
	auto const & l = lhs_cons, r = rhs_cons;
	return 
			l.def_value		==		r.def_value		&&
			l.null			==		r.null			&&
			l.primary		==		r.primary		&&
			l.foreign		==		r.foreign;
}
std::ostream & operator<<(std::ostream & os, constraint_container const & constr)
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
std::ostream & operator<<(std::ostream & os, satellite_data_item_container const & satellite_item)
{
	return os << satellite_item.first << ',' << satellite_item.second;
}
std::ostream & operator<<(std::ostream & os, column_and_satellite_data_item_container const & satellite_col_and_item)
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
boost::test_tools::predicate_result compare_create_clauses(create_clause_container const & exp, create_clause_container const & rcv)
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
boost::test_tools::predicate_result compare_update_clauses(update_clause_container const & exp, update_clause_container const & rcv)
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
boost::test_tools::predicate_result compare_delete_clauses(delete_clause_container const & exp, delete_clause_container const & rcv)
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


//BOOST_AUTO_TEST_SUITE( parsers )

