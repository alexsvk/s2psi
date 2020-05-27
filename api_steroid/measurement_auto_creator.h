#ifndef measurement_auto_creator_h
#define measurement_auto_creator_h

// SYSTEM
#include <vector> // uses
#include <string> // uses
// LOCAL
#include "mms_auto_creator.h" // inherits
#include "sql_gen_helper.h" // uses

namespace mms
{

class measurement_auto_creator : public virtual auto_creator
{
public:
	measurement_auto_creator(::mms::sql_query_string const & module_name_, std::string const & db_path_, ::mms::sql_query_string const & text_len = "4096"): 
		creator(""), module_name( module_name_ ), table_name( "measurement" + underline + module_name ), db_path( db_path_ ), text_length( text_len )
	{
	}
	void do_() override;
protected:
	typedef std::vector<std::string> data_container;
	virtual data_container select(::mms::sql_query_string const & query) const = 0;
	std::string const db_path;
private:
	void add_column(::mms::sql_query_string const & column_name)
	{
		mms_sql_query += comma + quote + table_name + column_name_delim + column_name + quote;
	}
	void add_foreign_constraint()
	{
		mms_sql_query += comma + opened_curly_bracket + quote + foreign + quote + colon + true_ + closed_curly_bracket;
	}
	void add_empty_constraint()
	{
		mms_sql_query += comma + opened_curly_bracket + closed_curly_bracket;
	}
	void add_foreign_sdata(::mms::sql_query_string const & column_name, ::mms::sql_query_string const & ref_column_name)
	{
		mms_sql_query += opened_curly_bracket + quote + column_name + quote + colon + opened_bracket + quote + ref_column_name + quote + closed_bracket + closed_curly_bracket;
	}
	void add_varchar_sdata(::mms::sql_query_string const & column_name, ::mms::sql_query_string const & length_str)
	{
		mms_sql_query += opened_curly_bracket + quote + column_name + quote + colon + opened_bracket + length_str + closed_bracket + closed_curly_bracket;
	}
	struct module_dependent_column
	{
		module_dependent_column(::mms::sql_query_string const & name_, ::mms::sql_query_string const & constraint_ = "", ::mms::sql_query_string const & sdata_ = ""):
			name(name_), constraint(constraint_), sdata(sdata_)
		{
		}
		::mms::sql_query_string name;
		::mms::sql_query_string constraint;
		::mms::sql_query_string sdata;
	};
	typedef std::vector<module_dependent_column> module_dependent_column_container;
	module_dependent_column_container get_module_dependent_columns() const;

	::mms::sql_query_string const module_name, table_name, text_length;
};


class sqlite3_measurement_auto_creator : public measurement_auto_creator, public sqlite3_creator
{
public:
	sqlite3_measurement_auto_creator(::mms::sql_query_string const & module_name_, std::string const & db_path_, ::mms::sql_query_string const & text_len = "4096") : 
		creator("create"), measurement_auto_creator(module_name_,db_path_,text_len), sqlite3_creator("")
	{
	}
protected:
	data_container select(::mms::sql_query_string const & query) const override;
	int create_table_in_db(sql_query_string const & sql_query, std::string const & db_path) 
	{
		return sqlite3_creator::create_table_in_db(sql_query,db_path);
	}
};

} // namespace mms

#endif