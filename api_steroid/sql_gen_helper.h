#ifndef sql_gen_helper_h
#define sql_gen_helper_h

// SYSTEM
#include <regex> // regex_search
#include <algorithm> // find
#include <map>
#include <unordered_set>
#include <vector>
#include <functional> // function
#include <string>
// LOCAL
#include "err_codes.h"
#include "parsers.hpp" // types
#include "policy.hpp" // SELECT compiler policies
// PROJECT
#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/get.hpp>
#include <boost/lexical_cast.hpp>


// TODO
// UTF-8 literals.

namespace mms
{
	// Types.
	typedef ::std::string gen_input_str;
	typedef ::std::function<void(gen_input_str)> gen_function;
	typedef ::std::string::const_iterator cit;
	typedef ::boost::spirit::qi::expectation_failure<cit> parser_exception;
	typedef ::std::vector <::std::string> splitted_vec_container;
	typedef ::std::vector <::std::string> splitted_vec_container;
	typedef ::std::string sql_query_string;

	void add_column_aggregate( sql_query_string & mms_sql_query, sql_query_string const & column, sql_faggregate const & aggr );

	struct filter_wrapper;
	struct filter_visitor;
	// Atoms.
	sql_query_string const 
		space = " ", 
		comma = ",",
		quote = "\"",
		underline = "_",
		column_name_delim = "@",
		opened_parenthesis = "(",
		closed_parenthesis = ")",
		opened_bracket = "[",
		closed_bracket = "]",
		opened_curly_bracket = "{",
		closed_curly_bracket = "}",
		semicolon = ";",
		colon = ":",
		as = "as",
		wild_card = "*",
		equality_ch = "=",
		varchar = "VARCHAR",
		boolean = "BOOLEAN",
		datetime = "DATETIME",
		integer = "INT",
		double_ = "DOUBLE",
		primary_key = "primary key",
		not = "not",
		null = "null",
		foreign_key = "foreign key",
		references = "references",
		at = "@",
		primary = "primary",
		true_ = "true",
		foreign = "foreign",
		single_quote = "'",
		default_ = "default",
		double_quote = "\"";
	// TODO:
	// Literals : in-place UTF-8 code points.
	struct value_visitor : ::boost::static_visitor<sql_query_string>
	{
		sql_query_string operator()(::std::string const & val) const
		{
			return quote + to_common_str(val) + quote;
		}
		sql_query_string operator()(int const & val) const
		{
			return to_common_str(val);
		}
		sql_query_string operator()(double const & val) const
		{
			return to_common_str(val);
		}
		sql_query_string operator()(bool const & val) const
		{
			return to_common_str(val);// == "1" ? "true" : "false";
		}
		sql_query_string operator()(column_and_aggregate const & col_and_aggr) const
		{
			sql_query_string col_and_aggr_str;
			add_column_aggregate( col_and_aggr_str, col_and_aggr.first, col_and_aggr.second );
			return col_and_aggr_str;
		}
		template<typename plain_t>
		sql_query_string to_common_str(plain_t const & val) const
		{
			return ::boost::lexical_cast<sql_query_string>(val);
		}

	}; // struct value_visitor

	// N.B.!!! Refer to http://www.boost.org/doc/libs/1_54_0/doc/html/variant/reference.html#variant.concepts.static-visitor for the StaticVisitor concept.

	struct constraint_def_val_visitor : value_visitor
	{
		std::string operator()(std::string const & val) const
		{
			return def() + value_visitor::operator()(val);
		}
		std::string operator()(double const & val) const
		{
			return def() + value_visitor::operator()(val);
		}
		std::string operator()(int const & val) const
		{
			return def() + value_visitor::operator()(val);
		}
		std::string operator()(bool const & val) const
		{
			return def() + value_visitor::operator()(val);
		}
		std::string operator()(mms::not_used_tag const & val) const
		{
			return space;
		}
		sql_query_string def() const
		{
			return  space + default_ + space;
		}
	}; // struct constraint_def_val_visitor

