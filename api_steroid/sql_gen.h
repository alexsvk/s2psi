#ifndef sql_gen_h_
#define sql_gen_h_
// TODO
// UTF-8 code points
#define SQLITE

#ifdef BUILDING_THE_DLL
#define EXPORTED __declspec(dllexport)
#else
#define EXPORTED __declspec(dllimport)
#endif

typedef char** selection_double_raw_ptr;
typedef char*** select_input_triple_ptr;
typedef char const* sql_spec_ptr;
/*!
\brief Selects all rows specified by the SQL query but using MEDIATOR AND FRIEND rules.
\param q_ptr The pointer to the beginning of the SQL query specification.
\param len The length of the SQL query specification.
\path The data base path.
\returns The table of elements.
\attention Each element is a value at a column and the correspondent row position in the data base.\
The first element is the number of elements.
*/
extern "C" EXPORTED selection_double_raw_ptr mediator_visit_subject_and_friend_project(sql_spec_ptr const q_ptr, unsigned const & len, char const* path);
/*!
\brief Selects all rows specified by the SQL query but using MEDIATOR rules.
\param q_ptr The pointer to the beginning of the SQL query specification.
\param len The length of the SQL query specification.
\param db_path The data base path.
\returns The table of elements.
\attention Each element is a value at a column and the correspondent row position in the data base.\
The first element is the number of elements.
*/
extern "C" EXPORTED selection_double_raw_ptr mediator_visit_subject(sql_spec_ptr const q_ptr, unsigned const & len, char const* db_path);
// -//-
extern "C" EXPORTED selection_double_raw_ptr mediator_project_module(sql_spec_ptr const q_ptr, unsigned const & len, char const* db_path);
// -//-
extern "C" EXPORTED selection_double_raw_ptr mediator_device_test_set(sql_spec_ptr const q_ptr, unsigned const & len, char const* db_path);
/*!
\brief Selects all table names from the data base located in the db_path.
\param db_path The data base path.
\returns The table names array with the array size at the start.
*/
extern "C" EXPORTED selection_double_raw_ptr all_table_names(char const *db_path);
/*!
\brief Selects all column names from the table named table_name.
\param table_name The table name.
\param db_path The data base path.
\returns The column names array with the array size at the start.
*/
extern "C" EXPORTED selection_double_raw_ptr table_column_names(char const *table_name, char const *db_path);
/*!
\brief Selects all column names and types from the table named table_name.
\param table_name The table name.
\param db_path The data base path.
\returns The column names array with the array size at the start.
*/
extern "C" EXPORTED selection_double_raw_ptr table_column_names_and_types(char const *table_name, char const *db_path);
/*!
\brief Deallocates memory of selected data.
*/
extern "C" EXPORTED void deallocate_selected_data(selection_double_raw_ptr double_raw_p, unsigned const & size);
/*!
\brief Creates a table named measurement_ concatenated with the value of module_name_raw_ptr in a data base located in the db_path.
\param module_name_raw_ptr The name of the module the measurement to be created for.
\param db_path The data base path.
\returns The result code.
*/
extern "C" EXPORTED int create_measurement(sql_spec_ptr module_name_raw_ptr, char const* db_path);
#endif