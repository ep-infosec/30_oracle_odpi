//-----------------------------------------------------------------------------
// Copyright (c) 2016, 2022, Oracle and/or its affiliates.
//
// This software is dual-licensed to you under the Universal Permissive License
// (UPL) 1.0 as shown at https://oss.oracle.com/licenses/upl and Apache License
// 2.0 as shown at http://www.apache.org/licenses/LICENSE-2.0. You may choose
// either license.
//
// If you elect to accept the software under the Apache License, Version 2.0,
// the following applies:
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// DemoBindObjects.c
//   Demos simple binding of objects.
//-----------------------------------------------------------------------------

#include "SampleLib.h"
#define OBJECT_TYPE_NAME    "UDT_OBJECT"
#define SQL_TEXT            "begin :1 := " \
                            "pkg_DemoBindObject.GetStringRep(:2); end;"
#define NUM_ATTRS           7

//-----------------------------------------------------------------------------
// main()
//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
    dpiData attrValue, objValue, *stringRepValue;
    dpiObjectAttr *attrs[NUM_ATTRS];
    uint32_t numQueryColumns, i;
    dpiObjectType *objType;
    dpiVar *stringRepVar;
    dpiObject *obj;
    dpiStmt *stmt;
    dpiConn *conn;

    // connect to database
    conn = dpiSamples_getConn(0, NULL);

    // get object type and attributes
    if (dpiConn_getObjectType(conn, OBJECT_TYPE_NAME, strlen(OBJECT_TYPE_NAME),
            &objType) < 0)
        return dpiSamples_showError();
    if (dpiObjectType_getAttributes(objType, NUM_ATTRS, attrs) < 0)
        return dpiSamples_showError();

    // create object and populate attributes
    if (dpiObjectType_createObject(objType, &obj) < 0)
        return dpiSamples_showError();
    attrValue.isNull = 0;
    attrValue.value.asDouble = 13;
    if (dpiObject_setAttributeValue(obj, attrs[0], DPI_NATIVE_TYPE_DOUBLE,
            &attrValue) < 0)
        return dpiSamples_showError();
    attrValue.value.asBytes.ptr = "Demo String";
    attrValue.value.asBytes.length = strlen(attrValue.value.asBytes.ptr);
    if (dpiObject_setAttributeValue(obj, attrs[1], DPI_NATIVE_TYPE_BYTES,
            &attrValue) < 0)
        return dpiSamples_showError();

    // prepare and execute statement
    if (dpiConn_prepareStmt(conn, 0, SQL_TEXT, strlen(SQL_TEXT), NULL, 0,
            &stmt) < 0)
        return dpiSamples_showError();
    if (dpiConn_newVar(conn, DPI_ORACLE_TYPE_VARCHAR, DPI_NATIVE_TYPE_BYTES, 1,
            100, 0, 0, NULL, &stringRepVar, &stringRepValue) < 0)
        return dpiSamples_showError();
    if (dpiStmt_bindByPos(stmt, 1, stringRepVar) < 0)
        return dpiSamples_showError();
    objValue.isNull = 0;
    objValue.value.asObject = obj;
    if (dpiStmt_bindValueByPos(stmt, 2, DPI_NATIVE_TYPE_OBJECT, &objValue) < 0)
        return dpiSamples_showError();
    if (dpiStmt_execute(stmt, 0, &numQueryColumns) < 0)
        return dpiSamples_showError();
    dpiObject_release(obj);
    dpiObjectType_release(objType);
    for (i = 0; i < NUM_ATTRS; i++)
        dpiObjectAttr_release(attrs[i]);

    // display result
    printf("String rep: '%.*s'\n", stringRepValue->value.asBytes.length,
            stringRepValue->value.asBytes.ptr);
    dpiVar_release(stringRepVar);

    // clean up
    dpiStmt_release(stmt);
    dpiConn_release(conn);

    printf("Done.\n");
    return 0;
}