	// Functions.
	template<class query_string, class column_name_string>
	query_string & append_column_name( query_string & sql_q, column_name_string const & column )
	{
		sql_q += space + double_quote + column + double_quote + space;
		return sql_q;
	}
	::std::cmatch action_regex(::std::string const sql_query, ::std::string const action_regexp);
	
	inline sql_query_string make_quoted_string(sql_query_string const & column)
	{
		return quote + column + quote;
	}

	void add_relation(sql_query_string & sql_q, relation const && relation_);

	void add_interval_filter_concatenation_logic( sql_query_string & sql_q, logic const && logic_ );

	void add_column_aggregate( sql_query_string & sql_q, sql_query_string const & column, sql_faggregate const & aggr );

	void add_column_filter(sql_query_string & sql_q, sql_query_string const & column, interval const & i);

	void add_column_filter(sql_query_string & sql_q, sql_query_string const & column, set const & s);

	void add_column_filter(sql_query_string & sql_q, sql_query_string const & column, sql_query_string const & pattern);

	void add_column_aggregate( sql_query_string & sql_q, sql_query_string const & column, sql_faggregate const & aggr );

	splitted_vec_container split_column(column_name_t const & col);

	template <typename parser, typename parser_result_container>
	bool parse_action(action_str const & action, sql_query_string const mms_sql_query, parser_result_container & a_clause, unsigned const & len)
	{
		std::cmatch matches = action_regex( mms_sql_query, action );
		if ( matches.empty() ) return false;
		parser p(action);
		cit f = mms_sql_query.cbegin(), l = mms_sql_query.cend();
		try
		{
			::boost::spirit::qi::phrase_parse(f, l, p, ::boost::spirit::qi::space, a_clause);
		}
		catch( parser_exception const & )
		{
			return false;
		}
		return true;
	}

	template<class clause_container, class string>
	bool extract_alias( clause_container & select_clause, string const & column, string & alias )
	{
		auto const & cit = 
							std::find_if( 
											select_clause.aliases.cbegin(), 
											select_clause.aliases.cend(), 
											[&column](column_and_alias_pair const & col_and_alias)
											{
												return col_and_alias.first == column;
											}
										);
		if ( cit != select_clause.aliases.cend() )
		{
			alias = cit->second;
			select_clause.aliases.erase( cit );
			return true;
		}
		return false;
	}

	template<class clause_container, class aggregate>
	bool extract_aggregate( clause_container & clause, columns_container const & columns, aggregate & aggr )
	{
		return
				std::any_of(
								columns.cbegin(),
								columns.cend(),
								[&aggr,&clause](columns_container::value_type const & column)
								{
									auto cit =
												std::find_if(
															clause.aggregates.cbegin(),
															clause.aggregates.cend(),
															[&column](column_and_aggregate const & col_and_aggr)
															{
																return col_and_aggr.first == column;
															}
														 );
									if ( cit != clause.aggregates.cend() )
									{
										aggr = cit->second;
										clause.aggregates.erase( cit );
										return true;
									}
									return false;
								}
							);
	}

	void add_columns_and_aggregates(select_clause_container & select_clause, sql_query_string & sql_query );

	sql_query_string get_table_name(sql_query_string const & column_name);

	template<class clause_container>
    int add_table(clause_container const & clause, sql_query_string & sql_query)
    {
        sql_query_string tb_name;
        try
        {
            tb_name = get_table_name( clause.columns.at(0) );
        }
        catch ( int const & code )
        {
            return code;
        }
        sql_query += space + tb_name;
        return SUCCESS;
    }

    int add_table(column_name_t const & column, sql_query_string & sql_query);

