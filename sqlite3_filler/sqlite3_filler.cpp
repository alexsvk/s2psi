// sqlite3_filler.cpp : Defines the entry point for the console application.
//
#pragma comment(lib,"..\\Debug\\sqlite3")
// PROJECT
#include <boost/lexical_cast.hpp>
// SYSTEM
#include <string>
#include <vector>
#include <random>
// LOCAL
#include "db.h"

#define TERMINATE(exception) std::cerr << "The app. has to terminate : " << e.what()
#define NO_DB_PATH -2
#define NO_TABLES -1

struct column_info
{
	bool pk;
	mms::db::sql_q name;
	mms::db::sql_q type;
};

int generate_int(int const & min = 0, int const & max = 0)
{
    // Seed with a real random value, if available.
    std::random_device rd;
 
    // Choose a random mean between 1 and 6.
    std::default_random_engine e1(rd());
    std::uniform_int_distribution<int> uniform_dist(1, 6);
    int mean = uniform_dist(e1);
 
    // Generate a normal distribution around that mean.
    std::mt19937 e2(rd());
    std::normal_distribution<> normal_dist(mean, 2);
	int val = max != 0 ? static_cast<int>(normal_dist(e2)) % max : normal_dist(e2);
	return val;
}

std::string generate_text()
{
	char const alphabet[] = "abcdefghijklmnoprstuvwxyz";
	std::string text;
	unsigned const & word_size = 10U;
	for ( auto idx = 0U; idx < word_size; ++idx ) text += alphabet[generate_int(0, sizeof(alphabet))];
	return text;
}

double generate_real()
{
	return generate_int() + generate_int() / 10.0;
}

std::string generate_blob()
{
	char alphabet[] = {0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6};
	std::string blob;
	unsigned const & blob_size = 10U;
	for ( auto idx = 0U; idx < blob_size; ++idx ) blob += alphabet[generate_int(0, sizeof(alphabet))];
	return blob;
}

template<class mms_sql_query>
mms_sql_query generate_value( mms_sql_query const & type )
{
	mms_sql_query const & 
		int_ = "INTEGER",
		text = "TEXT",
		real = "REAL",
		blob = "BLOB";
	mms_sql_query const quote = "'";
	if ( type.compare( int_ ) == 0 ) return ::boost::lexical_cast<mms_sql_query>(generate_int());
	else if ( type.compare( text ) == 0 ) return quote + ::boost::lexical_cast<mms_sql_query>(generate_text()) + quote;
	else if ( type.compare( real ) == 0 ) return ::boost::lexical_cast<mms_sql_query>(generate_real());
	else if ( type.compare( blob ) == 0 ) return ::boost::lexical_cast<mms_sql_query>(generate_blob());
	return mms_sql_query();
}

template<class col_info_container,class columns_info_container>
void query_construct_helper( 
								std::function<mms::db::sql_q(col_info_container const &)> const & col_info_handler, 
								columns_info_container const & columns_info, 
								mms::db::sql_q & query 
						   )
{
	for ( auto const & col_info : columns_info ) if ( !col_info.pk ) query += col_info_handler(col_info);
	if ( columns_info.size() != 0 ) query.resize(query.size()-1);
}

template<class db_t, class table, class columns_info_t>
bool add_record( db_t & db, table const & tb, columns_info_t const & columns_info )
{
	typename db_t::sql_q query = "insert into " + tb + "(";
	try
	{
		query_construct_helper<column_info>( []( column_info const & col_info )->mms::db::sql_q
		{
			return "'" + col_info.name + "'" + ',';
		}, columns_info, query);
		query += ") values(";
		query_construct_helper<column_info>( []( column_info const & col_info )->mms::db::sql_q
		{
			return generate_value( col_info.type ) + ',';
		}, columns_info, query);
		query += ");";
		db.insert( query );
	}
	catch ( std::runtime_error & e )
	{
		std::cerr << '\n' << query << '\n';
		TERMINATE(e);
		return false;
	}	
	return true;
}

int table_selected_data_handle(void* par, int col_cnt, char** data, char** col_names)
{
	try
	{
		if ( par != nullptr )
		{
			typedef std::pair<unsigned,std::pair<std::vector<mms::db::sql_q>,mms::db::sql_q>> param;
			auto par_ptr = static_cast<param*>(par);
			if ( col_cnt > 0 && data != nullptr && par_ptr->second.second.compare( col_names[0] ) == 0 ) 
			{
				++par_ptr->first;		
				try
				{
					par_ptr->second.first.push_back( data[0] );
				}
				catch ( std::bad_alloc & e )
				{
					TERMINATE(e);
					return 1; // SQLITE_ABORT is returned by sqlite3_exec	
				}
				return 0; // SQLITE_OK is returned by sqlite3_exec
			}
		}
	}
	catch ( ::boost::bad_lexical_cast & e )
	{
		TERMINATE(e);
	}
	return 1; // SQLITE_ABORT is returned by sqlite3_exec
}

