#ifndef policy_h
#define policy_h

// SYSTEM
#include <vector>
#include <memory> // shared_ptr, make_shared
#include <algorithm> // all_of, any_of, find_if, find, count_if
//#include <type_traits> // is_base_of
// LOCAL
#include "parsers.hpp" // select_clause_container

namespace mms
{
	/*!
	\brief The struct rule defines a rule which may be either a single rule or a policy - a set of single rules.
	\brief It defines a default implementation of children management for single rules and a default implementation of quering whether a rule is a policy or not.
	\brief Also it defines a rule validation methods with corresponding default implementations.
	*/
	struct rule
	{
		typedef std::shared_ptr<rule> rule_ptr;
		typedef std::vector<rule_ptr> rule_container;
		virtual bool is_policy() const
		{
			return false;
		}
		virtual void add_rule(rule_ptr const & rule_p)
		{
			throw std::runtime_error("Failed to add a rule : This is a single rule");
		}
		virtual void add_rules(rule_ptr const & rule_p)
		{
			throw std::runtime_error("Failed to add rules : This is a single rule");
		}
		virtual void delete_rule(unsigned const & which)
		{
			throw std::runtime_error("Failed to delete a rule : This is a single rule");
		}
		virtual rule_ptr const & get_rule(unsigned const & which) const
		{
			static rule_ptr const rup = rule_ptr();
			throw std::runtime_error("Failed to get a rule : This is a single rule");
			return rup;
		}
		virtual rule_container const & get_rules() const
		{
			static rule_container const rules = rule_container();
			throw std::runtime_error("Failed to get rules : This is a single rule");
			return rules;
		}
		virtual bool validate(create_clause_container const &) const = 0
		{
			throw std::runtime_error("Failed to validate a CREATE clause : No implementation is defined for validate(create_clause_container const &) const");
		}
		virtual bool validate(select_clause_container const &) const = 0
		{
			throw std::runtime_error("Failed to validate a SELECT clause : No implementation is defined for validate(select_clause_container const &) const");
		}
		virtual bool validate(select_clause_container const &, column_name_t const &) const = 0
		{
			throw std::runtime_error("Failed to validate a name in a SELECT clause : No implementation is defined for validate(select_clause_container const&,columns_name_t const&) const");
		}
	};
	// Interface.
	/*!
	\brief The struct policy defines a sensible implementation of children management for policies. 
	*/
	struct policy : rule
	{
		bool is_policy() const override
		{
			return true;
		}
		void add_rule(rule_ptr const & rule_p) override
		{
			rules.push_back( rule_p );
		}
		void add_rules( rule_ptr const & rule_p )
		{
			rules.resize(0);
			rules = rule_p->get_rules();
		}
		void delete_rule(unsigned const & which) override
		{
			rules.erase( rules.cbegin() + which );
		}
		rule_ptr const & get_rule(unsigned const & which) const override
		{
			return rules[which];
		}
		rule_container const & get_rules() const override
		{
			return rules;
		}
	protected:
		rule_container rules;		
	};
	/*!
	\brief The struct select_clause_policy defines an implementation of a MMS SQL SELECT query validation.
	*/
	struct select_clause_policy : policy
	{
		bool validate(select_clause_container const & select_clause) const override
		{
			return std::all_of( 
									rules.cbegin(), 
									rules.cend(), 
									[&select_clause](rule_ptr const & rule_ptr)->bool
									{
										return rule_ptr->validate( select_clause );
									}
							  );
		}
		bool validate(select_clause_container const & select_clause, column_name_t const & name) const override
		{
			return rule::validate(select_clause, name);
		}
		bool validate(create_clause_container const & create_clause) const override
		{
			return rule::validate(create_clause);
		}
	};
	// Interface.
	/*!
	\brief The struct names_policy defines an implementation of a MMS SQL SELECT query column names validation.
	*/
	struct names_policy : select_clause_policy
	{
		typedef std::vector<column_name_t> names_container;
		virtual names_container extract_names(select_clause_container const & select_clause) const = 0;
		bool validate(select_clause_container const & select_clause) const final
		{
			auto const & names = extract_names( select_clause );
			for ( auto const & name : names ) 
				if ( !std::any_of( 
									rules.cbegin(), 
									rules.cend(), 
									[&name,&select_clause](rule_ptr const & rule)
									{
										return rule->validate(select_clause, name);//static_cast<subpolicy const*>(static_cast<void const*>(rule.get()))->validate( select_clause, name );
									} 
					     		 ) 
				   ) return false;
			return true;				
		}
	};
	/*!
	\brief The struct filter_names_policy defines an implementation of a MMS SQL SELECT query filter column names extraction.
	*/
	struct filter_names_policy : names_policy
	{
		names_container extract_names(select_clause_container const & select_clause) const override
		{
			names_container names;
			for ( auto const & flt : select_clause.filters ) names.push_back( flt.first );
			return names;
		}
	};
	/*!
	\brief The struct filter_names_policy defines an implementation of a MMS SQL SELECT query group column names extraction.
	*/
	struct groups_names_policy : names_policy
	{
		names_container extract_names(select_clause_container const & select_clause) const override
		{
			return select_clause.groups;
		}
	};
	/*!
	\brief The struct filter_names_policy defines an implementation of a MMS SQL SELECT query group filter names extraction.
	*/
	struct group_filters_names_policy : names_policy
	{
		names_container extract_names(select_clause_container const & select_clause) const override
		{
			names_container names;
			for ( auto const & flt : select_clause.group_filters ) names.push_back( flt.first );
			return names;
		}
	};
	/*!
	\brief The struct filter_names_policy defines an implementation of a MMS SQL SELECT query order column names extraction.
	*/
	struct order_names_policy : names_policy
	{
		names_container extract_names(select_clause_container const & select_clause) const override
		{
			return select_clause.order_options.columns;
		}
	};
	// Interface.
	/*!
	\brief The struct name_single_rules_common defines a common interface for single rules of a name policy .
	*/
	struct name_single_rules_common : rule
	{
		bool validate(select_clause_container const & select_clause) const override
		{
			return rule::validate(select_clause);
		}
		bool validate(create_clause_container const & create_clause) const override
		{
			return rule::validate(create_clause);
		}
		bool present_in_columns(select_clause_container const & select_clause, column_name_t const & name) const
		{
			return in_columns(select_clause, name);
		}
		bool absent_in_columns(select_clause_container const & select_clause, column_name_t const & name) const
		{
			return !present_in_columns(select_clause, name);
		}
		bool in_columns(select_clause_container const & select_clause, column_name_t const & name, bool const & expected = true) const
		{
			bool const & indeed_present = std::find( select_clause.columns.cbegin(), select_clause.columns.cend(), name ) != select_clause.columns.cend();
			return indeed_present == expected;
		}
		bool absent_in_aliases(select_clause_container const & select_clause, column_name_t const & name) const
		{
			return	std::find_if( 
									select_clause.aliases.cbegin(), 
									select_clause.aliases.cend(), 
									[name](column_and_alias_pair const & column_and_alias)->bool
									{
										return name.compare( column_and_alias.first ) == 0 || name.compare( column_and_alias.second ) == 0;
									}
								) == select_clause.aliases.cend();		
		}
	};
	// Simple component.
	/*!
	\brief The struct single_rule_1 defines a MMS SQL query names policy single rule #1.
	*/
	struct single_rule_1 : name_single_rules_common
	{
		bool validate(select_clause_container const & select_clause, column_name_t const & name) const
		{
			return absent_in_columns( select_clause, name ) && absent_in_aliases( select_clause, name );
		}
	};
	// Simple component.
	/*!
	\brief The struct single_rule_2 defines a MMS SQL query names policy single rule #2.
	*/
	struct single_rule_2 : name_single_rules_common
	{
		bool validate(select_clause_container const & select_clause, column_name_t const & name) const
		{
			return present_in_columns(select_clause, name) && absent_in_aliases( select_clause, name );
		}
	};
	// Simple component.
	/*!
	\brief The struct single_rule_3 defines a MMS SQL query names policy single rule #3.
	*/
	struct single_rule_3 : name_single_rules_common
	{
		bool validate(select_clause_container const & select_clause, column_name_t const & name) const
		{
			bool absent_in_values = false;
			return 
				present_in_columns( select_clause, name ) &&
				// only one time in aliases' keys and not in values at the correspondent place
				std::count_if( 
								select_clause.aliases.cbegin(), 
								select_clause.aliases.cend(), 
								[name, &absent_in_values](column_and_alias_pair const & column_and_alias)->bool
								{
									auto const & present_in_keys = name.compare( column_and_alias.first ) == 0;
									bool const & absent_in_values_ = name.compare( column_and_alias.second ) != 0;
									if ( present_in_keys && absent_in_values_ ) absent_in_values = true;
									return present_in_keys;
								}
						     ) == 1 &&
				absent_in_values;
		}
	};
	// Simple component.
	/*!
	\brief The struct single_rule_4 defines a MMS SQL query names policy single rule #4.
	*/
	struct single_rule_4 : name_single_rules_common
	{
		bool validate(select_clause_container const & select_clause, column_name_t const & name) const
		{
			return 
				//absent_in_columns( select_clause, name ) &&
				// only one time in aliases' values
				std::count_if( 
								select_clause.aliases.cbegin(), 
								select_clause.aliases.cend(), 
								[name](column_and_alias_pair const & column_and_alias)->bool
								{
									return name.compare( column_and_alias.second ) == 0;
								}
							 ) == 1; /*29.08.2013. Refer to LabView - CPP.docx.
									 &&
				//// not in aliases' keys
				//std::find_if( 
				//				select_clause.aliases.cbegin(), 
				//				select_clause.aliases.cend(), 
				//				[name](column_and_alias_pair const & column_and_alias)->bool
				//				{
				//					return name.compare( column_and_alias.first ) == 0;
				//				}
				//			) == select_clause.aliases.cend();
				*/
		}
	};
	/*!
	\brief The struct create_clause_policy defines an implementation of a MMS SQL CREATE query validation.
	*/
	struct create_clause_policy : policy
	{
		bool validate(select_clause_container const & select_clause) const override
		{
			return rule::validate(select_clause);
		}
		bool validate(select_clause_container const & select_clause, column_name_t const & name) const override
		{
			return rule::validate(select_clause, name);
		}
		bool validate(create_clause_container const & create_clause) const override
		{
			return std::all_of( 
									rules.cbegin(), 
									rules.cend(), 
									[&create_clause](rule_ptr const & rule_ptr)->bool
									{
										return rule_ptr->validate(create_clause);
									}
							  );
		}	
	};
	// Interface.
	/*!
	\brief The struct create_clause_single_rule_common defines a common interface for single rules of the MMS CREATE clause policy .
	*/
	struct create_clause_single_rule_common : rule
	{
		bool validate(select_clause_container const & select_clause) const override
		{
			return rule::validate(select_clause);
		}
		bool validate(select_clause_container const & select_clause, column_name_t const & name) const override
		{
			return rule::validate(select_clause, name);
		}
	};
	/*!
	\brief The struct non_empty_columns_list_rule defines an implementation of the single rule a) in the LabView - CPP.docx.
	*/
	struct non_empty_columns_list_rule : create_clause_single_rule_common
	{
		bool validate(create_clause_container const & create_clause) const override
		{
			return create_clause.columns.size() != 0;
		}			
	};
	/*!
	\brief The struct dif_column_names_list_rule defines an implementation of the single rule b) in the LabView - CPP.docx.
	*/
	struct dif_column_names_list_rule : create_clause_single_rule_common
	{
		bool validate(create_clause_container const & create_clause) const override
		{
			for ( auto const & column : create_clause.columns ) if ( std::count( create_clause.columns.cbegin(), create_clause.columns.cend(), column ) > 1 ) return false;
			return true;
		}			
	};
	/*!
	\brief The struct columns_list_equal_to_types_list_rule defines an implementation of the single rule c) in the LabView - CPP.docx.
	*/
	struct columns_list_equal_to_types_list_rule : create_clause_single_rule_common
	{
		bool validate(create_clause_container const & create_clause) const override
		{
			return create_clause.columns.size() == create_clause.types.size();
		}			
	};
	/*!
	\brief The struct columns_list_equal_to_types_list_rule defines an implementation of the single rule d) in the LabView - CPP.docx.
	*/
	struct varchar_satellite_data_rule : create_clause_single_rule_common
	{
		bool validate(create_clause_container const & create_clause) const override
		{
			auto const & cc = create_clause;
			for ( auto idx = 0U; idx < cc.columns.size(); ++idx )
				if ( cc.types.size() > idx && cc.types[idx].v == sql_data_type::varchar ) 
				{
					auto const & cit = std::find_if( 
														cc.satellite_data.cbegin(), 
														cc.satellite_data.cend(), 
														[idx,&cc](column_and_satellite_data_item_container const & column_and_item)
														{
															return cc.columns[idx].compare( column_and_item.first ) == 0;
														}
												   );
					if ( cit != cc.satellite_data.cend() ) return cit->second.first != 0U;
					return false;
				}
			return true;
		}			
	};
	/*!
	\brief The struct foreign_satellite_data_rule defines an implementation of the single rule e) in the LabView - CPP.docx.
	*/
	struct foreign_satellite_data_rule : create_clause_single_rule_common
	{
		bool validate(create_clause_container const & create_clause) const override
		{
			auto const & cc = create_clause;
			for ( auto idx = 0U; idx < cc.columns.size(); ++idx )
				if ( cc.constraints.size() > idx && cc.constraints[idx].foreign ) 
				{
					auto const & cit = std::find_if( 
														cc.satellite_data.cbegin(), 
														cc.satellite_data.cend(), 
														[idx,&cc](column_and_satellite_data_item_container const & column_and_item)
														{
															return cc.columns[idx].compare( column_and_item.first ) == 0;
														}
												   );
					if ( cit != cc.satellite_data.cend() ) return cit->second.second.size() != 0U;
					return false;
				}
			return true;
		}			
	};
} // namespace mms