	// TODO
	// UTF-8 literals.
	template <class clause_container>
	int add_cross_joined_tables( clause_container & clause, sql_query_string & sql_query )
	{
		sql_query_string const & cross_join_expr = "cross join";
		sql_query += " from";
		typedef std::unordered_set<std::string> used_tables_container;
		used_tables_container used_tables;
		splitted_vec_container split_vec;
		for ( auto const & col : clause.columns )
		{
			::boost::split( split_vec, col, ::boost::is_any_of(column_name_delim) );
			try
			{
				auto const & table = split_vec.at(0);
				if ( used_tables.find( table ) == used_tables.end() )
				{
					used_tables.insert( table );
					sql_query += space + table + space + cross_join_expr; 
				}
			}
			catch ( std::out_of_range & )
			{
				return COLUMN_DELIMITER_WAS_NOT_FOUND;
			}
		}
		sql_query.resize( sql_query.size() - cross_join_expr.size() - space.size() );
		return SUCCESS;
	}

	// N.B. !!!
	// Append where clause. Comparison is done using either logical comparison operators with and,or or the keyword in.
	// By the way of not employing the method 'which' of the variant class, the filter wrapper is used. 
	// After the filter visitor is applied to the filter, the wrapper is returned and the filter type has aready been set inside it.
	// Thereafter the method get is used to explicitly retrieve the actual filter. 
	// Thus, the dependency on the order of the filter types in the variant is omitted. 

	// N.B. !!! P.S. If you got better solution, please e-mail to alexander.svk@gmail.com.
	void add_filters_helper(column_to_filter_container const && filters, sql_query_string & mms_sql_query);

	// TODO
	// UTF-8
	template <typename clause_container>
	void add_defined_filters(clause_container const & clause_, sql_query_string & sql_query, bool const & group_filter = false)
	{
		if ( clause_.groups.size() ) // N.B. !!! The sufficient condition for HAVING expression.
									// => http://www.sqlite.org/lang_select.html
		{
			sql_query += group_filter ? " having" : " where";
			if ( !group_filter && clause_.filters.size() == 0 || group_filter && clause_.group_filters.size() == 0 ) sql_query += space + "1";
			else add_filters_helper(group_filter ? ::std::move( clause_.group_filters ) : ::std::move( clause_.filters ), sql_query);
		}
	}
	// TODO
	// UTF-8
	template <typename clause_container>
	void add_filters(clause_container const & clause, sql_query_string & sql_query)
	{
		sql_query += " where";
		if ( clause.filters.size() == 0 ) sql_query += space + "1";
		else add_filters_helper( ::std::move( clause.filters ), sql_query);
	}
	// TODO
	// UTF-8
	template<class clause_container>
	void add_groups( clause_container const & clause, sql_query_string & sql_query )
	{
		if ( clause.groups.size() ) 
		{
			sql_query_string const group_by_str = "group by";
			sql_query += space + group_by_str;
			for ( auto const & column : clause.groups ) append_column_name( sql_query, column ) += comma; // Form group by comma sep. list.
			sql_query.resize( sql_query.size() - 1 );
		}
	}
	// TODO
	// UTF-8
	template<class clause_container>
	void add_order( clause_container const & clause, sql_query_string & sql_query )
	{
		if ( clause.order_options.columns.size() ) 
		{
			sql_query_string const order_by_str = "order by", asc_str = "asc", desc_str = "desc";
			sql_query += space + order_by_str;
			for ( auto const & column : clause.order_options.columns ) append_column_name( sql_query, column ) += comma; // Form order by comma sep. list.
			sql_query.resize( sql_query.size() - 1 ); // delete the last comma
			sql_query += space + (static_cast<order>(clause.order_options.order) == order::asc ? asc_str : desc_str);
		}
	}
    // TODO
    // UTF-8
    template<class clause_container>
    void add_limit_and_offset( clause_container const & clause, sql_query_string & sql_query )
    {
        if ( clause.limit_options != limit_pair() ) // zero-initialization
        {
            sql_query_string const limit_str = "limit";
            sql_query += space + limit_str;
            sql_query += space + ::boost::lexical_cast<sql_query_string>(clause.limit_options.first);
            sql_query += comma;
            sql_query += space + ::boost::lexical_cast<sql_query_string>(clause.limit_options.second) + semicolon;
        }
    }



