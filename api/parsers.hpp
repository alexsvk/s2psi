#ifndef parsers_hpp
#define parsers_hpp

#define BOOST_SPIRIT_DEBUG

// SYSTEM
#include <vector>
#include <string>
#include <memory>
#include <map>
// PROJECT
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp> // arguments, actors, values, references, ...
#include <boost/spirit/include/phoenix_operator.hpp> // if_else, bitwise, logical, arithmetic, ... operators
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_stl.hpp> // push_back
#include <boost/spirit/include/qi_symbols.hpp> // used in: columns_list
#include <boost/spirit/home/phoenix/bind/bind_function.hpp> // used in: aggregates_list
#include <boost/fusion/include/adapt_struct.hpp> // used in: filter options
#include <boost/spirit/include/phoenix_fusion.hpp> // used in : filter options
#include <boost/variant.hpp> // JSON value
#include <boost/bind.hpp> // used in: aggregate list
#include <boost/any.hpp> // used in: aggregate list, filter options
#include <boost/fusion/adapted/std_pair.hpp> // used in: filter options, aggregate list
#include <boost/fusion/adapted.hpp>
#include <boost/assign/list_of.hpp> 
#include <boost/algorithm/string/split.hpp> 
#include <boost/algorithm/string.hpp>

namespace mms
{
	typedef ::std::string action_str;
	// Actions.
	action_str const 
		insert_action = "insert", 
		select_action = "select", 
		create_action = "create", 
		delete_action = "delete", 
		update_action = "update",
		mediator_and_friend_action = "mediator_and_friend",
		mediator_action = "mediator";

	enum sql_faggregate { AVG, COUNT, FIRST, LAST, MAX, MIN, SUM };
	typedef ::std::string column_name_t;
	typedef ::std::pair< column_name_t, sql_faggregate > column_and_aggregate;
	typedef ::std::vector< column_and_aggregate > column_and_aggregate_container;
	typedef ::std::wstring db_string;
	typedef ::boost::variant< ::std::string, int, double, bool, column_and_aggregate > value_t;
	typedef unsigned char uchar;
	enum relation { GRT, LESS, GEQ, LEQ, EQ, NEQ };
	enum logic { and_, or_ };
	typedef ::std::pair<column_name_t,column_name_t> column_and_alias_pair;
	typedef ::std::vector<column_and_alias_pair> column_and_alias_container;
// filter
struct interval
{
	/*relation*/unsigned relation_with_min;
	value_t min_val;
	/*relation*/unsigned relation_with_max;
	value_t max_val;
	/*uchar*/unsigned concatenation_logic;
};

} // namespace mms


BOOST_FUSION_ADAPT_STRUCT(
	mms::interval,
	(/*s2p::mms::relation*/unsigned, relation_with_min)
	(mms::value_t, min_val)
	(/*s2p::mms::relation*/unsigned, relation_with_max)
	(mms::value_t, max_val)
	(unsigned, concatenation_logic)
)

namespace mms
{

	typedef ::std::vector<value_t> values_container;
	typedef values_container set;
	typedef ::boost::variant<interval,set,column_name_t> column_filter;
	typedef ::std::pair<column_name_t,column_filter> column_to_filter;
	typedef ::std::vector<column_to_filter> column_to_filter_container;

	typedef ::std::vector< column_name_t > columns_container;
	struct order_container
	{
		columns_container columns;
		unsigned order;
	};
	enum order { asc, desc };
} // namespace mms

BOOST_FUSION_ADAPT_STRUCT(
	mms::order_container,
	(mms::columns_container, columns)
	(unsigned, order)
)
namespace mms
{

	typedef unsigned long int offset_t; // At the maximum page size of 65536 bytes. The maximum size of a database file is 2147483646 pages.
										// A 140 terabytes database can hold no more than approximately 1e+13 rows, 
										// this translates into a maximum database size of approximately 1.4e+14 bytes (140 terabytes)
										// and then only if there are no indices and if each row contains very little data. 
	typedef unsigned long int rows_cnt_t;
	typedef ::std::pair< offset_t, rows_cnt_t > limit_pair;

	typedef ::std::pair<::std::string,::std::string> col_and_ref_col;
	typedef ::std::vector<col_and_ref_col> col_and_ref_col_container;

	struct select_clause_container
	{
		bool distinct;
		columns_container columns;
		column_and_aggregate_container aggregates;
		column_and_alias_container aliases;
		column_to_filter_container filters;
		columns_container groups;
		column_to_filter_container group_filters;
		order_container order_options;
		limit_pair limit_options;
		col_and_ref_col_container map;
	};
} // namespace mms