template<class table_container>
int select_tables(mms::db & db, table_container & tables, unsigned & count_of_tables )
{
	mms::db::sql_q sqlite_master_name_column = "name";
	mms::db::sql_q count_of_tables_query = "select name from sqlite_master where type = 'table';";
	auto & data_handle_param = std::make_pair( count_of_tables, std::make_pair( tables, sqlite_master_name_column ) );
	db.select( count_of_tables_query, table_selected_data_handle, &data_handle_param );
	count_of_tables = data_handle_param.first, tables = data_handle_param.second.first;
	return 0;
}

int selected_columns_info_handle( void* par, int cols_cnt, char** data, char** col_names )
{
	mms::db::sql_q const & 
		table_info_name_column = "name", 
		table_info_type_column = "type", 
		table_info_pk_column = "pk";
	int const table_info_columns_cnt = 6;
	if ( 
			par != nullptr &&
			cols_cnt == table_info_columns_cnt && 
			data != nullptr && 
			col_names != nullptr &&
			table_info_name_column.compare( col_names[1] ) == 0 &&
			table_info_type_column.compare( col_names[2] ) == 0  &&
			table_info_pk_column.compare( col_names[5] ) == 0
	   ) 
	{
		try
		{
			typedef std::vector<column_info> columns_info_container;
			auto par_ptr = static_cast<columns_info_container*>(par);
			column_info ci = {std::strcmp(data[5],"1") == 0 ? true : false, data[1], data[2]};
			par_ptr->push_back( ci );
		}
		catch ( std::bad_alloc & e )
		{
			TERMINATE(e);
			return 1; // SQLITE_ABORT is returned by sqlite3_exec	
		}
		return 0; // SQLITE_OK is returned by sqlite3_exec
	}
	return 1; // SQLITE_ABORT is returned by sqlite3_exec	
}

template<class columns_info_container>
int select_columns_info(mms::db & db, mms::db::sql_q const & tb, columns_info_container & columns_info)
{
	mms::db::sql_q table_info_query = "pragma table_info(";
	db.select( table_info_query + '\'' + tb + "');", selected_columns_info_handle, &columns_info);
	return 0;
}

template<class table_container>
int insert(mms::db & db, table_container const & tables,unsigned const & count_of_records)
{
	int err_code = 0;
	for ( auto const & tb : tables )
	{
		std::cout << tb << " is being inserted..." << '\n';
		typedef std::vector<column_info> column_info_container;
		column_info_container columns_info;
		if ( (err_code = select_columns_info( db, tb, columns_info )) != 0 ) return err_code;
		for ( auto idx = 0U; idx < count_of_records; ++idx ) 
			if ( !add_record( db, tb, columns_info ) ) 
			{
				err_code = 1;
				std::cerr << "\nFailed to add record #" << idx << " for the table " << tb << '\n';
				break;
			}
	}
	return err_code;
}
#define PARAMS_COUNT 3
int main(int argc, char* argv[])
{
	if ( argc == PARAMS_COUNT )
	{
		try
		{
			auto const & db_path = ::boost::lexical_cast<mms::db::source>( argv[1] );
			mms::db db;
			db.open( db_path );
			auto count_of_tables = 0U;
			typedef std::vector<mms::db::sql_q> table_container;
			table_container tables;
			int err_code = 0;
			if ( (err_code = select_tables( db, tables, count_of_tables )) != 0 ) return err_code;
			if ( count_of_tables == 0 )
			{
				std::cout << "\nThere are no tables !!!\n";
				return NO_TABLES;
			}
			std::cout << '\n' << count_of_tables << " tables are in the data base located in " << db_path << '\n';
			unsigned const & count_of_records = ::boost::lexical_cast<unsigned>( argv[2] );
			err_code = insert( db, tables, count_of_records );
		}
		catch ( std::exception & e ) // -> std::exception
		{
			TERMINATE(e);
		}
	}
	else 
	{
		std::cerr << "A data base path should be provided !!!\n";
		return NO_DB_PATH;
	}
	return 0;
}

