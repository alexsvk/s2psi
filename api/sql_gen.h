#ifndef sql_gen_h_
#define sql_gen_h_

#ifdef BUILDING_THE_DLL
#define EXPORTED __declspec(dllexport)
#else
#define EXPORTED __declspec(dllimport)
#endif

typedef char** selection_double_raw_ptr;
typedef char*** select_input_triple_ptr;
typedef char const* sql_spec_ptr;
/*!
\brief Selects all rows specified by the SQL query.
\param q_ptr The pointer to the beginning of the SQL query specification.
\param len The length of the SQL query specification.
\param db_path The data base path.
\returns The table of elements.
\attention Each element is a value at a column and the correspondent row position in the data base.\
The first element is the number of elements.
*/
extern "C" EXPORTED selection_double_raw_ptr select_(sql_spec_ptr const q_ptr, unsigned const & len, char const* db_path);

typedef long long insert_return_val;
/*!
\brief Inserts values specified by the correspondent SQL query.
\param q_ptr The pointer to the beginning of the SQL query specification.
\param len The length of the SQL query specification.
\param db_path The data base path.
\returns The last inserted row ID.
*/
extern "C" EXPORTED insert_return_val insert_(sql_spec_ptr const q_ptr, unsigned const & len, char const* db_path);

typedef int create_return_val;
/*!
\brief Creates a table using the correspondent SQL query in the correspondent data base.
\param q_ptr Specification of the SQL query to be compiled.
\param db_path The data base path. 
\returns 0 if successfull.
*/
extern "C" EXPORTED create_return_val create_(sql_spec_ptr const q_ptr, unsigned const & len, char const* db_path);

typedef int delete_return_val;
/*!
\brief Deletes data from a table using the correspondent SQL query in the correspondent data base.
\param q_ptr Specification of the SQL query to be compiled.
\param db_path The data base path. 
\returns 0 if successfull.
*/
extern "C" EXPORTED delete_return_val delete_(sql_spec_ptr const q_ptr, unsigned const & len, char const* db_path);

typedef int update_return_val;
/*!
\brief Updates a table using the correspondent SQL query in the correspondent data base.
\param q_ptr Specification of the SQL query to be compiled.
\param db_path The data base path. 
\returns 0 if successfull.
*/
extern "C" EXPORTED update_return_val update(sql_spec_ptr const q_ptr, unsigned const & len, char const* db_path);

/*!
\brief Deallocates memory of selected data.
*/
extern "C" EXPORTED void deallocate_selected_data(selection_double_raw_ptr double_raw_p, unsigned const & size);

/*!
\brief Drops the table named table_name_raw_ptr in a data base located in the db_path.
*/
extern "C" EXPORTED int drop_table(sql_spec_ptr table_name_raw_ptr, char const* db_path);

#endif