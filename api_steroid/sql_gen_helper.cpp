#include "sql_gen_helper.h" // implements
#include "db.h" // uses

namespace mms
{
	struct filter_wrapper
	{
		enum used_filter {interval, set, like};
		filter_wrapper(used_filter const && filter_wrapper__) : used_filter_(filter_wrapper__)
		{
		}
		used_filter used_filter_;
	}; // struct filter_wrapper

	struct filter_visitor : boost::static_visitor<filter_wrapper>
	{
		filter_wrapper operator()(interval const &) const
		{
			return filter_wrapper(filter_wrapper::interval);
		}
		filter_wrapper operator()(set const &) const
		{
			return filter_wrapper(filter_wrapper::set);
		}
		filter_wrapper operator()(sql_query_string const &) const
		{
			return filter_wrapper(filter_wrapper::like);
		}
	}; // struct filter_visitor

	// TODO:
	// regex UTF-8 support.
	::std::cmatch action_regex(sql_query_string const sql_query, ::std::string const action_regexp)
	{
		::std::cmatch matches;
		::std::regex rx( ::std::string("action.*?(")+action_regexp+")" );
		::std::regex_search( sql_query.c_str(), matches, rx );
		return matches;
	}

	// TODO:
	// Literals in-place code points.
	void add_relation(sql_query_string & sql_q, relation const && relation_)
	{
		switch ( relation_ )
		{
		case relation::EQ: sql_q += "=";
			break;
		case relation::GEQ: sql_q += ">=";
			break;
		case relation::GRT: sql_q += ">";
			break;
		case relation::LEQ: sql_q += "<=";
			break;
		case relation::LESS: sql_q += "<";
			break;
		case relation::NEQ: sql_q += "!=";
			break;
		}
	}
	// TODO:
	// Literals : in-place UTF-8 code points.
	void add_interval_filter_concatenation_logic( sql_query_string & sql_q, logic const && logic_ )
	{
		switch ( logic_ )
		{
		case and_: sql_q += "and";
			break;
		case or_: sql_q += "or";
			break;
		}
	}
	// TODO:
	// Literals in-place code points.
	void add_column_aggregate( sql_query_string & sql_q, sql_query_string const & column, sql_faggregate const & aggr )
	{
		sql_q += space;
		switch ( aggr )
		{
		case AVG: sql_q += "avg";
			break;
		case COUNT: sql_q += "count";
			break;
		case FIRST: sql_q += "first";
			break;
		case LAST: sql_q += "last";
			break;
		case MAX: sql_q += "max";
			break;
		case MIN: sql_q += "min";
			break;
		case SUM: sql_q += "sum";
			break;
		}
		sql_q += opened_parenthesis, append_column_name( sql_q, column ), sql_q += closed_parenthesis;
	}

	sql_query_string get_table_name(sql_query_string const & column_name)
    {
        auto const & tokens = split_column( column_name );    
        if ( tokens.size() < 2 ) throw COLUMN_DELIMITER_WAS_NOT_FOUND;
        return tokens[0];
    }

    int add_table(column_name_t const & column, sql_query_string & sql_query)
    {
        sql_query_string tb_name;
        try
        {
            tb_name = get_table_name( column );
        }
        catch ( int const & code )
        {
            return code;
        }
        sql_query += space + tb_name;
        return SUCCESS;
    }

	void add_column_filter(sql_query_string & sql_q, sql_query_string const & column, interval const & i)
	{
		append_column_name( sql_q, column );
		add_relation( sql_q, static_cast<relation>(i.relation_with_min) );
		sql_q += space + ::boost::apply_visitor( value_visitor(), i.min_val ) + space;
		add_interval_filter_concatenation_logic( sql_q, static_cast<logic>(i.concatenation_logic) );
		append_column_name( sql_q, column );
		add_relation( sql_q, static_cast<relation>(i.relation_with_max) );
		sql_q += space + ::boost::apply_visitor( value_visitor(), i.max_val );
	}
	// TODO:
	// Literals in-place code point.
	void add_column_filter(sql_query_string & sql_q, sql_query_string const & column, set const & s)
	{
		append_column_name( sql_q, column ) += space + "in" + space + opened_parenthesis;
		for ( auto const & val : s ) sql_q += ::boost::apply_visitor( value_visitor(), val ) + comma;
		sql_q[sql_q.size() - 1] = closed_parenthesis.at(0);
	}
	// TODO:
	// Literals in-place code point.
	void add_column_filter(sql_query_string & sql_q, sql_query_string const & column, sql_query_string const & pattern)
	{
		append_column_name( sql_q, column ) += space + "like" + space + single_quote + pattern + single_quote;
	}

	splitted_vec_container split_column(column_name_t const & col)
	{
		splitted_vec_container split_vec;
		::boost::split( split_vec, col, ::boost::is_any_of(column_name_delim) );
		return split_vec;
	}

	void add_columns_and_aggregates(select_clause_container & select_clause, sql_query_string & sql_q )
	{
		if ( select_clause.columns.size() == 0 ) sql_q += space + wild_card;
		else
		{
			for ( auto const & column : select_clause.columns ) 
			{
				splitted_vec_container split_vec;
				::boost::split( split_vec, column, ::boost::is_any_of(column_name_delim) );
				if ( split_vec.size() < 2 ) throw COLUMN_DELIMITER_WAS_NOT_FOUND;
				columns_container columns(1, column);
				auto alias = column;
				if ( extract_alias( select_clause, column, alias ) ) columns.push_back(alias);
				sql_faggregate aggregate;
				if ( extract_aggregate( select_clause, columns, aggregate ) ) add_column_aggregate( sql_q, column, aggregate );
				else append_column_name( sql_q, column );
				if ( columns.size() == 2 ) sql_q += space + as + space + alias;
				sql_q += comma;
				columns.resize(0);
			}
			sql_q.resize( sql_q.size() - 1 ); // Delete the last comma.
		}
	}
	// TODO:
	// Literals in-place code point.
	void add_filters_helper(column_to_filter_container const && filters, sql_query_string & sql_q)
	{
		int count_of_col_filters = filters.size();
		for ( auto const & flt : filters )
		{
			auto flt_wrapper = ::boost::apply_visitor( filter_visitor(), flt.second );
			switch ( flt_wrapper.used_filter_ )
			{
			case filter_wrapper::interval: add_column_filter( sql_q, flt.first, ::boost::get<interval>(flt.second) );
				break;
			case filter_wrapper::set: add_column_filter( sql_q, flt.first, ::boost::get<set>(flt.second) );
				break;
			case filter_wrapper::like: add_column_filter( sql_q, flt.first, ::boost::get<sql_query_string>(flt.second));
				break;
			}
			if ( --count_of_col_filters != 0 ) sql_q += space + "and";
		}
	}

} // namespace mms