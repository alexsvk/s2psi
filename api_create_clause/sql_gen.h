#ifndef sql_gen_h_
#define sql_gen_h_

#ifdef BUILDING_THE_DLL
#define EXPORTED __declspec(dllexport)
#else
#define EXPORTED __declspec(dllimport)
#endif

typedef int create_return_val;
/*!
\brief Creates a table using the correspondent SQL query in the correspondent data base.
\param q_ptr Specification of the SQL query to be compiled.
\param db_path The data base path. 
\returns 0 if successfull.
*/
extern "C" EXPORTED create_return_val create_(sql_spec_ptr const q_ptr, unsigned const & len, char const* db_path);

#endif