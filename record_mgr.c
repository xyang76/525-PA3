#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "buffer_mgr.h"
#include "storage_mgr.h"
#include "dberror.h"
#include "record_mgr.h"
#include "tables.h"
#include "expr.h"

/***************************************************************
 * Function Name: initRecordManager 
 *
 * Description: initial record manager
 *
 * Parameters: void *mgmtData
 *
 * Return: RC
 *
 * Author: Xiaoliang Wu
 *
 * History:
 *      Date            Name                        Content
 *      2016/03/12      Xiaoliang Wu                Complete
 *
***************************************************************/

RC initRecordManager (void *mgmtData){
    printf("----------------------------- Initial Record Manager ---------------------------------\n");
    return RC_OK;
}

/***************************************************************
 * Function Name: shutdownRecordManager 
 *
 * Description: shutdown record manager 
 *
 * Parameters: NULL
 *
 * Return: RC
 *
 * Author: Xiaoliang Wu
 *
 * History:
 *      Date            Name                        Content
 *
***************************************************************/

RC shutdownRecordManager (){
    printf("-------------------------------- shutdown record manager ---------------------------------\n");
    return RC_OK;
}

/***************************************************************
 * Function Name: createTable 
 *
 * Description: Creating a table should create the underlying page file and store information about the schema, free-space, ... and so on in the Table Information pages
 *
 * Parameters: char *name, Schema *schema
 *
 * Return: RC
 *
 * Author: Xiaoliang Wu
 *
 * History:
 *      Date            Name                        Content
 *      03/19/16        Xiaoliang Wu                Complete.
 *
***************************************************************/

RC createTable (char *name, Schema *schema){
    RC RC_flag;
    SM_FileHandle fh;
    SM_PageHandle ph;
    int numAttr, keySize;
    int fileMetadataSize;
    Value *fMetadataSize;
    Value *recordSize;
    Value *slotSize;
    Value *recordNum;
    char *input, *pageMetadataInput;
    int pageMetadataNum;
    Value *pageNum, *capacityFlag;

    int i,j,k;

    RC_flag = createPageFile(name);

    if(RC_flag != RC_OK){
        return RC_flag;
    }

    RC_flag = openPageFile(name, &fh);

    if(RC_flag != RC_OK){
        return RC_flag;
    }

    numAttr = schema->numAttr;
    keySize = schema->keySize;

    fileMetadataSize = (6 + numAttr + keySize)*sizeof(int) + numAttr*sizeof(char);

    if(fileMetadataSize%PAGE_SIZE == 0){
        fileMetadataSize = fileMetadataSize/PAGE_SIZE;
    }
    else{
        fileMetadataSize = fileMetadataSize/PAGE_SIZE + 1;
    }

    fMetadataSize->dt = DT_INT;
    fMetadataSize->v.intV = fileMetadataSize;
    slotSize->dt = DT_INT;
    slotSize->v.intV = 256;
    recordSize->dt = DT_INT;
    recordSize->v.intV = (getRecordSize(schema)/(slotSize->v.intV));
    recordNum->dt = DT_INT;
    recordNum->v.intV = 0;

    input = (char *)calloc(PAGE_SIZE, sizeof(char));

    if(fileMetadataSize>1){
        printf("file metadata size > 1.\n");
        while(1){}
    }

    strcat(input,serializeValue(fMetadataSize));
    strcat(input,serializeValue(recordSize));
    strcat(input,serializeValue(slotSize));
    strcat(input,serializeValue(recordNum));
    strcat(input,serializeSchema(schema));

    RC_flag = ensureCapacity(fileMetadataSize, &fh);
    if(RC_flag != RC_OK){
        RC_flag = closePageFile(&fh);
        return RC_flag;
    }

    for (i = 0; i < fileMetadataSize; ++i) {
        
        ph = (SM_PageHandle)calloc(1,PAGE_SIZE);
        for (k = 0, j = i*PAGE_SIZE; j < (i+1)*PAGE_SIZE; ++j, ++k) {
            ph[k] = input[j];
        }
        ph[PAGE_SIZE] = '\0';
        RC_flag = writeBlock(i,&fh,ph);
        free(ph);
        if(RC_flag != RC_OK){
            RC_flag = closePageFile(&fh);
            return RC_flag;
        }
    }
    free(input);

    RC_flag = appendEmptyBlock(&fh);
    if(RC_flag != RC_OK){
        RC_flag = closePageFile(&fh);
        return RC_flag;
    }

    pageNum->dt = DT_INT;
    pageNum->v.intV = 0;
    capacityFlag->dt = DT_BOOL;
    capacityFlag->v.boolV = 1;

    pageMetadataNum = (strlen(serializeValue(pageNum)) + strlen(serializeValue(capacityFlag)))/PAGE_SIZE;

    pageMetadataInput = (char *)calloc(1, PAGE_SIZE);

    for (i = 0; i < pageMetadataNum; ++i) {
        strcat(pageMetadataInput, serializeValue(pageNum)); 
        strcat(pageMetadataInput, serializeValue(capacityFlag)); 
        pageNum->v.intV = i+1;
        if(i == pageMetadataNum-1){
            capacityFlag->v.boolV = 0;
            pageNum->v.intV = pageMetadataNum;
        }
    }
    RC_flag = writeBlock(fileMetadataSize, &fh, ph);
    if(RC_flag != RC_OK){
        RC_flag = closePageFile(&fh);
        return RC_flag;
    }

    free(pageMetadataInput);
    RC_flag = closePageFile(&fh);
    return RC_flag;
}

