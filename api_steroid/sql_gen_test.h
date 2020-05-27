#ifndef sql_gen_h_
#define sql_gen_h_

#ifdef BUILDING_THE_DLL
#define EXPORTED __declspec(dllexport)
#else
#define EXPORTED __declspec(dllimport)
#endif

typedef char** select_return_val_double_ptr;
typedef char*** select_input_triple_ptr;
typedef char const* sql_spec_ptr;
/*!
\brief Selects all rows specified by an SQL query.
\param q_ptr The pointer to the beginning of the SQL query.
\param len The length of the SQL query.
\returns A table of elements. Each element is a value at a column and the corresponding row position in a data base.\
The first element is a number of elements following the element.
*/
/*extern "C" EXPORTED */select_return_val_double_ptr select(sql_spec_ptr const q_ptr, unsigned const & len);
/*!
\brief Deallocates memory after a job with the select function returned data is done.
*/
/*extern "C" EXPORTED */void deallocate_selected_data();
#endif