	/*!
	\brief Stores data into data_raw_ptr newly allocated memory.
	\attention The newly allocated memory takes the size of data plus one for the 2D table - *data_raw_ptr.
	\param data Data to be copied.
	\param data_raw_ptr Reference to the double raw pointer of the newly allocated memory.
	\returns An execution result code (refer to err_codes.h for more details).
	*/
	template<class data_container, typename raw_double_pointer>
	int allocate(data_container const & data, raw_double_pointer* data_raw_ptr)
	{
		typedef char select_return_val_atom;
		typedef select_return_val_atom* select_return_val_single_ptr;
		*data_raw_ptr = new (::std::nothrow) select_return_val_single_ptr[data.size()];
		if ( *data_raw_ptr == nullptr ) return MEM_ALLOC;
		auto idx = 0U;
		raw_double_pointer data_double_raw_ptr = *data_raw_ptr;
		for ( auto const & str : data )
		{
			data_double_raw_ptr[idx] = new (::std::nothrow) select_return_val_atom[str.size() + 1];
			if ( data_double_raw_ptr[idx] == nullptr ) 
			{
				deallocate( data_double_raw_ptr, idx );
				return MEM_ALLOC;
			}
			strcpy_s( data_double_raw_ptr[idx++], str.size() + 1, str.c_str() );
		}
		return SUCCESS;
	}

	/*!
	\brief Deallocates raw_p pointed memory of the size.
	\param raw_p Double raw pointer.
	\param size The number of rows in the 2D table - raw_p.
	*/
	template<class raw_double_pointer>
	void deallocate( raw_double_pointer raw_p, unsigned const & size )
	{
		for ( auto idx = 0U; idx < size; delete [] raw_p[idx++] );
		delete [] raw_p;
	}

	/*!
	\brief Creates a table using sql_query in a data base located in db_path.
	\param sql_query MMS SQL query.
	\param db_path The path to a data base.
	\returns Result code.
	*/
	template<class creator, typename create_return_val>
	create_return_val create_impl(sql_query_string const & sql_query, ::std::string const & db_path)
	{
		try
		{
			creator creator_( sql_query );
			creator_.create_table( db_path );
		}
		catch ( int const & code )
		{
			return static_cast<create_return_val>( code );
		}
		return static_cast<create_return_val>( SUCCESS );
	}

	template<class selection_raw_pointer,class data_container>
	selection_raw_pointer convert_data_container_to_selection_raw_pointer(data_container & data)
	{
		try
		{
			data.push_front( ::boost::lexical_cast<mms::selector::data_atom>(data.size()) );
		}
		catch ( ::boost::bad_lexical_cast & ) // -> ::std::bad_cast -> ::std::exception
		{
			return reinterpret_cast<selection_raw_pointer>( BOOST_LEXICAL_CAST ); 
		}
		catch ( ::std::bad_alloc & ) // -> ::std::exception
		{
			return reinterpret_cast<selection_raw_pointer>( STD_PUSH_BACK_MEM_ALLOC );
		}
		int err_code = 0;
		selection_raw_pointer data_raw_ptr = nullptr;
		if ( ( err_code = mms::allocate( data, &data_raw_ptr ) ) != SUCCESS ) return reinterpret_cast<selection_raw_pointer>( err_code );
		return data_raw_ptr;	
	}

	/*!
	\brief Selects data using sql_query from a data base located in db_path.
	\param mms_sql_query MMS SQL query.
	\param db_path The path to a data base.
	\returns Newly allocated data.
	
	\pre selector should inherit mms::selector class.
	\pre selector should contain method select(data_base_path) and throw exception of type int containing error codes from the err_codes.h.

	*/
	template<class selector, typename select_return_val_t>
	select_return_val_t select_impl(sql_query_string const & mms_sql_query, ::std::string const & db_path)
	{
		typename selector::data_deque_container data;
		try
		{
			selector selector_( mms_sql_query );
			data = selector_.select( db_path );
		}
		catch ( int const & code )
		{
			return reinterpret_cast<select_return_val_t>( code );
		}
		return convert_data_container_to_selection_raw_pointer<select_return_val_t>(data);
	}

