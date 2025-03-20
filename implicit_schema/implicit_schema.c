#include "postgres.h"
#include "executor/spi.h"
#include "commands/event_trigger.h"
#include "nodes/parsenodes.h"
#include "catalog/namespace.h"
#include "utils/builtins.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(auto_create_schema);

Datum auto_create_schema(PG_FUNCTION_ARGS)
{
    EventTriggerData *trigdata;
    Node *parsetree;
    char *schema = NULL;
    char *quoted_schema;
    char *query;
    int ret;

    if (!CALLED_AS_EVENT_TRIGGER(fcinfo))
        elog(ERROR, "auto_create_schema can only be called as an event trigger");

    trigdata = (EventTriggerData *)fcinfo->context;
    parsetree = trigdata->parsetree;

    /*
     * Check if the parsetree corresponds to a supported command type.
     * Currently supports CREATE TABLE and CREATE VIEW.
     */
    if (IsA(parsetree, CreateStmt))
    {
        RangeVar *relation = ((CreateStmt *)parsetree)->relation;
        if (relation && relation->schemaname)
            schema = relation->schemaname;
    }
    else if (IsA(parsetree, ViewStmt))
    {
        RangeVar *relation = ((ViewStmt *)parsetree)->view;
        if (relation && relation->schemaname)
            schema = relation->schemaname;
    }

    /* No schema specified, nothing to do. */
    if (schema == NULL)
        PG_RETURN_NULL();

    quoted_schema = quote_identifier(schema);

    query = psprintf("CREATE SCHEMA IF NOT EXISTS %s", quoted_schema);

    ret = SPI_connect();
    if (ret != SPI_OK_CONNECT)
        elog(ERROR, "SPI_connect failed: %d", ret);

    ret = SPI_exec(query, 0);
    if (ret != SPI_OK_UTILITY)
        elog(ERROR, "SPI_exec failed: %d", ret);

    SPI_finish();

    pfree(query);

    PG_RETURN_NULL();
}