BOOST_FUSION_ADAPT_STRUCT(
	mms::select_clause_container,
	(bool, distinct)
	(mms::columns_container, columns)
	(mms::column_and_aggregate_container, aggregates)
	(mms::column_and_alias_container, aliases)
	(mms::column_to_filter_container, filters)
	(mms::columns_container, groups)
	(mms::column_to_filter_container, group_filters)
	(mms::order_container, order_options)
	(mms::limit_pair, limit_options)
	(mms::col_and_ref_col_container, map)
)
namespace mms
{
	typedef ::boost::variant< ::std::string, int, double, bool > insert_value_t;
	typedef ::std::vector<insert_value_t> insert_values_container;
	struct insert_clause_container
	{
		insert_clause_container
		(
			columns_container const & columns_ = columns_container(),
			insert_values_container values_ = insert_values_container(),
			bool const default_all_ = false
		) : columns( columns_ ), values( values_ ), default_all( default_all_ )
		{
		}
		columns_container columns;
		insert_values_container values;
		bool default_all;
	};
} // namespace mms

BOOST_FUSION_ADAPT_STRUCT(
	mms::insert_clause_container,
	(mms::columns_container, columns)
	(mms::insert_values_container, values)
	(bool, default_all)
)
namespace mms
{
	using namespace ::boost::spirit;
	template<typename it, typename skipper>
	struct action_parser : qi::grammar<it, ::std::string(::std::string const &), skipper>
	{
		action_parser() : action_parser::base_type(action_parser_)
		{
			using namespace ::boost::spirit::qi;
			action_parser_ %= 
			'"'
			> no_case[lit("action")] //N.B.!!! no_case directive brings case-insensitive behavior in the code.
			> '"'
			> lit(':')
			> '"'
			> no_case[::boost::spirit::qi::string(_r1)]
			> '"';
			BOOST_SPIRIT_DEBUG_NODE(action_parser_);
		}
	private:
		::boost::spirit::qi::rule<it, ::std::string(::std::string const &), skipper> action_parser_;
	};
	template<typename it, typename skipper>
	struct distinct_parser : qi::grammar<it, bool(), skipper>
	{
		distinct_parser() : distinct_parser::base_type(distinct_expr)
		{
			using namespace ::boost::spirit::qi;
			distinct_expr %= lit('"') >> no_case["distinct"] > lit('"') > lit(':') > bool_ > ',' | attr(false);
			BOOST_SPIRIT_DEBUG_NODE(distinct_expr);
		}
	private:
		::boost::spirit::qi::rule<it, bool(), skipper> distinct_expr;
	};
	template <typename it, typename skipper = qi::space_type>
	struct quoted_string_parser : qi::grammar<it, ::std::string(), skipper>
	{
		quoted_string_parser() : quoted_string_parser::base_type(quoted_string)
		{
			using namespace ::boost::spirit::qi;
			quoted_string %= lexeme['"' >> *~char_('"') >> '"'];
			BOOST_SPIRIT_DEBUG_NODE(quoted_string);
		}
		::boost::spirit::qi::rule<it, ::std::string(), skipper> quoted_string;
	};
	template <typename it, typename skipper = qi::space_type>
	struct columns_container_parser : qi::grammar<it, columns_container(), skipper>
	{
		columns_container_parser() : columns_container_parser::base_type(columns)
		{
			using namespace ::boost::spirit::qi;
			columns %= 
						lit('[') >> ']'

						|

						'[' > quoted_string % ',' > ']';  

			BOOST_SPIRIT_DEBUG_NODE(columns);
		}
	  private:
		::boost::spirit::qi::rule<it, columns_container(), qi::space_type> columns;
		quoted_string_parser<it,skipper> quoted_string;
	};
	template <typename it, typename skipper = qi::space_type>
	struct columns_parser : qi::grammar<it, columns_container(), skipper>
	{
		columns_parser() : columns_parser::base_type(columns_expr)
		{
			using namespace ::boost::spirit::qi;
			columns_expr %= '"' >> no_case[lit("columns")] >> '"' > lit(':') > columns;  
			BOOST_SPIRIT_DEBUG_NODE(columns_expr);
		}
	  private:
		::boost::spirit::qi::rule<it, columns_container(), qi::space_type> columns_expr;
		quoted_string_parser<it,skipper> quoted_string;
		columns_container_parser<it,skipper> columns;
	};
	template <typename it, typename skipper = qi::space_type>
	struct aggregate_parser : qi::grammar<it, column_and_aggregate(), skipper>
	{
		aggregate_parser() : aggregate_parser::base_type(agg_pair)
		{
			using namespace ::boost::spirit::qi;
			agg_pair %= quoted_string >> ':' >> int_[_pass = (qi::_1 >= AVG && qi::_1 <= SUM)];
			BOOST_SPIRIT_DEBUG_NODE(agg_pair);
		}
	  private:    
		::boost::spirit::qi::rule<it, column_and_aggregate(), skipper> agg_pair;
		quoted_string_parser<it,skipper> quoted_string;
	};
	template <typename it, typename skipper = qi::space_type>
	struct aggregates_parser : qi::grammar<it, column_and_aggregate_container(), skipper>
	{
		aggregates_parser() : aggregates_parser::base_type(aggregates_expr)
		{
			using namespace ::boost::spirit::qi;
			aggregates_expr= '"' >> no_case[lit("aggregates")] >> '"' > ':' > '{' > aggr_pair % ',' > '}';
			BOOST_SPIRIT_DEBUG_NODE(aggregates_expr);
		}
	  private:    
		aggregate_parser<it, skipper> aggr_pair;
		::boost::spirit::qi::rule<it, column_and_aggregate_container(), skipper> aggregates_expr;
	};
	template <typename it, typename skipper = qi::space_type>
	struct value_parser : ::boost::spirit::qi::grammar<it, value_t(), skipper>
	{
		value_parser() : value_parser::base_type(value)
		{
			using namespace ::boost::spirit::qi;
			value = aggregate | 
					quoted_string | 
					real_parser<double,strict_real_policies<double>>() | 
					int_ | 
					bool_;
			BOOST_SPIRIT_DEBUG_NODE(value);
		}
	  private:
		::boost::spirit::qi::rule<it, value_t(), skipper> value;
		quoted_string_parser<it,skipper> quoted_string;
		aggregate_parser<it,skipper> aggregate;
	};
	template <typename it, typename skipper = qi::space_type, typename value_parser_ = value_parser<it,skipper>>
	struct filter_options_parser : qi::grammar<it, column_to_filter_container(::std::string const &), skipper>
	{
		filter_options_parser() : filter_options_parser::base_type(filters_expr)
		{
			using namespace ::boost::spirit::qi;
			interval_ %= '"' >> no_case[lit("interval")] >> '"' > ':' 
					> '{'
					> int_[_pass = (qi::_1 >= GRT && qi::_1 <= NEQ)] > ':' > value > ','
					> int_[_pass = (qi::_1 >= GRT && qi::_1 <= NEQ)] > ':' > value > ','
					> '"' > no_case[lit("logic")] > '"' > ':' > int_[_pass = (qi::_1 >= and_ && qi::_1 <= or_)]
					> '}';
			set_ %= '"' >> no_case[lit("in")] > '"' > ':' > lit('[') > value % ',' > ']';
			like %= '"' >> no_case[lit("like")] > '"' > ':' > quoted_string;
			filter %= quoted_string > ':' > '{' > (interval_ | set_ | like) > '}';
			filters_expr %= '"' >> omit[no_case[_r1]] >> '"' > ':' > '{' > filter % ',' > '}';
			BOOST_SPIRIT_DEBUG_NODE(like);
			BOOST_SPIRIT_DEBUG_NODE(interval_);
			BOOST_SPIRIT_DEBUG_NODE(set_);
			BOOST_SPIRIT_DEBUG_NODE(filter);
			BOOST_SPIRIT_DEBUG_NODE(filters_expr);
		}
	  protected:
		::boost::spirit::qi::rule<it,column_to_filter_container(::std::string const &),skipper> filters_expr;
		::boost::spirit::qi::rule<it,column_to_filter(),skipper> filter;
		::boost::spirit::qi::rule<it,column_name_t(),skipper> like;
		::boost::spirit::qi::rule<it,set(),skipper> set_;
		::boost::spirit::qi::rule<it,interval(),skipper> interval_;
		quoted_string_parser<it,skipper> quoted_string;
		value_parser_ value;
	};
	template <typename it, typename skipper = qi::space_type>
	struct groups_parser : qi::grammar<it, columns_container(), skipper>
	{
		groups_parser() : groups_parser::base_type(groups)
		{
			using namespace ::boost::spirit::qi;
			groups %= '"' >> no_case[lit("groups")] >> '"' > ':' > names;
			BOOST_SPIRIT_DEBUG_NODE(groups);
		}
	  private:
		  columns_container_parser<it,skipper> names;
		  ::boost::spirit::qi::rule<it,columns_container(),skipper > groups;
	};
	template <typename it, typename skipper = qi::space_type>
	struct group_filter : qi::grammar<it, column_to_filter_container(), skipper>
	{
		group_filter() : group_filter::base_type(groups_filter_expr), expr_header("group_filter")
		{
			using namespace ::boost::spirit::qi;
			groups_filter_expr %= filter(expr_header);
			BOOST_SPIRIT_DEBUG_NODE(groups_filter_expr);
		}
	  private:
		  ::boost::spirit::qi::rule<it,column_to_filter_container(),skipper> groups_filter_expr;
		  filter_options_parser<it,skipper> filter;
		  ::std::string const expr_header;
	};
	template <typename it, typename skipper = qi::space_type>
	struct order_options_parser : qi::grammar<it, order_container(), skipper>
	{
		order_options_parser() : order_options_parser::base_type(order_options)
		{
			using namespace ::boost::spirit::qi;
			order_options %= '"' >> no_case[lit("order")] >> '"' > ':' > lit('[') > column_names > ',' > int_[_pass = (qi::_1 >= asc && qi::_1 <= desc)] > ']';
			BOOST_SPIRIT_DEBUG_NODE(order_options);
		}
	  private:
		  ::boost::spirit::qi::rule<it,order_container(),skipper> order_options;
		  quoted_string_parser<it,skipper> quoted_string;
		  columns_container_parser<it,skipper> column_names;
	};
	template <typename it, typename skipper = qi::space_type>
	struct limit_parser : qi::grammar<it, limit_pair(), skipper>
	{
		limit_parser() : limit_parser::base_type(limit_and_offset)
		{
			using namespace ::boost::spirit::qi;
			limit_and_offset %= '"' >> no_case[lit("limit")] >> '"' > ':' > '[' > int_ > ',' > int_ > ']';
			BOOST_SPIRIT_DEBUG_NODE(limit_and_offset);
		}
	  private:
		  ::boost::spirit::qi::rule<it,limit_pair(),skipper> limit_and_offset;
	};
	template <typename it, typename skipper = qi::space_type>
	struct map_parser : qi::grammar<it, col_and_ref_col_container(), skipper>
	{
		map_parser() : map_parser::base_type(map)
		{
			using namespace ::boost::spirit::qi;
			map_pair %= column_name > ',' > column_name;
			map %= '"' >> no_case[lit("map")] >> '"' > ':' > '[' > map_pair % ',' > ']';
			BOOST_SPIRIT_DEBUG_NODE(map);
			BOOST_SPIRIT_DEBUG_NODE(map_pair);
		}
	  private:
		  ::boost::spirit::qi::rule<it,col_and_ref_col_container(),skipper> map;
		  ::boost::spirit::qi::rule<it,col_and_ref_col(),skipper> map_pair;
		  quoted_string_parser<it,skipper> column_name;
	};
	template <typename it, typename skipper = qi::space_type>
	struct optional_comma : qi::grammar<it, void(), skipper>
	{
		optional_comma() : optional_comma::base_type(optional_comma_)
		{
			using namespace ::boost::spirit::qi;
			optional_comma_ = ',' | omit[attr(0)];
		}
	private:
		::boost::spirit::qi::rule<it,void(),skipper> optional_comma_;
	};
	template <typename it, typename skipper = qi::space_type>
	struct as_parser : qi::grammar<it, column_and_alias_container(), skipper>
	{
		as_parser() : as_parser::base_type(aliases)
		{
			using namespace ::boost::spirit::qi;
			column_and_alias %= '{' > column_name > ':' > column_name > '}';
			aliases %= '"' >> no_case[lit("as")] >> '"' > ':' > '[' > column_and_alias % ',' > ']';
			BOOST_SPIRIT_DEBUG_NODE(aliases);
			BOOST_SPIRIT_DEBUG_NODE(column_and_alias);
		}
	private:
		::boost::spirit::qi::rule<it,column_and_alias_container(),skipper> aliases;
		::boost::spirit::qi::rule<it,column_and_alias_pair(),skipper> column_and_alias;
		quoted_string_parser<it,skipper> column_name;
	};

