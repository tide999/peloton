/*-------------------------------------------------------------------------
 *
 * dll.cpp
 * file description
 *
 * Copyright(c) 2015, CMU
 *
 * /n-store/src/bridge/ddl.cpp
 *
 *-------------------------------------------------------------------------
 */

#include "backend/bridge/bridge.h"
#include "backend/bridge/ddl.h"
#include "backend/catalog/catalog.h"
#include "backend/storage/backend_vm.h"
#include "backend/storage/table_factory.h"

#include <cassert>

namespace nstore {
namespace bridge {

bool DDL::CreateTable(std::string table_name,
                      DDL_ColumnInfo* ddl_columnInfo,
                      int num_columns, 
                      catalog::Schema* schema = NULL){
  // Either columns or schema is required to create a table
  if( ddl_columnInfo == NULL && schema == NULL ) 
    return false;

  int db_oid = GetCurrentDatabaseOid();
  if( db_oid == 0 )
    return false;

  // Construct schema with ddl_columnInfo
  if( schema == NULL ){
    std::vector<catalog::ColumnInfo> columnInfoVect;

    for( int column_itr = 0; column_itr < num_columns; column_itr++ ){

      ValueType currentValueType;

      switch( ddl_columnInfo[column_itr].type ){
        // Could not find yet corresponding types in Postgres...
        // Also - check below types again to make sure..
        // TODO :: change the numbers to enum type
  
        //case ??:
        //currentValueType = VALUE_TYPE_NULL;
        //break;
  
        //case ??:
        //currentValueType = VALUE_TYPE_TINYINT;
        //ddl_columnInfo[column_itr].is_inlined = true;
  
        //break;
        //case ??:
        //currentValueType = VALUE_TYPE_ADDRESS;
        //break;
  
        //case ??:
        //currentValueType = VALUE_TYPE_VARBINARY;
        //break;
  
        /* BOOLEAN */
        case 16: // boolean, 'true'/'false'
        currentValueType = VALUE_TYPE_BOOLEAN;
        break;
  
        /* INTEGER */
        case 21: // -32 thousand to 32 thousand, 2-byte storage
        currentValueType = VALUE_TYPE_SMALLINT;
        ddl_columnInfo[column_itr].is_inlined = true;
        break;
        case 23: // -2 billion to 2 billion integer, 4-byte storage
        currentValueType = VALUE_TYPE_INTEGER;
        ddl_columnInfo[column_itr].is_inlined = true;
        break;
        case 20: // ~18 digit integer, 8-byte storage
        currentValueType = VALUE_TYPE_BIGINT;
        ddl_columnInfo[column_itr].is_inlined = true;
        break;

        /* DOUBLE */
        case 701: // double-precision floating point number, 8-byte storage
        currentValueType = VALUE_TYPE_DOUBLE;
        ddl_columnInfo[column_itr].is_inlined = true;
        break;
   
        /* CHAR */
        case 1042: // char(length), blank-padded string, fixed storage length
        currentValueType = VALUE_TYPE_VARCHAR;
        ddl_columnInfo[column_itr].is_inlined = true;
        break;
        // !!! NEED TO BE UPDATED ...
        case 1043: // varchar(length), non-blank-padded string, variable storage length;
        currentValueType = VALUE_TYPE_VARCHAR;
        ddl_columnInfo[column_itr].is_inlined = true;
        break;
   
        /* TIMESTAMPS */
        case 1114: // date and time
        case 1184: // date and time with time zone
        currentValueType = VALUE_TYPE_TIMESTAMP;
        ddl_columnInfo[column_itr].is_inlined = true;
        break;
   
        /* DECIMAL */
        case 1700: // numeric(precision, decimal), arbitrary precision number
        currentValueType = VALUE_TYPE_DECIMAL;
        break;
   
        /* INVALID VALUE TYPE */
        default:
        currentValueType = VALUE_TYPE_INVALID;
        printf("INVALID VALUE TYPE : %d \n", ddl_columnInfo[column_itr].type);
        break;
      }

        catalog::ColumnInfo *columnInfo = new catalog::ColumnInfo( currentValueType,
                                                                   ddl_columnInfo[column_itr].column_offset,
                                                                   ddl_columnInfo[column_itr].column_length,
                                                                   ddl_columnInfo[column_itr].name,
                                                                   ddl_columnInfo[column_itr].allow_null,
                                                                   ddl_columnInfo[column_itr].is_inlined );
        // Add current columnInfo into the columnInfoVect
        columnInfoVect.push_back(*columnInfo);
      }

      // Construct schema from vector of ColumnInfo
      schema = new catalog::Schema(columnInfoVect);
    }

    // Construct backend
    storage::VMBackend *vmbackend = new storage::VMBackend;

    // Create a table from schema
    storage::DataTable *table;

    table =  storage::TableFactory::GetDataTable(db_oid, schema, table_name);

    //LOG_WARN("Created table : %s \n", table_name);
    printf("Created table : %s \n", table_name.c_str());
    return true;
}

bool DDL::DropTable(std::string table_name){

  int db_oid = GetCurrentDatabaseOid();
  if( db_oid == 0 )
    return false;

  if( table_name == "" )
    return false;


  bool ret = storage::TableFactory::DropDataTable(table_name);
  if( !ret )
    return false;

  printf("Drop table : %s \n", table_name.c_str());
  return true;
}

extern "C" {
  bool DDL_CreateTable(char* table_name, DDL_ColumnInfo* ddl_columnInfo, int num_columns) {
    return DDL::CreateTable(table_name, ddl_columnInfo, num_columns);
  }
  bool DDL_DropTable(char* table_name) {
    return DDL::DropTable(table_name);
  }
}

} // namespace bridge
} // namespace nstore