/***************************************************************
 * Function Name: openTable
 *
 * Description: open a table
 *
 * Parameters: RM_TableData *rel, char *name
 *
 * Return: RC
 *
 * Author: Xiaoliang Wu
 *
 * History:
 *      Date            Name                        Content
 *
***************************************************************/

RC openTable (RM_TableData *rel, char *name){
}

/***************************************************************
 * Function Name: closeTable
 *
 * Description: close a table
 *
 * Parameters: RM_TableData *rel
 *
 * Return: RC
 *
 * Author: Xiaoliang Wu
 *
 * History:
 *      Date            Name                        Content
 *
***************************************************************/

RC closeTable (RM_TableData *rel){
    freeSchema(rel->schema);

    //free other things.
    return RC_OK;
}

/***************************************************************
 * Function Name: deleteTable
 *
 * Description: delete a table
 *
 * Parameters: char *name
 *
 * Return: RC
 *
 * Author: Xiaoliang Wu
 *
 * History:
 *      Date            Name                        Content
 *      03/19/16        Xiaoliang Wu                Complete.
 *
***************************************************************/

RC deleteTable (char *name){
    return destroyPageFile(name);
}

/***************************************************************
 * Function Name: getNumTuples
 *
 * Description: get the number of record
 *
 * Parameters: RM_TableData *rel
 *
 * Return: int
 *
 * Author: Xiaoliang Wu
 *
 * History:
 *      Date            Name                        Content
 *
***************************************************************/

int getNumTuples (RM_TableData *rel){
}

/***************************************************************
 * Function Name: 
 *
 * Description: 
 *
 * Parameters: 
 *
 * Return: 
 *
 * Author: 
 *
 * History:
 *      Date            Name                        Content
 *
***************************************************************/

RC insertRecord (RM_TableData *rel, Record *record){
}

/***************************************************************
 * Function Name: 
 *
 * Description: 
 *
 * Parameters: 
 *
 * Return: 
 *
 * Author: 
 *
 * History:
 *      Date            Name                        Content
 *
***************************************************************/

RC deleteRecord (RM_TableData *rel, RID id){
}

/***************************************************************
 * Function Name: 
 *
 * Description: 
 *
 * Parameters: 
 *
 * Return: 
 *
 * Author: 
 *
 * History:
 *      Date            Name                        Content
 *
***************************************************************/

RC updateRecord (RM_TableData *rel, Record *record){
}

/***************************************************************
 * Function Name: 
 *
 * Description: 
 *
 * Parameters: 
 *
 * Return: 
 *
 * Author: 
 *
 * History:
 *      Date            Name                        Content
 *
***************************************************************/