	template< typename it, typename skipper = qi::space_type>
	struct grammar_start_validation_helper_rule : qi::grammar<it,void(),skipper>
	{
		grammar_start_validation_helper_rule() : grammar_start_validation_helper_rule::base_type(start)
		{
			using namespace ::boost::spirit::qi;
			start = omit[qi::attr(0)];
		}
	private:
		::boost::spirit::qi::rule<it,void(),skipper> start;
	};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////SELECT//////////////////////////////////////////////////////////////////////////////////////////////

	template <typename action_str, typename it, typename skipper = qi::space_type>
	struct select_parser : qi::grammar<it, select_clause_container(), skipper>
	{
		select_parser(action_str const & action_ = select_action) : select_parser::base_type(select_clause), action(action_), filters_expr_header("filter")
		{
			using namespace ::boost::spirit::qi;
			select_clause %= grammar_start_validation_helper >
				'[' 
					> omit[action_psr(action)] > ','
					> distinct_psr // N.B. !!! The comma after DISTINCT is handled in distinct_psr if DISTINCT is present
					> columns_psr 
					// N.B. !!!
					// If the right part of the operator >> fails the iterator will return to the point before the left token of the operator >>.
					> ( lit(',') >> aggregates | attr(column_and_aggregate_container()) )
					> ( lit(',') >> as_expr | attr(column_and_alias_container()) )
					> ( lit(',') >> filters(filters_expr_header) | attr(column_to_filter_container()) )
					> ( lit(',') >> groups | attr(columns_container()) )
					> ( lit(',') >> groups_filters | attr(column_to_filter_container()) )
					> ( lit(',') >> order | attr(order_container()) )
					> ( lit(',') >> limit_and_offset | attr(limit_pair()) ) 
					// N.B. !!!
					// The first component in the expectation chain failure does not throw qi::expectation_failure exception.
					// Hence, false should be assigned to the default_all field. However, if a comma is present, it should follow valid default_all expression.
					// Otherwise, qi::expectation_failure is thrown.
					> ( lit(',') > map | attr(col_and_ref_col_container()) ) >
				']';
			BOOST_SPIRIT_DEBUG_NODE(select_clause);
		}
	private:
		::boost::spirit::qi::rule<it,select_clause_container(),skipper> select_clause;
		optional_comma<it,skipper> optional_comma;
		action_parser<it,skipper> action_psr;
		distinct_parser<it,skipper> distinct_psr;
		columns_parser<it,skipper> columns_psr;
		aggregates_parser<it,skipper> aggregates;
		as_parser<it,skipper> as_expr;
		filter_options_parser<it,skipper> filters;
		groups_parser<it,skipper> groups;
		group_filter<it,skipper> groups_filters;
		order_options_parser<it,skipper> order;
		limit_parser<it,skipper> limit_and_offset;
		map_parser<it,skipper> map;
		grammar_start_validation_helper_rule<it,skipper> grammar_start_validation_helper;
		::std::string const action, filters_expr_header;
	};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////INSERT//////////////////////////////////////////////////////////////////////////////////////////////

