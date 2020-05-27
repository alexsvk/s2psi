#ifndef err_codes_h
#define err_codes_h

#define FIRST_CODE_NUMBER -1

#define MALFORMED_PARAMS -1
#define PARSER -2
// N.B. !!! Deprecated since api_0_2, api_steroid_0_3. The wild card (*) is used instead of throwing the error. Refer to LabView - CPP.docx
//#define COLUMNS_SIZE_IS_ZERO -3
#define VALUES_SIZE_IS_ZERO -4
#define COMPILER_ADD_TABLE_NAME -5
#define COLUMN_DELIMITER_WAS_NOT_FOUND -6
#define OPEN_DB -7
#define SELECT_FROM_DB -8
#define INSERT_TO_DB -8
#define CREATE_TABLE -8
#define DEL_FROM_DB -8
#define UPDATE_DB_TABLE -8
#define MEM_ALLOC -9
// N.B. !!! Deprecated since api_0_2, api_steroid_0_3. The CREATE clause policy handles these cases. Refer to LabView - CPP.docx
//#define CREATE_COMPILER_ADD_COLUMN_TYPE -10
//#define CREATE_COMPILER_ADD_COLUMN_CONSTRAINT -11
// N.B. !!! The error is returned if create_compiler_impl() encountered a std::runtime_error which is thrown either by add_constraints or add_type method.
#define BROKEN_CREATE_POLICY_BEHAVIOR -10
#define BOOST_LEXICAL_CAST -12
#define STD_PUSH_BACK_MEM_ALLOC -13
#define NO_SQLITE3_DATA_HANDLE_IS_SET -14
#define MMS_SQL_QUERY_VALIDATION -15
#define MEASUREMENT_AUTO_CREATOR_NO_MODULE_WAS_FOUND -16
#define MEASUREMENT_AUTO_CREATOR_MORE_THAN_ONE_MODULE -17
#define MEASUREMENT_AUTO_CREATOR_UKNOWN_DATA_WAS_SELECTED -18

#define MEDIATOR_AND_FRIEND_MAIN_FILTER -30
#define MEDIATOR_MAIN_FILTER -31
#define DROP_TABLE -32

#define LAST_CODE_NUMBER -32

// N.B. !!! 
// Used only internally, 12.08.2013.
// Used also for a client, 28.08.2013.
#define SUCCESS 0

#endif