#pragma once

class base_scope_guard
{
public:
	void dismiss( ) const
	{
		dismissed = true;
	}
protected:
	base_scope_guard( ): dismissed( false )
	{
	}
	base_scope_guard( base_scope_guard const & rhs )
	{
		dismissed = rhs.dismissed;
		rhs.dismiss( );
	}
	~base_scope_guard( )
	{
	}

protected:
	mutable bool dismissed;
private:
	base_scope_guard & operator=( base_scope_guard const & );
};

template < class func_ptr >
class scope_guard_impl0 : public base_scope_guard
{
public:
	scope_guard_impl0( func_ptr const & func_ptr_ ): p_func( func_ptr_ )
	{
	}
	~scope_guard_impl0( )
	{
		if ( !dismissed ) p_func( );
	}
private:
	func_ptr p_func;
};

template < class func_ptr, class param >
class scope_guard_impl1 : public base_scope_guard
{
public:
	scope_guard_impl1( func_ptr const & func_ptr_, param param_ ): p_func( func_ptr_ ), par( param_ )
	{
	}
	~scope_guard_impl1( )
	{
		if ( !dismissed ) p_func( par );
	}
private:
	func_ptr p_func;
	param const par;
};

template < class func_ptr, class param1_type, class param2_type >
class scope_guard_impl2 : public base_scope_guard
{
public:
	scope_guard_impl2( func_ptr const & func_ptr_, param1_type param1, param2_type param2 ): p_func( func_ptr_ ), par1( param1 ), par2( param2 )
	{
	}
	~scope_guard_impl2( )
	{
		if ( !dismissed ) p_func( par1, par2 );
	}
private:
	func_ptr p_func;
	param1_type const par1;
	param2_type const par2;
};

typedef base_scope_guard const & scope_guard;

template < class func_ptr >
scope_guard_impl0< func_ptr > make_guard_0( func_ptr const & func_p )
{
	return scope_guard_impl0< func_ptr >( func_p );
}

template < class func_ptr, class param >
scope_guard_impl1< func_ptr, param > make_guard_1( func_ptr const & func_p, param const & par )
{
	return scope_guard_impl1< func_ptr, param >( func_p, par );
}

template < class func_ptr, class param1_type, class param2_type >
scope_guard_impl2< func_ptr, param1_type, param2_type > make_guard_2( func_ptr const & func_p, param1_type const & par1, param2_type const & par2 )
{
	return scope_guard_impl2< func_ptr, param1_type, param2_type >( func_p, par1, par2 );
}

template < class ref_type > 
class ref_box
{
public:
	ref_box( ref_type & ref_ ): ref( ref_ )
	{
	}
	operator ref_type &( ) const
	{
		return ref;
	}
private:
	ref_type & ref;
};

template < class ref_type >
inline ref_box< ref_type > make_ref( ref_type & ref )
{
	return ref_box< ref_type >( ref );
}