	template <typename it, typename skipper = qi::space_type>
	struct insert_value_parser : qi::grammar<it, insert_value_t(), skipper>
	{
		insert_value_parser() : insert_value_parser::base_type(insert_value_parser_)
		{
			using namespace ::boost::spirit::qi;
			insert_value_parser_ = quoted_string | real_parser<double,strict_real_policies<double>>() | int_ | bool_;
			BOOST_SPIRIT_DEBUG_NODE(insert_value_parser_);
		}
	  private:
		::boost::spirit::qi::rule<it, insert_value_t(), skipper> insert_value_parser_;
		quoted_string_parser<it,skipper> quoted_string;
	};

	template <typename action_str, typename it, typename skipper = qi::space_type>
	struct insert_parser : qi::grammar<it, insert_clause_container(), skipper>
	{
		insert_parser(action_str const & action_ = insert_action) : insert_parser::base_type(insert_clause), action(action_)
		{
			using namespace ::boost::spirit::qi;
			insert_clause %= grammar_start_validation_helper >
				'[' 
					> omit[action_psr(action)] > ',' 
					> columns 
					>	( lit(',') >> lit('"') >> "values" > '"' > ':' > 
							(
								lit('[') >> ']' 

								|

								'[' > value % ',' > ']'
							) | attr(insert_values_container())
						)
					// N.B. !!!
					// The first component in the expectation chain failure does not throw qi::expectation_failure exception.
					// Hence, false should be assigned to the default_all field. However, if a comma is present, it should follow valid default_all expression.
					// Otherwise, qi::expectation_failure is thrown.
					> ( lit(',') > '"' > no_case["default_all"] > '"' > ':' > bool_ | attr(false) ) >
				']';
			BOOST_SPIRIT_DEBUG_NODE(insert_clause);
		}
	private:
		::boost::spirit::qi::rule<it,insert_clause_container(),skipper> insert_clause;
		action_parser<it,skipper> action_psr;
		columns_parser<it,skipper> columns;
		insert_value_parser<it,skipper> value;
		grammar_start_validation_helper_rule<it,skipper> grammar_start_validation_helper;
		::std::string const action;
	};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////CREATE//////////////////////////////////////////////////////////////////////////////////////////////
	enum class sql_data_type {int_, double_, bool_, varchar, datetime};
	struct sql_data_type_wrapper
	{
		sql_data_type_wrapper(int const & v_ = 0): v(static_cast<sql_data_type>(v_))
		{
		}
		operator sql_data_type() const
		{
			return v;
		}
		sql_data_type v;
	};
	typedef ::std::vector<sql_data_type_wrapper> sql_types_container;
} // namespace mms
namespace std
{
	template<class traits>
	basic_ostream<char, traits>& operator<<(
												basic_ostream<char, traits> & out_stream, 
												mms::sql_data_type_wrapper const & sql_type_wrapper
											)
	{
		out_stream << static_cast<int>( sql_type_wrapper.v );
		return out_stream;
	}
}
namespace mms
{
	template <typename it, typename skipper = qi::space_type>
	struct sql_types_parser : qi::grammar<it, sql_types_container(), skipper>
	{
		sql_types_parser() : sql_types_parser::base_type(sql_types)
		{
			using namespace ::boost::spirit::qi;
			sql_type %= int_
						[
							_pass = (::boost::spirit::qi::_1 >= static_cast<int>(sql_data_type::int_) && 
							::boost::spirit::qi::_1 <= static_cast<int>(sql_data_type::datetime))
						];
			sql_types %= '"' > lit("types") > '"' > ':' > 
						'[' > 
								sql_type % ',' > 
						']';
			BOOST_SPIRIT_DEBUG_NODE(sql_types);
		}
	  private:
		  ::boost::spirit::qi::rule<it,int(),skipper> sql_type;
		  ::boost::spirit::qi::rule<it,sql_types_container(),skipper> sql_types;
	};
	enum class not_used_tag {not_used};
} // namespace mms
namespace std
{
	template<class traits>
	basic_ostream<char, traits>& operator<<(
												basic_ostream<char, traits> & out_stream, 
												mms::not_used_tag const &
											)
	{
		out_stream << "Not used DEFAULT value";
		return out_stream;
	}
}
namespace mms
{
	typedef ::boost::variant< ::std::string, int, double, bool, not_used_tag > constraint_def_value;
	typedef ::std::vector<constraint_def_value> constraint_def_value_container;
	struct constraint_container
	{
		constraint_container(
								bool const & null_ = false,
								bool const & primary_ = false,
								bool const & foreign_ = false,
								constraint_def_value const & def_value_ = not_used_tag::not_used
							): def_value(def_value_), null(null_), primary(primary_), foreign(foreign_)
		{
		}
		constraint_def_value def_value;
		bool null;
		bool primary;
		bool foreign;
	};
} // namespace mms