	/*!
	\brief Inserts data using sql_query into a data base located in db_path.
	\param sql_query MMS SQL query.
	\param db_path The path to a data base.
	\returns Result code.
	*/
	template<class inserter, typename insert_return_val>
	insert_return_val insert_impl(sql_query_string const & sql_query, ::std::string const & db_path)
	{
		try
		{
			inserter inserter_( sql_query );
			return inserter_.insert( db_path );
		}
		catch ( int const & code )
		{
			return static_cast<insert_return_val>( code );
		}
	}

	/*!
	\brief Updates data using sql_query in a data base located in db_path.
	\param sql_query MMS SQL query.
	\param db_path The path to a data base.
	\returns Result code.
	*/
	template<class updater, typename update_return_val>
	update_return_val update_impl(sql_query_string const & sql_query, ::std::string const & db_path)
	{
		try
		{
			updater updater_( sql_query );
			updater_.update( db_path );
		}
		catch ( int const & code )
		{
			return static_cast<update_return_val>( code );
		}
		return SUCCESS;
	}

	/*!
	\brief Deletes data using sql_query from a data base located in db_path.
	\param sql_query MMS SQL query.
	\param db_path The path to a data base.
	\returns Result code.
	*/
	template<class deleter, typename delete_return_val>
	delete_return_val delete_impl(sql_query_string const & sql_query, ::std::string const & db_path)
	{
		try
		{
			deleter deleter_( sql_query );
			deleter_.delete_( db_path );
		}
		catch ( int const & code )
		{
			return static_cast<delete_return_val>( code );
		}
		return SUCCESS;
	}
} // namespace mms
#define GENERATE_MMS_MEDIATOR_AND_FRIEND(name, selector_impl, ...)\
	namespace mms\
	{\
		class name: public mediator_and_friend<selector_impl>\
		{\
		public:\
			typedef mediator_and_friend<selector_impl> base_type;\
			name(sql_query_string const & mms_sql_query):base_type(__VA_ARGS__,mms_sql_query)\
			{\
			}\
		};\
	}
#define GENERATE_MMS_MEDIATOR(name, selector_impl, ...)\
	namespace mms\
	{\
		class name: public mediator<selector_impl>\
		{\
		public:\
			typedef mediator<selector_impl> base_type;\
			name(sql_query_string const & mms_sql_query):base_type(__VA_ARGS__,mms_sql_query)\
			{\
			}\
		};\
	}

#define GENERATE_MMS_BASIC_CREATOR(creator_impl)\
	namespace mms\
	{\
		class basic_creator: public ::creator_impl\
		{\
		public:\
			basic_creator(sql_query_string const & mms_sql_query): ::creator_impl(create_action,mms_sql_query)\
			{\
			}\
		};\
	}

#define GENERATE_MMS_BASIC_SELECTOR(selector_impl)\
	namespace mms\
	{\
		class basic_selector: public ::selector_impl\
		{\
		public:\
			basic_selector(sql_query_string const & mms_sql_query): ::selector_impl(select_action,mms_sql_query)\
			{\
			}\
		};\
	}

#define GENERATE_MMS_BASIC_INSERTER(inserter_impl)\
	namespace mms\
	{\
		class basic_inserter: public ::inserter_impl\
		{\
		public:\
			basic_inserter(sql_query_string const & mms_sql_query): ::inserter_impl(insert_action,mms_sql_query)\
			{\
			}\
		};\
	}

#define GENERATE_MMS_BASIC_UPDATER(updater_impl)\
	namespace mms\
	{\
		class basic_updater: public ::updater_impl\
		{\
		public:\
			basic_updater(sql_query_string const & mms_sql_query): ::updater_impl(update_action,mms_sql_query)\
			{\
			}\
		};\
	} 

#define GENERATE_MMS_BASIC_DELETER(deleter_impl)\
	namespace mms\
	{\
		class basic_deleter: public ::deleter_impl\
		{\
		public:\
			basic_deleter(sql_query_string const & mms_sql_query): ::deleter_impl(delete_action,mms_sql_query)\
			{\
			}\
		};\
	}
#endif
