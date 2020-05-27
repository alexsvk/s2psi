#ifndef db_h_
#define db_h_

/*!
\brief Common DB interface definition.
\author Alexander Syvak
\date July 13 2013
*/

// SYSTEM
#include <string>
#include <memory>
#include <functional>

// PROJECT
#include <boost/noncopyable.hpp>

namespace mms
{
	class db : ::boost::noncopyable
	{
	public:
		typedef int(*data_reader_raw_func_ptr)(void*,int,char**,char**);
		typedef std::function<int(void*,int,char**,char**)> data_reader_func;
		db();
		~db()
		{
			if ( pimpl ) close();
		}
		typedef std::string source;
		void open(source const & src_name);
		typedef std::string sql_q;
		void select(sql_q const & query, data_reader_func const & data_reader, void *data_reader_arg = nullptr) const;
		void insert(sql_q const & query);
		long long last_inserted_row_id() const;
		void create_table(sql_q const & query);
		void delete_(sql_q const & query);
		void update(sql_q const & query);
		void close();
		void set_db_option(sql_q const & query);
	private:
		struct impl;
		typedef std::shared_ptr<impl> impl_ptr;
		impl_ptr pimpl;		
	};
}; // namespace mms

#endif
