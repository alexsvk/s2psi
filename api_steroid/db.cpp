#include "db.h"
#include "sqlite3.inl"

namespace mms
{
	db::db() : pimpl(std::make_shared<impl>())
	{
	}
	void db::open(source const & src_name)
	{
		pimpl->open(src_name);
	}
	void db::insert(sql_q const & query)
	{
		pimpl->insert(query);
	}
	void db::select(sql_q const & query, data_reader_func const & data_reader, void *data_reader_arg) const
	{
		pimpl->select( query, data_reader, data_reader_arg );
	}
	long long db::last_inserted_row_id() const
	{
		return pimpl->last_inserted_row_id();
	}
	void db::create_table(sql_q const & query)
	{
		pimpl->create_table( query );
	}
	void db::delete_(sql_q const & query)
	{
		pimpl->delete_( query );
	}
	void db::update(sql_q const & query)
	{
		pimpl->update( query );
	}
	void db::close()
	{
		pimpl->close();
	}
	void db::set_db_option( sql_q const & query )
	{
		pimpl->set_db_option(query);
	}
}