///* 
//* The PP_NARG macro returns the number of arguments that have been
//* passed to it.
//*/
//#define __VA_NARG__(...) \
//        (__VA_NARG_(_0, ## __VA_ARGS__, __RSEQ_N()) - 1)
//#define __VA_NARG_(...) \
//        __VA_ARG_N(__VA_ARGS__)
//#define __VA_ARG_N( \
//         _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
//        _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
//        _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
//        _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
//        _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
//        _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
//        _61,_62,_63,N,...) N
//#define __RSEQ_N() \
//        63, 62, 61, 60,                         \
//        59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
//        49, 48, 47, 46, 45, 44, 43, 42, 41, 40, \
//        39, 38, 37, 36, 35, 34, 33, 32, 31, 30, \
//        29, 28, 27, 26, 25, 24, 23, 22, 21, 20, \
//        19, 18, 17, 16, 15, 14, 13, 12, 11, 10, \
//         9,  8,  7,  6,  5,  4,  3,  2,  1,  0 
//#define ADD_RULE(child_id, policy_ptr, rule) policy_ptr->get_rule(child_id)->add_rule( std::make_shared<rule>() );
//#define ADD_RULES(child_id, policy_ptr, ...)\
//	auto const & args_cnt = __VA_NARG__(__VA_ARGS__);\

#endif