BOOST_FUSION_ADAPT_STRUCT(
	mms::constraint_container,
	(mms::constraint_def_value, def_value)
	(bool, null)
	(bool, primary)
	(bool, foreign)
)
namespace mms
{

	typedef ::std::vector<constraint_container> constraints_container;
	template <typename it, typename skipper = qi::space_type>
	struct constraint_value_parser : qi::grammar<it, constraint_def_value(), skipper>
	{
		constraint_value_parser() : constraint_value_parser::base_type(constraint_value_parser_)
		{
			using namespace ::boost::spirit::qi;
			constraint_value_parser_ = quoted_string | real_parser<double,strict_real_policies<double>>() | int_ | bool_;
			BOOST_SPIRIT_DEBUG_NODE(constraint_value_parser_);
		}
	  private:
		::boost::spirit::qi::rule<it, constraint_def_value(), skipper> constraint_value_parser_;
		quoted_string_parser<it,skipper> quoted_string;
	};
	template <typename it, typename skipper = qi::space_type>
	struct constraints_parser : qi::grammar<it, constraints_container(), skipper>
	{
		constraints_parser() : constraints_parser::base_type(constraints), null_header("null"), primary_key_header("primary"), foreign_key_header("foreign")
		{
			using namespace ::boost::spirit::qi;
			default_value = '"' >> no_case["default"] >> '"' > ':' > value;
			rule_template_with_bool_attr = '"' >> no_case[_r1] >> '"' > ':' > bool_;
			null = rule_template_with_bool_attr(null_header);
			primary_key = rule_template_with_bool_attr(primary_key_header);
			foreign_key = rule_template_with_bool_attr(foreign_key_header);
			constraint = 
							// N.B. !!!
							// If the right part of the operator >> fails the iterator will return to the point before the left token of the operator >>.
							lit('{') >> default_value[::boost::phoenix::at_c<0>(_val) = qi::_1] > 
														(
															',' >> null[::boost::phoenix::at_c<1>(_val) = qi::_1] >
															(
																',' >> primary_key[::boost::phoenix::at_c<2>(_val) = qi::_1] >
																(

																	',' >> foreign_key[::boost::phoenix::at_c<3>(_val) = qi::_1] > 
																	'}'

																	|

																	// foreign is not set
																	'}'

																) // default, null, primary 
															
																|

																// primary is not set
																',' >> foreign_key[::boost::phoenix::at_c<3>(_val) = qi::_1] > '}' 

																|

																// primary and foreign are not set
																'}'

															) // default, null

															|

															 // null is not set
															',' >> primary_key[::boost::phoenix::at_c<2>(_val) = qi::_1] >
															(

																	',' >> foreign_key[::boost::phoenix::at_c<3>(_val) = qi::_1] > '}'
																 
																	|

																	// foreign is not set
																	'}'

															) // default, primary

															| 

															// null and primary are not set
															',' >> foreign_key[::boost::phoenix::at_c<3>(_val) = qi::_1] > '}' 

															|

															// null, primary, foreign are not set 
															'}'

														) // the 1st is default 

							|

							lit('{') >> null[::boost::phoenix::at_c<1>(_val) = qi::_1] >
											(

												',' >> primary_key[::boost::phoenix::at_c<2>(_val) = qi::_1] >
												(

													',' >> foreign_key[::boost::phoenix::at_c<3>(_val) = qi::_1] > '}'

													|

													// foreign is not set
													'}'

												) // null, primary 
															
												|

												// primary is not set
												',' >> foreign_key[::boost::phoenix::at_c<3>(_val) = qi::_1] > '}' 

												|

												// primary and foreign are not set
												'}'

											) // the 1st is null

							|

							lit('{') >> primary_key[::boost::phoenix::at_c<2>(_val) = qi::_1] >
													(

													',' >> foreign_key[::boost::phoenix::at_c<3>(_val) = qi::_1] > '}' 

													|

													// foreign is not set
													'}'

													) // the 1st is primary
						
							|
							// the 1st is foreign
							lit('{') >> foreign_key[::boost::phoenix::at_c<3>(_val) = qi::_1] > '}' 
						
							|
							// all constraints are set by default
							lit('{') > '}';
			constraints %= '"' >> no_case[lit("constraints")] >> '"' > ':' > 
							(
								lit('[') >> ']'
							
								|

								'[' > constraint % ',' > ']'
							);
			BOOST_SPIRIT_DEBUG_NODE(constraints);
			BOOST_SPIRIT_DEBUG_NODE(constraint);
			BOOST_SPIRIT_DEBUG_NODE(default_value);
			BOOST_SPIRIT_DEBUG_NODE(null);
			BOOST_SPIRIT_DEBUG_NODE(primary_key);
			BOOST_SPIRIT_DEBUG_NODE(foreign_key);
		}
	  private:
		  ::boost::spirit::qi::rule<it,constraint_container(),skipper> constraint;
		  ::boost::spirit::qi::rule<it,constraint_def_value(),skipper> default_value;
		  ::boost::spirit::qi::rule<it,bool(::std::string const &),skipper> rule_template_with_bool_attr;
		  ::boost::spirit::qi::rule<it,bool(),skipper> null;
		  ::boost::spirit::qi::rule<it,bool(),skipper> primary_key;
		  ::boost::spirit::qi::rule<it,bool(),skipper> foreign_key;
		  ::boost::spirit::qi::rule<it,constraints_container(),skipper> constraints;
		  optional_comma<it,skipper> optional_comma;
		  constraint_value_parser<it,skipper> value;
		  grammar_start_validation_helper_rule<it,skipper> grammar_start_validation_helper;
		  ::std::string const null_header, primary_key_header, foreign_key_header;
	};
	typedef ::std::pair<unsigned, ::std::string> satellite_data_item_container;
	typedef ::std::pair<::std::string, satellite_data_item_container> column_and_satellite_data_item_container;
	typedef ::std::vector<column_and_satellite_data_item_container> columns_satellite_data_container;
	struct create_clause_container
	{
		columns_container columns;
		sql_types_container types;
		constraints_container constraints;
		columns_satellite_data_container satellite_data;
	};
} // namespace mms
BOOST_FUSION_ADAPT_STRUCT(
	mms::create_clause_container,
	(mms::columns_container, columns)
	(mms::sql_types_container, types)
	(mms::constraints_container, constraints)
	(mms::columns_satellite_data_container, satellite_data)
)
namespace mms
{
	template <class action_str, typename it, typename skipper = qi::space_type>
	struct create_parser : qi::grammar<it, create_clause_container(), skipper>
	{
		create_parser(action_str const & action_ = create_action) : create_parser::base_type(create_clause), action(action_)
		{
			using namespace ::boost::spirit::qi;
			item %= grammar_start_validation_helper > 
					'[' > 
					(
							uint_[_pass = qi::_1 > 0U] >> ',' > quoted_string > ']'  |
							uint_[_pass = qi::_1 > 0U] > ']' > attr("")  |
							attr(0U) >> quoted_string > ']'  |
							attr(0U) > attr("") > ']'
					);
			satellite_data %= grammar_start_validation_helper > '"' > lit("constraints_sdata") > '"' > ':' > 
				(		
					lit('[') >> ']' 
					
					|
					
					'[' >
						(
							'{' 
								> quoted_string > ':' > item > 
							'}'
						) % ',' > 
					']' | attr(columns_satellite_data_container())
				);
			create_clause %= grammar_start_validation_helper >
				'[' 
					> omit[action_psr(action)] > ','
					> columns > ','
					> sql_types
					> (lit(',') >> constraints | attr(constraints_container()))
					> (lit(',') > satellite_data | attr(columns_satellite_data_container())) > // if a comma is present, it necessarily follows satellite data
				']';
			BOOST_SPIRIT_DEBUG_NODE(create_clause);
			BOOST_SPIRIT_DEBUG_NODE(satellite_data);
			BOOST_SPIRIT_DEBUG_NODE(item);
		}
	private:
		::boost::spirit::qi::rule<it,create_clause_container(),skipper> create_clause;
		quoted_string_parser<it,skipper> quoted_string;
		::boost::spirit::qi::rule<it,satellite_data_item_container(),skipper> item;
		::boost::spirit::qi::rule<it,columns_satellite_data_container(),skipper> satellite_data;
		action_parser<it,skipper> action_psr;
		columns_parser<it,skipper> columns;
		sql_types_parser<it,skipper> sql_types;
		constraints_parser<it,skipper> constraints;
		optional_comma<it,skipper> optional_comma;
		grammar_start_validation_helper_rule<it,skipper> grammar_start_validation_helper;
		::std::string const action;
	};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////DELETE//////////////////////////////////////////////////////////////////////////////////////////////
	struct delete_clause_container
	{
		constraint_def_value_container values;
		column_to_filter_container filters;
	};
} // namespace mms