RC getRecord (RM_TableData *rel, RID id, Record *record){
}

/***************************************************************
 * Function Name: 
 *
 * Description: 
 *
 * Parameters: 
 *
 * Return: 
 *
 * Author: 
 *
 * History:
 *      Date            Name                        Content
 *
***************************************************************/

RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond)
{

}

/***************************************************************
 * Function Name: 
 *
 * Description: 
 *
 * Parameters: 
 *
 * Return: 
 *
 * Author: 
 *
 * History:
 *      Date            Name                        Content
 *
***************************************************************/

RC next (RM_ScanHandle *scan, Record *record){
}

/***************************************************************
 * Function Name: closeScan
 *
 * Description: free the memo space of this scan
 *
 * Parameters: RM_ScanHandle *scan
 *
 * Return: RC
 *
 * Author: liu zhipeng
 *
 * History:
 *      Date            Name                        Content
 * 03/19/2016    liuzhipeng	first time to implement the function
***************************************************************/

RC closeScan (RM_ScanHandle *scan)
{
	free(scan->rel);
	free(scan->mgmtData);
	free(scan);

	return RC_OK;
}

/***************************************************************
 * Function Name: getRecordSize	
 *
 * Description: get the size of record described by "schema"
 *
 * Parameters: Schema *schema
 *
 * Return: int
 *
 * Author: liu zhipeng
 *
 * History:
 *      Date            Name                        Content
 * 03/19/2016    liuzhipeng	first time to implement the function
***************************************************************/

int getRecordSize (Schema *schema)
{
	int i;
	int result=0;

	for(i=0;i<schema->numAttr;i++)
	{
		switch (schema->dataTypes[i])
		{
			case DT_INT:
				result+=sizeof(int);
				break;
			case DT_FLOAT:
				result+=sizeof(float);
				break;
			case DT_BOOL:
				result+=sizeof(bool);
				break;
			case DT_STRING:
				result+=schema->typeLength[i];
				break;
		}
	}
	return result;
}

/***************************************************************
 * Function Name: createSchema
 *
 * Description: create a new schema described by the parameters
 *
 * Parameters: int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys
 *
 * Return: Schema
 *
 * Author: liu zhipeng
 *
 * History:
 *      Date            Name                        Content
 * 03/19/2016    liuzhipeng	first time to implement the function
***************************************************************/

Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys)
{
	Schema *newschema=(Schema*)malloc(sizeof(Schema));

	newschema->numAttr=numAttr;
	newschema->attrNames=attrNames;
	newschema->dataTypes=dataTypes;
	newschema->typeLength=typeLength;
	newschema->keySize=keySize;
	newschema->keyAttrs=keys;

	return newschema;
}

/***************************************************************
 * Function Name: freeSchema
 *
 * Description: free the memo space of this schema
 *
 * Parameters: Schema *schema
 *
 * Return: RC
 *
 * Author: liu zhipeng
 *
 * History:
 *      Date            Name                        Content
 * 03/19/2016    liuzhipeng	first time to implement the function
***************************************************************/

RC freeSchema (Schema *schema)
{
	int i;

	free(schema->keyAttrs);
	free(schema->typeLength);
	free(schema->dataTypes);
	for(i=0;i<schema->numAttr;i++)
		free(schema->attrNames[i]);
	free(schema->attrNames);
	free(schema);

	return RC_OK;
}

/***************************************************************
 * Function Name: createRecord
 *
 * Description: Create a record by the schema
 *
 * Parameters: Record *record, Schema *schema
 *
 * Return: RC
 *
 * Author: Xincheng Yang
 *
 * History:
 *      Date            Name                        Content
 *   2016/3/18      Xincheng Yang             first time to implement the function
 *
***************************************************************/
RC createRecord (Record **record, Schema *schema){
    *record = (struct Record *)calloc(1, sizeof(struct Record)); 
    (*record)->data = (char*)calloc(getRecordSize(schema), sizeof(char));
    
    return RC_OK;
}

