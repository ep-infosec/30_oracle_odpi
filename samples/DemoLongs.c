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
// DemoLongs.c
//   Demos inserting and fetching long columns.
//-----------------------------------------------------------------------------

#include "SampleLib.h"
#define SQL_TEXT_TRUNC      "truncate table DemoLongs"
#define SQL_TEXT_INSERT     "insert into DemoLongs values (:1, :2)"
#define SQL_TEXT_QUERY      "select * from DemoLongs order by IntCol"
#define ARRAY_SIZE          3
#define NUM_ROWS            10
#define SIZE_INCREMENT      50000

//-----------------------------------------------------------------------------
// main()
//-----------------------------------------------------------------------------
int main(int argc, char **argv)
{
    uint32_t i, longValueLength, numQueryColumns, bufferRowIndex;
    dpiData *intColValue, *longColValue;
    dpiVar *intColVar, *longColVar;
    char *longValue;
    dpiConn *conn;
    dpiStmt *stmt;
    int found;

    // connect to database
    conn = dpiSamples_getConn(0, NULL);

    // truncate the table so that the demo can be repeated
    if (dpiConn_prepareStmt(conn, 0, SQL_TEXT_TRUNC, strlen(SQL_TEXT_TRUNC),
            NULL, 0, &stmt) < 0)
        return dpiSamples_showError();
    if (dpiStmt_execute(stmt, DPI_MODE_EXEC_DEFAULT, &numQueryColumns) < 0)
        return dpiSamples_showError();
    dpiStmt_release(stmt);

    // create variables for insertion
    if (dpiConn_newVar(conn, DPI_ORACLE_TYPE_NUMBER, DPI_NATIVE_TYPE_INT64,
            ARRAY_SIZE, 0, 0, 0, NULL, &intColVar, &intColValue) < 0)
        return dpiSamples_showError();
    if (dpiConn_newVar(conn, DPI_ORACLE_TYPE_LONG_VARCHAR,
            DPI_NATIVE_TYPE_BYTES, ARRAY_SIZE, 0, 0, 0, NULL, &longColVar,
            &longColValue) < 0)
        return dpiSamples_showError();

    // prepare insert statement
    if (dpiConn_prepareStmt(conn, 0, SQL_TEXT_INSERT, strlen(SQL_TEXT_INSERT),
            NULL, 0, &stmt) < 0)
        return dpiSamples_showError();

    // insert the requested number of rows
    for (i = 1; i <= NUM_ROWS; i++) {

        // perform binds
        if (dpiStmt_bindByPos(stmt, 1, intColVar) < 0)
            return dpiSamples_showError();
        if (dpiStmt_bindByPos(stmt, 2, longColVar) < 0)
            return dpiSamples_showError();

        // create long string of specified size
        longValueLength = i * SIZE_INCREMENT;
        printf("Inserting row %d with long column of length %d\n", i,
                longValueLength);
        longValue = malloc(longValueLength);
        if (!longValue) {
            fprintf(stderr, "Out of memory!\n");
            return -1;
        }
        memset(longValue, 'A', longValueLength);

        // insert value
        intColValue->isNull = 0;
        intColValue->value.asInt64 = i;
        if (dpiVar_setFromBytes(longColVar, 0, longValue, longValueLength) < 0)
            return dpiSamples_showError();
        free(longValue);
        if (dpiStmt_execute(stmt, DPI_MODE_EXEC_DEFAULT, &numQueryColumns) < 0)
            return dpiSamples_showError();

    }
    dpiStmt_release(stmt);
    printf("\n");

    // perform commit
    if (dpiConn_commit(conn) < 0)
        return dpiSamples_showError();

    // prepare statement for query
    if (dpiConn_prepareStmt(conn, 0, SQL_TEXT_QUERY, strlen(SQL_TEXT_QUERY),
            NULL, 0, &stmt) < 0)
        return dpiSamples_showError();
    if (dpiStmt_setFetchArraySize(stmt, ARRAY_SIZE) < 0)
        return dpiSamples_showError();
    if (dpiStmt_execute(stmt, DPI_MODE_EXEC_DEFAULT, &numQueryColumns) < 0)
        return dpiSamples_showError();
    if (dpiStmt_define(stmt, 1, intColVar) < 0)
        return dpiSamples_showError();
    if (dpiStmt_define(stmt, 2, longColVar) < 0)
        return dpiSamples_showError();

    // fetch rows
    while (1) {
        if (dpiStmt_fetch(stmt, &found, &bufferRowIndex) < 0)
            return dpiSamples_showError();
        if (!found)
            break;
        printf("Fetched row %" PRId64 " with long column of length %d\n",
                intColValue[bufferRowIndex].value.asInt64,
                longColValue[bufferRowIndex].value.asBytes.length);
    }

    // clean up
    dpiVar_release(intColVar);
    dpiVar_release(longColVar);
    dpiStmt_release(stmt);
    dpiConn_release(conn);

    printf("Done.\n");
    return 0;
}