BOOST_FUSION_ADAPT_STRUCT(
	mms::delete_clause_container,
	(mms::column_to_filter_container, filters)
)
namespace mms
{
	template <typename it, typename skipper = qi::space_type>
	struct basic_value_parser : qi::grammar<it, value_t(), skipper>
	{
		basic_value_parser() : basic_value_parser::base_type(basic_value)
		{
			using namespace ::boost::spirit::qi;
			basic_value = quoted_string | real_parser<double,strict_real_policies<double>>() | int_ | bool_;
			BOOST_SPIRIT_DEBUG_NODE(basic_value);
		}
	  private:
		::boost::spirit::qi::rule<it, value_t(), skipper> basic_value;
		quoted_string_parser<it,skipper> quoted_string;
	};
	template <class action_str, typename it, typename skipper = qi::space_type>
	struct delete_parser : qi::grammar<it, delete_clause_container(), skipper>
	{
		delete_parser(action_str const & action_ = delete_action) : delete_parser::base_type(delete_clause), action(action_), filter_header("filter")
		{
			using namespace ::boost::spirit::qi;
			delete_clause %=
				grammar_start_validation_helper > 
				'[' 
					> omit[action_psr(action)] 
					> (lit(',') > filters(filter_header) | attr(column_to_filter_container())) > // if a comma is present, it necessarily follows filters
				']';
			BOOST_SPIRIT_DEBUG_NODE(delete_clause);
		}
	private:
		::boost::spirit::qi::rule<it,delete_clause_container(),skipper> delete_clause;
		action_parser<it,skipper> action_psr;
		filter_options_parser<it,skipper,basic_value_parser<it,skipper>> filters;
		grammar_start_validation_helper_rule<it,skipper> grammar_start_validation_helper;
		::std::string const action, filter_header;
	};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////UPDATE//////////////////////////////////////////////////////////////////////////////////////////////
	struct update_clause_container
	{
		columns_container columns;
		values_container values;
		column_to_filter_container filters;
	};
} // namespace mms
BOOST_FUSION_ADAPT_STRUCT(
	mms::update_clause_container,
	(mms::columns_container, columns)
	(mms::values_container, values)
	(mms::column_to_filter_container, filters)
)
namespace mms
{
	template <typename it, typename skipper = qi::space_type>
	struct values_parser : qi::grammar<it, values_container(), skipper>
	{
		values_parser() : values_parser::base_type(values)
		{
			using namespace ::boost::spirit::qi;
			values %= lit('"') > no_case["values"] > '"' > ':' > '[' > basic_value % ',' > ']';
			BOOST_SPIRIT_DEBUG_NODE(values);
		}
	private:
		basic_value_parser<it,skipper> basic_value;
		::boost::spirit::qi::rule<it,values_container(),skipper> values;
	};
	template <class action_str, typename it, typename skipper = qi::space_type>
	struct update_parser : qi::grammar<it, update_clause_container(), skipper>
	{
		update_parser(action_str const & action_ = update_action) : update_parser::base_type(update_clause), action(action_), filter_header("filter")
		{
			using namespace ::boost::spirit::qi;
			update_clause %= grammar_start_validation_helper >
				'[' 
					> omit[action_psr(action)] > ','
					> columns > ','
					> values
					> (lit(',') > filters(filter_header) | attr(column_to_filter_container())) > // if a comma is present, it necessarily follows filters
				']';
			BOOST_SPIRIT_DEBUG_NODE(update_clause);
		}
	private:
		::boost::spirit::qi::rule<it,update_clause_container(),skipper> update_clause;
		action_parser<it,skipper> action_psr;
		columns_parser<it,skipper> columns;
		values_parser<it,skipper> values;
		filter_options_parser<it,skipper> filters;
		grammar_start_validation_helper_rule<it,skipper> grammar_start_validation_helper;
		::std::string const action, filter_header;
	};

} // namespace mms

#endif // define parsers_hpp