/***************************************************************
 * Function Name: freeRecord
 *
 * Description: Free the space of a record
 *
 * Parameters: Record *record
 *
 * Return: RC
 *
 * Author: Xincheng Yang
 *
 * History:
 *      Date            Name                        Content
 *   2016/3/18      Xincheng Yang             first time to implement the function
 *
***************************************************************/
RC freeRecord (Record *record){
    free(record->data);
    free(record);
    
    return RC_OK;
}

/***************************************************************
 * Function Name: getAttr
 *
 * Description: Get the value of a record
 *
 * Parameters: Record *record, Schema *schema, int attrNum, Value **value
 *
 * Return: RC, value
 *
 * Author: Xincheng Yang
 *
 * History:
 *      Date            Name                        Content
 *   2016/3/18      Xincheng Yang             first time to implement the function
 *
***************************************************************/
RC getAttr (Record *record, Schema *schema, int attrNum, Value **value){
    int offset = 0;
    
    // Calculate offset
    for(int i=0; i< attrNum; i++){
        switch (schema->dataTypes[i]){
            case DT_INT:
                offset += sizeof(int);
                break;
            case DT_FLOAT:
                offset += sizeof(float);
                break;
            case DT_BOOL:
                offset += sizeof(bool);
                break;
            case DT_STRING:
                offset += schema->typeLength[i];
                break;
        }
    }
    
    // Get value from record.
    *value = (Value *)malloc(sizeof(Value));
    (*value)->dt=schema->dataTypes[attrNum];
    switch (schema->dataTypes[attrNum]) 
	{
		case DT_INT:
			memcpy(&((*value)->v.intV), record->data+offset, sizeof(int));
			break;
		case DT_FLOAT:
			memcpy(&((*value)->v.floatV), record->data+offset, sizeof(float));
			break;
		case DT_BOOL:		
			memcpy(&((*value)->v.boolV), record->data+offset, sizeof(int));
			break;
		case DT_STRING:
            //We need append \0 in the end of string.
            char end = '\0';
            (*value)->v.stringV = (char *)malloc(schema->typeLength[attrNum] + 1);
            memcpy((*value)->v.stringV, record->data+offset, schema->typeLength[attrNum]);
            memcpy((*value)->v.stringV+schema->typeLength[attrNum], &end, 1);
			break;
	}
    
    return RC_OK;
}

/***************************************************************
 * Function Name: setAttr
 *
 * Description: Set the value of a record
 *
 * Parameters: Record *record, Schema *schema, int attrNum, Value *value
 *
 * Return: RC
 *
 * Author: Xincheng Yang
 *
 * History:
 *      Date            Name                        Content
 *   2016/3/18      Xincheng Yang             first time to implement the function
 *
***************************************************************/
RC setAttr (Record *record, Schema *schema, int attrNum, Value *value){
    int offset = 0;
    
    // Calculate offset 
    for(int i=0; i< attrNum; i++){
        switch (schema->dataTypes[i]){
            case DT_INT:
                offset += sizeof(int);
                break;
            case DT_FLOAT:
                offset += sizeof(float);
                break;
            case DT_BOOL:
                offset += sizeof(bool);
                break;
            case DT_STRING:
                offset += schema->typeLength[i];
                break;
        }
    }
    
    // Set value into record.
    switch (schema->dataTypes[attrNum])
	{
		case DT_INT:
			memcpy(record->data+offset, &(value->v.intV), sizeof(int));
			break;
		case DT_FLOAT:
			memcpy(record->data+offset, &(value->v.floatV), sizeof(float));
			break;
		case DT_BOOL:
			memcpy(record->data+offset, &(value->v.boolV), sizeof(int));
			break;
		case DT_STRING:
            // We need to calculate the strlen of the input string.
            if(strlen(value->v.stringV) >= schema->typeLength[attrNum]){
                memcpy(record->data + offset, value->v.stringV, schema->typeLength[attrNum]);
            } else {
                strcpy(record->data+offset, value->v.stringV);
            }
			break;
	}
	
	return RC_OK;
}

