#define DEBUG
// LOCAL
#include "measurement_auto_creator.h" // implements
#include "sql_gen_helper.h" // uses
#include "err_codes.h" // uses
#include "sqlite3_selector.h" // uses
// PROJECT
#include <boost/lexical_cast.hpp> // uses

namespace mms
{
	sqlite3_measurement_auto_creator::data_container sqlite3_measurement_auto_creator::select(::mms::sql_query_string const & query) const
	{
		sqlite3_selector selector("");
		auto const & code = selector.select_from_db( query, db_path );
		if ( code != SUCCESS ) throw code;
		auto const & data_ = selector.get_data();
		return data_container(data_.cbegin(), data_.cend());
	}

	measurement_auto_creator::module_dependent_column_container measurement_auto_creator::get_module_dependent_columns() const
	{
		module_dependent_column_container mdc;
		try
		{
			auto sel_data = select( "select module_id from module where module_name = '" + module_name + "';" );
			if ( sel_data.size() == 0 ) throw MEASUREMENT_AUTO_CREATOR_NO_MODULE_WAS_FOUND;
			if ( sel_data.size() != 1 ) throw MEASUREMENT_AUTO_CREATOR_MORE_THAN_ONE_MODULE;
			int module_id;
			::mms::sql_query_string module_id_str;
			try
			{
				module_id = ::boost::lexical_cast<int>(sel_data[0]);
			}
			catch ( ::boost::bad_lexical_cast & )
			{
				throw MEASUREMENT_AUTO_CREATOR_UKNOWN_DATA_WAS_SELECTED;
			}
			module_id_str = sel_data.at(0);
			sel_data = select( "select mip_group_module_group_id from mip_group where mip_group_module = " + module_id_str + ";" );
			for ( auto const & id : sel_data ) 
			{
				column_name_t const & column_name = "mip" + underline + id;
				mdc.push_back( module_dependent_column(
															column_name, 
															opened_curly_bracket + quote + foreign + quote + colon + true_ + closed_curly_bracket, 
															opened_curly_bracket + quote + table_name + column_name_delim + column_name + quote + colon + opened_bracket + quote + "mip_group" + column_name_delim + "module_group_id" + quote + closed_bracket + closed_curly_bracket
													  ) );
			}
			sel_data = select( "select mrp_parameter_id from mrp_parameter join mrp_group on mrp_parameter_mrp_group = mrp_group_id where mrp_group_module = " + module_id_str + ";" );
			for ( auto const & id : sel_data ) 
			{
				column_name_t const & column_name = "mrp" + underline + id;
				mdc.push_back( module_dependent_column(
															column_name, 
															opened_curly_bracket + quote + foreign + quote + colon + true_ + closed_curly_bracket, 
															opened_curly_bracket + quote + table_name + column_name_delim + column_name + quote + colon + opened_bracket + quote + "mrp_parameter" + column_name_delim + "id" + quote + closed_bracket + closed_curly_bracket
													  ) );
			}
		}
		catch ( int const & )
		{
			throw;
		}
		return mdc;
	}
	void measurement_auto_creator::do_()
	{
		module_dependent_column_container module_dependent_columns;
		try
		{
			module_dependent_columns = get_module_dependent_columns();
		}
		catch ( int const & )
		{
			throw;
		}
		::mms::sql_query_string const & tb = table_name;
		mms_sql_query = opened_bracket + quote + "action" + quote + colon + quote + "create" + quote + comma + quote + "columns" + quote + colon + opened_bracket;
		mms_sql_query += quote + tb + column_name_delim + "id" + quote;
		add_column( "project" ), add_column( "visit" ), add_column( "file" ), add_column( "date" ), add_column( "repetition" ), add_column( "subject" ), add_column( "subject_height" ), add_column( "subject_weight" ), add_column( "subject_foot_size" ), add_column( "subject_dominant_leg" ), add_column( "subject_dominant_arm" ), add_column( "subject_type" ), add_column( "subject_type_l1" ), add_column( "subject_type_l2" );
#ifdef DEBUG
		std::cout << mms_sql_query << '\n';
#endif
		for ( auto const & module_dependent_column : module_dependent_columns ) add_column( module_dependent_column.name );
		mms_sql_query += closed_bracket + comma + quote + "types" + quote + colon + opened_bracket;
		mms_sql_query += "0" + comma + "0" + comma + "0" + comma + "3" + comma + "3" + comma + "0" + comma + "0" + comma + "1" + comma + "1" + comma + "1" + comma + "0" + comma + "0" + comma + "0" + comma + "0" + comma + "0";
		for ( auto const & module_dependent_column : module_dependent_columns ) if ( module_dependent_column.name.substr(0,3) == "mip" ) mms_sql_query += comma + "0"; else if ( module_dependent_column.name.substr(0,3) == "mrp" ) mms_sql_query += comma + "1";
		mms_sql_query += closed_bracket;
#ifdef DEBUG
		std::cout << mms_sql_query << '\n';
#endif
		mms_sql_query += comma + quote + "constraints" + quote + colon + opened_bracket;
		mms_sql_query += opened_curly_bracket + quote + primary + quote + colon + true_ + closed_curly_bracket; // measurement_id
		for ( auto idx = 0U; idx < 2U; ++idx ) add_foreign_constraint(); // project, visit
		for ( auto idx = 0U; idx < 3U; ++idx ) add_empty_constraint(); // file, date, repetition
		add_foreign_constraint(); // subject
		for ( auto idx = 0U; idx < 3U; ++idx ) add_empty_constraint(); // subject_height, subject_weight, subject_foot_size
		for ( auto idx = 0U; idx < 5U; ++idx ) add_foreign_constraint(); // subject_dominant_leg, subject_dominant_arm, subject_type, subject_type_l1, subject_type_l2
		for ( auto const & module_dependent_column : module_dependent_columns ) mms_sql_query += comma + module_dependent_column.constraint;
#ifdef DEBUG
		std::cout << mms_sql_query << '\n';
#endif
		mms_sql_query += closed_bracket + comma + quote + "constraints_sdata" + quote + colon + opened_bracket;
		add_foreign_sdata( tb + column_name_delim + "project", "project" + column_name_delim + "id" ), mms_sql_query += comma, 
			add_foreign_sdata( tb + column_name_delim + "visit", "visit" + column_name_delim + "id" ), mms_sql_query += comma, 
			add_varchar_sdata( tb + column_name_delim + "file", text_length ), mms_sql_query += comma,
			add_varchar_sdata( tb + column_name_delim + "date", text_length ), mms_sql_query += comma,
			add_foreign_sdata( tb + column_name_delim + "subject", "subject" + column_name_delim + "id" ), mms_sql_query += comma,
			add_foreign_sdata( tb + column_name_delim + "subject_dominant_leg", "dominant_leg" + column_name_delim + "id" ), mms_sql_query += comma,
			add_foreign_sdata( tb + column_name_delim + "subject_dominant_arm", "dominant_arm" + column_name_delim + "id" ), mms_sql_query += comma,
			add_foreign_sdata( tb + column_name_delim + "subject_type", "subject_type" + column_name_delim + "id" ), mms_sql_query += comma,
			add_foreign_sdata( tb + column_name_delim + "subject_type_l1", "subject_type_l1" + column_name_delim + "id" ), mms_sql_query += comma,
			add_foreign_sdata( tb + column_name_delim + "subject_type_l2", "subject_type_l2" + column_name_delim + "id" ); 
		for ( auto const & module_dependent_column : module_dependent_columns ) mms_sql_query += comma + module_dependent_column.sdata;
		mms_sql_query += closed_bracket + closed_bracket;
#ifdef DEBUG
		std::cout << mms_sql_query << '\n';
#endif
		try
		{
			create_table(db_path);
		}
		catch ( int const & )
		{
			throw;
		}
	}
} // namespace mms