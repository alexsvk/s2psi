#ifndef auto_creator_h
#define auto_creator_h
// SYSTEM
#include <memory> // shared_ptr
// LOCAL
#include "mms_creator.h" // inherits

namespace mms
{

class auto_creator : protected virtual creator
{
public:
	auto_creator(): creator("")
	{
	}
	typedef std::shared_ptr<auto_creator> acreator_ptr;
	virtual void do_() = 0;
};

} // namespace mms

#endif