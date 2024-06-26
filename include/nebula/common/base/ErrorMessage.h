// Copyright (c) 2024 vesoft inc. All rights reserved.

#pragma once

namespace nebula {

struct ErrorMessage {
    // COMMON
    static constexpr const char* EMPTY_STATEMENT = "Empty statement";

    static constexpr const char* GET_ALL_PART_FAILED = "Get all partitions failed";

    static constexpr const char* GET_ALL_LEADER_FAILED = "Get all leaders failed";

    static constexpr const char* PART_NOT_FOUND = "Partition `{}` not found";

    static constexpr const char* PROPERTY_NOT_FOUND = "Property `{}` of type `{}` not found";

    static constexpr const char* BAD_PLAN_NODE_TYPE = "Bad plan node type";

    static constexpr const char* SPLIT_PLAN_TO_STORAGE_FAILED = "Split plan to storage failed";

    static constexpr const char* PROPERTY_NOT_NULLABLE =
            "Property `{}` of type `{}` is not nullable";

    static constexpr const char* VALUES_NOT_COMPARABLE = "{} is not comparable";


    // UNSUPPORTED
    static constexpr const char* UNSUPPORTED_INDEX_PROPERTY_TYPE =
            "Index property type `{}` not supported";

    // DML
    static constexpr const char* DML_NUM_OF_PROPERTY_NOT_MATCH =
            "Insert number of properties of type `{}` does not match, expect: {}, got: {}";

    static constexpr const char* INVALID_DELETE_TYPE =
            "Invalid delete type, `{}` should be Node or Edge";

    static constexpr const char* INSERT_INFER_MORE_THAN_ONE_TYPE =
            "Type inference for `{}` failed, multiple types inferred: `{}`, `{}`";

    static constexpr const char* ENDPOINT_NODE_TYPES_MISMATCH =
            "Endpoint node types of edge type `{}` mismatch";

    static constexpr const char* INSERT_PROPERTY_NOT_FOUND =
            "Insert failed, property `{}` of type `{}` not found";

    static constexpr const char* NOT_IN_CURRENT_GRAPH =
            "`{}` in `{}` is not in current working graph `{}`.";

    // CATALOG
    static constexpr const char* CATALOG_NODE_TYPE_NOT_FOUND = "Node type not found: `{}`";

    static constexpr const char* CATALOG_EDGE_TYPE_NOT_FOUND = "Edge type not found: `{}`";

    static constexpr const char* CATALOG_PRIMARY_KEY_PROPERTY_NOT_FOUND =
            "Primary key property not found: `{}`";

    static constexpr const char* CATALOG_PRIMARY_KEY_NULLABLE =
            "Property `{}` of node type `{}` is nullable, can't be primary index property";

    static constexpr const char* CATALOG_PRIMARY_KEY_NOT_FOUND =
            "Primary key of node type `{}` not found";

    static constexpr const char* CATALOG_PROPERTY_NOT_FOUND =
            "Property `{}` of type `{}` not found";

    static constexpr const char* CATALOG_ELEMENT_TYPE_FOR_INSERT_NOT_FOUND =
            "Element type matching `{}` not found";

    static constexpr const char* CATALOG_INDEX_NOT_FOUND = "Catalog index not found: `{}`";

    static constexpr const char* CATALOG_NODE_TYPE_ALREADY_EXIST =
            "Node type already exist: `{}`";

    static constexpr const char* CATALOG_EDGE_TYPE_ALREADY_EXIST =
            "Edge type already exist: `{}`";

    static constexpr const char* CATALOG_LABEL_ALREADY_EXIST = "Label already exist: `{}.{}`";

    static constexpr const char* CATALOG_LABEL_NOT_FOUND = "Label not found: `{}.{}`";

    static constexpr const char* INVALID_SCHEMA_PATH =
            "Invalid schema path, it should be a canonical path";

    static constexpr const char* CATALOG_GRAPH_TYPE_NOT_FOUND = "Graph type not found: `{}`";

    static constexpr const char* CATALOG_GRAPH_TYPE_ALREADY_EXIST =
            "Graph type already exist: `{}`";

    static constexpr const char* CATALOG_GRAPH_NOT_FOUND = "Graph not found: `{}`";

    static constexpr const char* CATALOG_GRAPH_ALREADY_EXIST = "Graph already exist: `{}`";

    static constexpr const char* DROP_REFERENCED_GRAPH_TYPE =
            "Can not drop graph type `{}` that is referenced by graph `{}`";

    static constexpr const char* CATALOG_MIRROR_GRAPH_ALREADY_EXIST =
            "Mirror graph already exist: `{}`";

    static constexpr const char* CATALOG_MIRROR_GRAPH_NOT_FOUND = "Mirror graph not found `{}`";

    static constexpr const char* CATALOG_GRAPH_ALREADY_HAVE_MIRROR_GRAPH =
            "Graph `{}` already have a mirror graph `{}`";

    static constexpr const char* CATALOG_GRAPH_DO_NOT_HAVE_MIRROR_GRAPH =
            "Graph `{}` does not have a mirror graph";

    static constexpr const char* CATALOG_MIRROR_GRAPH_NAME_CONFLICT =
            "Mirror graph name is same as an existing graph name: `{}`";

    static constexpr const char* CATALOG_MIRROR_GRAPH_BASE_GRAPH_EMPTY =
            "Create mirror graph, base graph is empty";

    static constexpr const char* CATALOG_NODE_INDEX_ALREADY_EXISTS =
            "Node index already exists: {}";

    static constexpr const char* CATALOG_EDGE_INDEX_ALREADY_EXISTS =
            "Edge index already exists: {}";

    static constexpr const char* CATALOG_INDEX_PROPERTY_TYPE_NOT_SUPPORTED =
            "Index property type `{}` not supported";

    static constexpr const char* CATALOG_VERSION_RETIRED = "Catalog version retired: {}";

    static constexpr const char* CATALOG_STATE_PERMISSION_DENIED =
            "Catalog state permission denied";

    static constexpr const char* CATALOG_SCHEMA_NOT_FOUND = "Catalog schema not found: `{}`";

    static constexpr const char* CATALOG_SCHEMA_ALREADY_EXISTS =
            "Catalog schema already exists: `{}`";

    static constexpr const char* CATALOG_DIRECTORY_NOT_FOUND =
            "Catalog directory not found: `{}`";

    static constexpr const char* CATALOG_PROPERTY_ALREADY_EXIST =
            "Property `{}.{}` already exists";

    static constexpr const char* CATALOG_DDL_MANAGER_STOPPED = "Catalog ddl manager stopped";

    static constexpr const char* CATALOG_DDL_TIMEOUT = "Catalog ddl timeout, job id is {}";

    static constexpr const char* CATALOG_MAXIMUM_PENDING_DDL_EXCEEDED =
            "Pending ddl exceeds maximum threshold";

    static constexpr const char* CATALOG_INDEX_TYPE_NOT_SUPPORTED = "Index type not supported";

    // FUNCTION
    static constexpr const char* NUMERIC_VALUE_OUT_OF_RANGE =
            "Numeric value out of range: `{}`, type: `{}`";

    static constexpr const char* INVALID_ARGUMENT_FOR_POWER_FUNCTION =
            "Invalid argument for power function: {}.";

    static constexpr const char* DIVISION_BY_ZERO = "Division by zero: `{}`, type: `{}`";

    static constexpr const char* INVALID_CAST = "Invalid cast from `{}`:`{}` to `{}`";

    static constexpr const char* TRIM_LIST_OUT_OF_RANGE =
            "Trim list out of range, list size: `{}`, trim number: `{}`";

    static constexpr const char* INVALID_CHARACTER_VALUE_FOR_CAST =
            "Invalid character value cast from `{}`:`{}` to `{}`";

    static constexpr const char* LIST_ELEMENT_ERROR = "List element error: {}";

    static constexpr const char* INVALID_FUNCTION_ARGUMENT =
            "Invalid function argument `{}` of `{}`";

    static constexpr const char* SUBSTRING_ERROR =
            "Substring error, invalid length `{}`, must greater than or equal to 0";

    static constexpr const char* INVALID_DATE_TIME_OR_DATETIME_FORMAT =
            "Invalid format: `{}` is not a valid `{}`";

    static constexpr const char* PARSE_TEMPORAL_FAIL = "Parse {}: `{}` fail";

    static constexpr const char* INVALID_LOGICAL_EXPRESSION =
            "Invalid logical expression, only support bool type, expression `{}`, type: `{}`";

    static constexpr const char* EXPR_COMPILE_FAILED =
            "Failed to compile expression, expr: `{}`, error: {}";

    static constexpr const char* EXPR_EVAL_FAILED =
            "Failed to evaluate expression: `{}`, exception: {}";

    static constexpr const char* RESOLVE_FUNCTION_FAILED = "Resolve function failed: `{}`";

    static constexpr const char* EXPR_TYPE_INFER_FAILED =
            "Expression type infer failed, expression: `{}`";

    static constexpr const char* INVALID_CAST_EXPR =
            "Invalid cast expression: `{}`, can not cast from `{}` to `{}`";

    static constexpr const char* INVALID_CONST_EXPR = "Invalid const expression: `{}`";

    static constexpr const char* INVALID_CONSTRUCT_PATH_EXPR_INPUT =
            "Invalid construct path expression input type: `{}`, only support node, edge, "
            "list<edge>";

    static constexpr const char* INVALID_CONSTRUCT_LIST_EXPR_INPUT =
            "Invalid construct list expression input types: `{}`. All elements in list should "
            "have compatible types";

    static constexpr const char* INVALID_CONSTRUCT_RECORD_EXPR_INPUT =
            "Record could only contain unique field names, but `{}` appears more than once.";

    static constexpr const char* INVALID_CASE_EXPR_WHEN_INPUT =
            "The when operand of case expression should be type of "
            "boolean, but got {}: {}";

    static constexpr const char* INVALID_CASE_EXPR_THEN_INPUT =
            "The then/else operands of case expression should have compatible types, but got: "
            "{}";

    static constexpr const char* FUNCTION_TRANSFORM_FAILED =
            "Transform to execution expression failed: `{}`";

    // SEMANTIC
    static constexpr const char* UNDEFINED_VARIABLE =
            "Semantic error, undefined variable: `{}`";

    static constexpr const char* DUPLICATE_DEFINE_VARIABLE =
            "Semantic error, duplicate defined variable: `{}`";

    static constexpr const char* UNDEFINED_PARAMETER =
            "Semantic error, undefined parameter: `{}`";

    static constexpr const char* SEMANTIC_LINEAR_QUERY_COLUMN_SIZE_MISMATCH =
            "Semantic error, column size `{}` vs. `{}` mismatched for the linear query of "
            "composite query statement: `{}`";

    static constexpr const char* SEMANTIC_FILTER_CLAUSE_MUST_BE_BOOL_TYPE =
            "Semantic error, filter clause must be type of bool";

    static constexpr const char* SEMANTIC_INVALID_EXPRESSION_TYPE =
            "Semantic error, invalid expression type, expect `{}` but got `{}` at expression: "
            "`{}`";

    static constexpr const char* SEMANTIC_LINEAR_QUERY_COLUMN_NAME_MISMATCH =
            "Semantic error, column name `{}` vs. `{}` mismatched for the linear query of "
            "composite query statement: `{}`";

    static constexpr const char* SEMANTIC_NODE_PRIMARY_KEY_PROPERTY_NUM_MISMATCH =
            "Semantic error, node properties mismatched primary index, "
            "expect `{}` but got `{}`";

    static constexpr const char* SEMANTIC_NODE_PRIMARY_KEY_PROPERTY_NOT_FOUND =
            "Semantic error, primary index property not found: `{}`";

    static constexpr const char* SEMANTIC_GRAPH_PATTERN_QUANTIFIER_EXCEED_LOW_BOUND =
            "Semantic error, the value of upper bound of graph pattern quantifier shall be "
            "greater than 0";

    static constexpr const char* SEMANTIC_GRAPH_PATTERN_QUANTIFIER_LOW_BOUND_EXCEED_UP_BOUND =
            "Semantic error, lower bound of graph pattern quantifier shall be "
            "less than or equal to upper bound";

    static constexpr const char* SEMANTIC_INVALID_EDGE_PATTERN =
            "Semantic error, expect the path primary to be EdgePattern, but got: {}";

    static constexpr const char* SEMANTIC_INCONSISTENT_VARIABLE_DEFINITION =
            "Semantic error, variable {} cannot be defined as both group variable and {}";

    static constexpr const char* SEMANTIC_GROUP_VARIABLE_REDEFINITION =
            "Semantic error, group variable redefinition {}";

    static constexpr const char* SEMANTIC_INVALID_EDGE_PREDICATE =
            "Semantic error, The predicate can only refer to the binding variable itself `{}` "
            "in element pattern: {}";

    static constexpr const char* PATTERN_NOT_SUPPORTED = "Pattern not supported: `{}`";

    static constexpr const char* SEMANTIC_RETURN_ALL_WITH_GROUPBY =
            "Semantic error, return all can't be used with user specified group bys";

    static constexpr const char* SEMANTIC_RETURN_ALL_WITH_NO_VARS_IN_SCOPE =
            "RETURN * is not allowed when there are no variables in scope";

    static constexpr const char* SEMANTIC_RETURN_ALL_WITH_EMPTY_GROUPBY =
            "Semantic error, return all can't be used with empty group by";

    static constexpr const char* SEMANTIC_RETURN_DUPLICATE_COLUMN =
            "Semantic error, duplicate column name in return statement: `{}`";

    static constexpr const char* SEMANTIC_EMPTY_GROUPBY_MISMATCH =
            "Semantic error, empty group by mismatched";

    static constexpr const char* SEMANTIC_NO_AGG_FOUND_WITH_GROUPBY =
            "Semantic error, no aggregates found but specified group bys";

    static constexpr const char* SEMANTIC_GROUPBY_NO_BINDING =
            "Semantic error, user specified group bys must be binding variable: `{}`";

    static constexpr const char* SEMANTIC_GROUPBY_CONTAIN_AGG_EXPR =
            "Semantic error, group by item `{}` can't contain aggregate expressions: `{}`";

    static constexpr const char* SEMANTIC_GROUPBY_MISMATCH =
            "Semantic error, group bys mismatched";

    static constexpr const char* SEMANTIC_IMPLICIT_AGG_IN_RETURN =
            "Semantic error, implicit aggregation is not allowed in return statement";

    static constexpr const char* SEMANTIC_VAR_PROP_SETITEM_ERROR =
            "Variable and property name between any two SetPropertyItem should not be the same";

    static constexpr const char* SEMANTIC_ASSIGN_EXPR_TYPE_ERROR =
            "The type of `{}({})` cannot be assigned to `{}({})`";

    static constexpr const char* SEMANTIC_CURRENT_WORKING_GRAPH_NOT_FOUND =
            "Current working graph not found";

    static constexpr const char* INVALID_LABEL_EXPR_INPUT =
            "Invalid label expression input: `{}`, expect NODE or EDGE type but got `{}`";

    static constexpr const char* INVALID_TYPE_EXPR_INPUT =
            "Invalid type expression input: `{}`, expect NODE or EDGE type but got `{}`";

    static constexpr const char* INVALID_OFFSET_EXPR_TYPE =
            "Invalid type OFFSET expression type: `{}`, expect `{}`";

    static constexpr const char* INVALID_LIMIT_EXPR_TYPE =
            "Invalid type LIMIT expression type: `{}`, expect `{}`";

    static constexpr const char* INVALID_NUM_OF_PATHS_EXPR_TYPE =
            "Invalid type NUMBER OF PATHS expression type: `{}`, expect `{}`";

    static constexpr const char* INVALID_NUM_OF_GROUPS_EXPR_TYPE =
            "Invalid type NUMBER OF GROUPS expression type: `{}`, expect `{}`";

    // SYNTAX
    static constexpr const char* INVALID_SYNTAX_MULTIPLE_PRIMARY_KEY =
            "Invalid syntax, multiple primary key defined";

    static constexpr const char* INVALID_SYNTAX_PRIMARY_KEY_NOT_FOUND_IN_PROPS =
            "Invalid syntax, primary key `{}` not found in properties list";

    static constexpr const char* INVALID_SYNTAX_SRC_TYPE_OF_EDGE_NOT_SPECIFIED =
            "Invalid syntax, source node type of edge type `{}` not specified";

    static constexpr const char* INVALID_SYNTAX_DST_TYPE_OF_EDGE_NOT_SPECIFIED =
            "Invalid syntax, destination node type of edge type `{}` not specified";

    static constexpr const char* INVALID_SYNTAX_UNKNOWN_ELEMENT_TYPE =
            "Invalid syntax, unknown element type";

    static constexpr const char* INVALID_SYNTAX_INDEX_TYPE_ALIAS_ILLEGAL =
            "Invalid syntax, index `{}` type alias `{}` illegal";

    static constexpr const char* INVALID_SYNTAX_DUPLICATE_PROPERTY =
            "Invalid syntax, duplicate property: `{}`";

    static constexpr const char* INVALID_SYNTAX_NEST_QUERY_LAST_STATEMENT_ERROR =
            "Invalid syntax, last statement in value query expression is not a query "
            "statement: `{}`";

    static constexpr const char* INVALID_SYNTAX_NEST_QUERY_RESULT_STATEMENT_ERROR =
            "Invalid syntax, result statement in value query expression should be linear query "
            "statement";

    static constexpr const char* INVALID_SYNTAX_NEST_QUERY_COLUMN_SIZE_ERROR =
            "Invalid syntax, value query expression should have only 1 column, but got: `{}`";

    static constexpr const char* INVALID_SYNTAX_NEST_QUERY_RETURN_AGG_ERROR =
            "Invalid syntax, result of expression should generated by aggregate function "
            "without group by, but got: `{}`";

    static constexpr const char* INVALID_SYNTAX_NEST_QUERY_RETURN_LIMIT_ERROR =
            "Invalid syntax, result in value query expression should be aggregate expression "
            "without group by or add limit 1 clause in return statement";

    static constexpr const char* INVALID_SYNTAX_VAR_TYPE_CONFLICT =
            "Invalid syntax, variable `{}` defined with conflicting type";

    static constexpr const char* INVALID_SYNTAX_EQUI_JOIN_ON_CONDITIONAL_VAR =
            "Invalid syntax, implicit equi-joins on conditional variable `{}` are prohibited";

    static constexpr const char* INVALID_SYNTAX_EQUI_JOIN_ON_GROUP_VAR =
            "Invalid syntax, implicit equi-joins on group variable `{}` are prohibited";

    static constexpr const char* INVALID_SYNTAX_VAR_TYPE_ERROR =
            "Invalid syntax, type of Variable `{}` is not {}";

    static constexpr const char* INVALID_SYNTAX_VAR_NOT_DEFINED =
            "Invalid syntax, variable `{}` not defined";

    static constexpr const char* INVALID_SYNTAX_INSERT_BOUND_NODE =
            "Invalid syntax, cannot insert bound node `{}`";

    static constexpr const char* INVALID_SYNTAX_VAR_INVALID_NODE_TYPE =
            "Invalid syntax, `{}` does not refer to a valid node type";

    static constexpr const char* INVALID_SYNTAX_VAR_REFER_MULTIPLE_NODE_TYPE =
            "Invalid syntax, `{}` shouldn't refer to multiple node types";

    static constexpr const char* INVALID_SYNTAX_REDEFINED_VAR =
            "Invalid syntax, redefined variable: `{}`";

    static constexpr const char* INVALID_SYNTAX_CONFLICT_VAR_TYPE =
            "Invalid syntax, redefined variable: `{}` with conflict type(`{}` vs `{}`)";

    static constexpr const char* INVALID_SYNTAX_YIELD_ITEM_TYPE =
            "Invalid syntax, yield item should be NODE, EDGE or PATH type";

    static constexpr const char* INVALID_SYNTAX_MULTIPLE_NODE_INSERT =
            "Invalid syntax, cannot insert node `{}` more than once";

    static constexpr const char* INVALID_SYNTAX_DUPLICATED_PROPERTY_NAME =
            "Invalid syntax, duplicate property names in `{}`";

    static constexpr const char* INVALID_SYNTAX_DUPLICATED_INDEX_PROPERTY =
            "Invalid syntax, duplicate index property names in `{}`";

    static constexpr const char* INVALID_SYNTAX_CONFLICT_PROPERTY_VALUE_TYPE =
            "Invalid syntax, property `{}` of type `{}` defined conflict type: `{}`, previous "
            "defined as `{}`";

    static constexpr const char* INVALID_SYNTAX_ALTER_ELEMENT_MULTI_TIME =
            "Invalid syntax, `{}` cannot occur more than once in an alter graph type "
            "statement.";

    static constexpr const char* INVALID_SYNTAX_ADD_PRIMARY_KEY_OR_MULTI_EDGE_KEY =
            "Invalid syntax, adding PRIMARY KEY or MULTI EDGE KEY `{}` is not allowed.";

    static constexpr const char* INVALID_SYNTAX_ADD_INVALID_PROPERTY =
            "Invalid syntax, add not nullable property {} without default value is not "
            "allowed.";

    static constexpr const char* INVALID_SYNTAX_ELEMENT_TYPE_NAME_REDEFINE =
            "Invalid syntax, `{}` cannot be defined multiple times.";

    static constexpr const char* INVALID_SYNTAX_ILLEGAL_BYTE_STRING =
            "Invalid syntax, illegal byte string: {}";

    static constexpr const char* INVALID_SYNTAX_LABEL_EXPR_ELEMENT_TYPE_EXPR_COEXIST =
            "Invalid syntax, label expression and type expression cannot coexist in single {} "
            "pattern `{}`";

    static constexpr const char* INVALID_SYNTAX_FIELD_NUMBER_MISMATCH =
            "Invalid syntax, the number of fields in `{}` does not match with `{}`.";

    static constexpr const char* INVALID_SYNTAX_FIELD_NOT_FOUND =
            "Invalid syntax, field `{}` not found in table.";

    static constexpr const char* INVALID_IDENTIFIER =
            "Invalid syntax, illegal identifier: `{}`, error: {}";

    // MUTATE
    static constexpr const char* DEPENDENT_OBJECT_ERROR =
            "Dependent object error, node {} can not be deleted, {} edge connected";

    static constexpr const char* PRIMARY_KEY_CONSTRAINT_VIOLATION =
            "Insert node failed, primary key constraint violation, node already exist";

    static constexpr const char* EDGE_RANK_CONSTRAINT_VIOLATION =
            "Insert edge failed, rank constraint violation, edge already exist";

    static constexpr const char* NODE_IS_MISSING = "Insert edge failed, node `{}` is missing.";

    static constexpr const char* GET_NODE_ID_FROM_PRIMARY_KEY_FAILED =
            "Get node id from primary key failed : in part={}, nodeType={}, primary key "
            "first={}";

    static constexpr const char* MULTI_GET_NODE_ID_FROM_PRIMARY_KEY_FAILED =
            "Get id from primary keys failed in part {}";

    static constexpr const char* SRC_NODE_NOT_EXISTED = "Source node not existed";

    static constexpr const char* DST_NODE_NOT_EXISTED = "Destination node not existed";

    static constexpr const char* SET_INVALID_TYPE =
            "The type of element {} updated in the set statement must be Node or Edge";

    static constexpr const char* NODE_ALREADY_EXIST = "Node already exist, id: `{}`";

    // TODO (codesigner) return edge key
    static constexpr const char* EDGE_ALREADY_EXIST = "Edge already exist";

    static constexpr const char* DISCRIMINATOR_NOT_FOUND_IN_PROPS =
            "Discriminator property name `{}` is not found in property set";

    // STORAGE
    static constexpr const char* STORAGE_EXECUTOR_PARAMETER_INVALID =
            "Storage executor invalid parameter";

    static constexpr const char* STORAGE_EXECUTOR_PARAMETER_TYPE_INVALID =
            "{} should be int64, but got: `{}`, value: `{}`";

    static constexpr const char* STORAGE_EXECUTOR_PARAMETER_PRIMARY_KEY_INVALID =
            "Primary key is not consistent with latest schema";

    static constexpr const char* LEADER_CHANGED = "Leader changed";

    static constexpr const char* STORAGE_CODEC_PROPERTY_NOT_NULLABLE =
            "Property not nullable: {}";

    static constexpr const char* STORAGE_CODEC_PROPERTY_NOT_SET = "Property not set: {}";

    static constexpr const char* STORAGE_CODEC_PROPERTY_NOT_FOUND = "Property not found: {}";

    static constexpr const char* STORAGE_CODEC_PROPERTY_TYPE_NOT_MATCH =
            "Property type not match, property: `{}`:`{}`, got: `{}`:`{}`";

    static constexpr const char* STORAGE_CODEC_PROPERTY_UNABLE_TO_ENCODE =
            "Property type unable to encode: `{}`";

    static constexpr const char* STORAGE_CODEC_PROPERTY_OUT_OF_RANGE =
            "Property out of range, property: `{}`:`{}` got: `{}`:`{}`";

    static constexpr const char* STORAGE_CODEC_PROPERTY_LIST_NEST_TYPE_NOT_FOUND =
            "Property list nested property not found: {}";

    static constexpr const char* STORAGE_CODEC_INDEX_CORRUPTION_NO_TERMINATOR =
            "Index corruption: no terminator found";

    static constexpr const char* STORAGE_CODEC_INDEX_PROPERTY_NOT_REQUIRED =
            "Index property not required";

    static constexpr const char* STORAGE_CODEC_INDEX_PROPERTIES_ILLEGAL =
            "Index property type illegal";

    static constexpr const char* STORAGE_CODEC_INDEX_ILLEGAL =
            "Index corruption: index illegal";

    static constexpr const char* STORAGE_CODEC_INDEX_PROPERTIES_ILLEGAL_PROPERTY_NOT_EXIST =
            "The required property does not exist in index: {}";

    static constexpr const char* STORAGE_CODEC_INDEX_ILLEGAL_NOT_RESERVED_PROPERTY =
            "`{}` is not a reserved properties";

    static constexpr const char* KVSTORE_OPEN_FAILED = "Open kvstore failed: {}";

    static constexpr const char* KVSTORE_LOCK_FAILED = "KVStore lock failed";

    static constexpr const char* KVSTORE_COLUMN_FAMILY_NOT_FOUND =
            "KVStore column family not found";

    static constexpr const char* KVSTORE_INGEST_FAILED = "KVStore ingest failed";

    static constexpr const char* KVSTORE_SET_PROPERTY_FAILED = "KVStore set property failed";

    static constexpr const char* KVSTORE_GET_PROPERTY_FAILED = "KVStore get property failed";

    static constexpr const char* KVSTORE_COMPACT_FAILED = "KVStore compact failed";

    static constexpr const char* KVSTORE_FLUSH_FAILED = "KVStore flush failed";

    static constexpr const char* KVSTORE_CHECKPOINT_FAILED = "KVStore checkpoint failed, {}";

    static constexpr const char* KVSTORE_BATCH_WRITE_FAILED = "KVStore batch write failed";

    static constexpr const char* KVSTORE_COMMIT_FAILED = "KVStore commit failed";

    static constexpr const char* KVSTORE_KEY_NOT_FOUND = "KVStore key not found";

    static constexpr const char* KVSTORE_GET_FAILED = "KVStore get failed";

    static constexpr const char* KVSTORE_PUT_FAILED = "KVStore put failed";

    static constexpr const char* KVSTORE_REMOVE_FAILED = "KVStore remove failed";

    static constexpr const char* KVSTORE_EXPORT_TABLE_FAILED =
            "KVStore export table failed, {}";

    static constexpr const char* KVSTORE_OPEN_SST_FAILED = "KVStore open sst file `{}` failed";

    static constexpr const char* RAFT_BUSY = "Raft busy";

    static constexpr const char* RAFT_INVALID_PEER = "Raft invalid peer";

    static constexpr const char* RAFT_LEADER_CONFLICT = "Raft leader conflict";

    // CONFIGURATION
    static constexpr const char* CONFIGURATION_INVALID = "Configuration invalid: `{}`";

    static constexpr const char* CONFIGURATION_ITEM_NOT_FOUND =
            "Configuration item `{}` not found";

    static constexpr const char* CONFIGURATION_ITEM_TYPE_INVALID =
            "Invalid config value type `{}` for key `{}`, expect `{}`";

    static constexpr const char* CONFIGURATION_ITEM_RANGE_INVALID =
            "Invalid config value `{}`, valid range: `{}` for key `{}`";

    static constexpr const char* CONFIGURATION_INFO_FILE_FORMAT_ILLEGAL =
            "Info file format illegal";

    static constexpr const char* CONFIGURATION_INVALID_CONFIG_KEY = "Invalid config key `{}`";

    static constexpr const char* CONFIGURATION_INVALID_AUDIT_CATEGORY =
            "Invalid audit log category `{}`";

    static constexpr const char* CONFIGURATION_MISSING_CONFIG_KEY =
            "Missing `{}` configuration";

    static constexpr const char* CONFIGURATION_ILLEGAL_FORMAT =
            "Configuration illegal format: {}";


    // PROCEDURE
    static constexpr const char* PROCEDURE_ERROR = "Procedure error: {}";

    static constexpr const char* PROCEDURE_NOT_FOUND = "Procedure `{}` not found";

    static constexpr const char* PROCEDURE_NO_OUTPUT = "Procedure `{}` has no output";

    static constexpr const char* PROCEDURE_NUM_ARGS_NOT_ENOUGH =
            "The number of arguments for procedure `{}` is not enough, expect: {}, got {}";

    static constexpr const char* PROCEDURE_NUM_ARGS_EXCEEDED =
            "The number of arguments passed in exceeds the max allowed for procedure `{}`: {} "
            "> {}";

    static constexpr const char* PROCEDURE_INVALID_ARGUMENT_TYPE =
            "Invalid type of `{}` argument: `{}`, type:`{}`, expect:`{}`";

    static constexpr const char* PROCEDURE_INVALID_ARGUMENT_ENCODE =
            "Invalid encoding of `{}` argument: `{}`, expect encoding:`{}`, error: {}";

    static constexpr const char* PROCEDURE_YIELD_COLUMN_ERROR =
            "The yield column `{}` of procedure `{}` has conflicts with the "
            "previous statement, please use an alias to distinguish them";

    static constexpr const char* PROCEDURE_UNKNOWN_YIELD_COLUMN =
            "Unknown yield column `{}` for procedure `{}`";

    static constexpr const char* GET_PART_ROUTER_FAILED = "Get partition router failed";

    static constexpr const char* GET_META_CLIENT_FAILED = "Get meta client failed";

    // META
    static constexpr const char* META_CLUSTER_ALREADY_EXISTS = "Cluster already exists: {}";

    static constexpr const char* META_CLUSTER_NOT_FOUND = "Cluster not found: {}";

    static constexpr const char* META_GET_KV_FAILED = "MetaStore::get failed";

    static constexpr const char* META_GET_KV_PREFIX_FAILED = "MetaStore::prefix failed";

    static constexpr const char* META_SERVICE_HOST_NOT_ADDED =
            "No matched service from host: {}";

    static constexpr const char* META_FAILED_CHECK_SERVICE = "Failed to check service";

    static constexpr const char* META_FAILED_CHECK_SERVICE_EXIST =
            "Failed to check service exist: {}";

    static constexpr const char* META_SERVICE_ALREADY_EXISTS =
            "Service for host already exists";

    static constexpr const char* META_HOST_ALREADY_EXISTS = "Host already exists: {}";
    static constexpr const char* META_HOST_NOT_FOUND = "Host not found: {}";

    static constexpr const char* META_AGENT_ALREADY_EXISTS = "Agent already exists: {}";

    static constexpr const char* META_SERVICE_STATIC_PORT_ALREADY_EXISTS =
            "Service with static port already exists";
    static constexpr const char* META_SERVICE_DYNAMIC_PORT_ALREADY_EXISTS =
            "Service with dynamic port already exists";

    static constexpr const char* META_FAILED_ADD_SERVICE = "Failed to add service: {}";

    static constexpr const char* META_PUT_SERVICE_ADDRESS_FAILED = "Put service address failed";

    static constexpr const char* META_LIST_SERVICE_FAILED = "List services failed";

    static constexpr const char* META_SERVICE_REPORT_NOT_MATCH_ADDED =
            "Report service not match added";

    static constexpr const char* META_PART_CONFIG_INVALID =
            "Can't find proper replica number under given parts per service config";

    static constexpr const char* META_NOT_ENOUGH_LOCATION =
            "No enough services({}) to hold replica({})";

    static constexpr const char* META_NOT_SUPPORTED = "Meta not support, Unknown opType: {}";

    static constexpr const char* META_NOT_INIT = "Meta not init";

    static constexpr const char* META_FETCH_SERVICE_KEY_FAILED = "Failed to fetch service key";

    static constexpr const char* META_SERVICE_NOT_FOUND = "Service not found: {}";

    static constexpr const char* META_SERVICE_NOT_ADDED = "Service not added";

    static constexpr const char* META_SERVICE_IN_USE = "Service is in part";

    static constexpr const char* META_SERVICE_NOT_IN_CLUSTER = "Service not in cluster";

    static constexpr const char* META_PURGE_SERVICE_INFO_FAILED =
            "Failed to purge service info";

    static constexpr const char* META_LOCATION_NOT_FOUND = "Location {} not found";

    static constexpr const char* META_LOCATION_ALREADY_EXIST = "Location {} already exist";

    static constexpr const char* META_PART_ALREADY_EXIST = "Meta Part {} already exist";

    static constexpr const char* META_PART_LEADER_NOT_ENOUGH =
            "Meta Part with leader not enough";

    static constexpr const char* META_PART_NOT_ENOUGH = "Meta Part not enough";

    static constexpr const char* META_PART_NOT_FOUND = "Meta Part not found: `{}`";

    static constexpr const char* META_ACTIVE_SERVICE_NOT_ENOUGH = "Active service not enough";

    static constexpr const char* META_CANNOT_FIND_SERVICE = "Can't find service: {}";

    static constexpr const char* META_SERVICE_ID_ERROR =
            "Failed to fetch service id from meta: {}";

    static constexpr const char* META_REPORT_ADDRESS_ERROR = "Failed to report address to meta";

    static constexpr const char* META_UNSUPPORTED_SERVICE = "Unsupported service type";
    static constexpr const char* META_SERVICE_FAIL_TO_ALLOC_ID =
            "Failed to allocate service id: {}";
    static constexpr const char* META_DROP_CLUSTER_IS_NOT_EMPTY = "Cluster is not empty.";

    // UNSUPPORTED
    static constexpr const char* UNSUPPORTED = "Unsupported: {}";

    static constexpr const char* UNSUPPORTED_NODE_TYPE_WITHOUT_PRIMARY_KEY =
            "Unsupported, node type without primary key: `{}`";

    static constexpr const char* UNSUPPORTED_GRAPH_TYPE_EXPR =
            "Unsupported graph type expression: `{}`";

    static constexpr const char* UNSUPPORTED_GRAPH_TYPE_INITIALIZER =
            "Unsupported graph type initializer: `{}`";

    static constexpr const char* UNSUPPORTED_EDGE_DIRECTION =
            "Unsupported edge direction: `{}`";

    static constexpr const char* UNSUPPORTED_INSERT_EDGE_STATEMENT =
            "InsertEdge Statement doesn't support pattern like `{}`";

    static constexpr const char* UNSUPPORTED_INSERT_NODE_STATEMENT =
            "InsertNode Statement doesn't support pattern like `{}`";

    static constexpr const char* UNSUPPORTED_DATE_MODIFY_STATEMENT =
            "Adding any statement after data modify statement is not supported.";

    static constexpr const char* UNSUPPORTED_MULTIPLE_PATH_PATTERN =
            "Not supported path pattern with more than one path term";

    static constexpr const char* UNSUPPORTED_PATH_FACTOR = "Not supported path factor: {}";

    static constexpr const char* UNSUPPORTED_UPDATE_PRIMARY_KEY =
            "Updating the primary key is not supported, node type: `{}`, property name: `{}`";

    static constexpr const char* UNSUPPORTED_SET_MULTIPLE_NODE_TYPE =
            "Unsupported set multiple node types";

    static constexpr const char* UNSUPPORTED_SET_ALL_SET_LABEL =
            "SetAllProperties and kSetLabel are not supported yet";

    static constexpr const char* UNSUPPORTED_UPDATE_MULTIEDGE_KEY =
            "Updating the multiedge key is not supported, egde type: `{}`, property name: `{}`";

    static constexpr const char* UNSUPPORTED_TRIM_NON_ASCII_CHAR =
            "Trim functions with custom trim characters and non-ASCII inputs are not supported "
            "yet";

    static constexpr const char* UNSUPPORTED_TYPE_GET_PROP = "ValueType {} can't get property";

    static constexpr const char* UNSUPPORTED_AGGREGATION_EXPRESSION_NESTED =
            "Unsupported, aggregate expressions cannot be nested: `{}`";

    // TODO (jie.wang) refine error message
    static constexpr const char* AGGREGATION_EXPRESSION_NOT_ALLOWED =
            "Aggregate function `{}` is not allowed in this context";

    static constexpr const char* UNDEFINED_AGG_FUNCTION = "Unknown aggregate function `{}`";

    static constexpr const char* UNSUPPORTED_CHARSET = "Charset `{}` not support";

    static constexpr const char* UNSUPPORTED_COLLATION = "Collation `{}` not support";

    static constexpr const char* UNSUPPORTED_CHARSET_COLLATION_NOT_MATCH =
            "Charset `{}` and Collation `{}` not match";

    // SESSION
    static constexpr const char* QUERY_ID_NOT_FOUND = "Query id not found: {}";

    static constexpr const char* SESSION_NOT_FOUND = "Session not found: {}";

    static constexpr const char* SESSION_IS_BUSY = "Session is busy";

    static constexpr const char* KILL_QUERY_FAILED = "Kill query failed: {}";

    static constexpr const char* MAX_EXECUTION_TIME_EXCEEDED =
            "Max execution time exceeded, query was killed.";

    static constexpr const char* KILL_SESSION_FAILED = "Kill session failed: {}";

    // Authorization runtime error
    static constexpr const char* CREATE_ROLE_FAILED = "Create role failed: {}";
    static constexpr const char* ALTER_ROLE_FAILED = "Alter role failed: {}";
    static constexpr const char* DROP_ROLE_FAILED = "Drop role failed: {}";
    static constexpr const char* ALTER_OWNER_FAILED = "Alter Owner failed: {}";
    static constexpr const char* GRANT_ROLE_FAILED = "Grant role failed: {}";
    static constexpr const char* REVOKE_ROLE_FAILED = "Revoke role failed: {}";
    static constexpr const char* GRANT_PRIVILEGE_FAILED = "Grant privilege failed: {}";
    static constexpr const char* REVOKE_PRIVILEGE_FAILED = "Revoke privilege failed: {}";

    // GRAPH COMPUTING
    static constexpr const char* GRAPH_COMPUTE_ERROR = "Graph compute error: {}";

    // JOB
    static constexpr const char* JOB_DESCRIPTION_INVALID = "Job description invalid: {}";

    // PLUGIN
    static constexpr const char* PLUGIN_INVALID_TYPE = "Invalid plugin type `{}` for {}";

    static constexpr const char* PLUGIN_MULTIPLE_PLUGIN =
            "Multiple plugins found, {} found for {}";

    static constexpr const char* PLUGIN_CONFIG_NOT_FOUND =
            "Plugin `{}`, configuration not found: `{}`";

    static constexpr const char* PLUGIN_CONFIG_PARSE_ERROR =
            "Plugin `{}`, parse configuration error: `{}`";

    static constexpr const char* PLUGIN_INIT_ERROR = "Plugin `{}`, init error: `{}`";

    static constexpr const char* PLUGIN_DESTROY_ERROR = "Plugin `{}`, destroy error: `{}`";

    static constexpr const char* PLUGIN_UNSUPPORT_AUTH_TYPE =
            "Unsupported authentication plugin: {}";
    // MODULE
    static constexpr const char* MODULE_ALREADY_LOADED = "Module already loaded: `{}`";

    static constexpr const char* MODULE_NOT_FOUND = "Module not found: `{}`";

    static constexpr const char* MODULE_BEING_USED = "Module is being used: `{}`";

    static constexpr const char* MODULE_INVALID_PROC_NAME = "Invalid procedure name: `{}`";

    static constexpr const char* MODULE_INVALID_FUNC_NAME = "Invalid function name: `{}`";

    static constexpr const char* MODULE_INVALID_PLUGIN_NAME = "Invalid plugin name: `{}`";

    static constexpr const char* MODULE_INVALID_PLUGIN_API_VERSION =
            "Invalid plugin api version: `{}`";

    static constexpr const char* MODULE_INVALID_NULL_PLUGIN = "Invalid null plugin";

    static constexpr const char* MODULE_INVALID_REGISTER =
            "Invalid register `{}` for module: `{}`";

    static constexpr const char* MODULE_DLOPEN_ERROR =
            "Module dlopen error, name: {} error: {}";

    static constexpr const char* MODULE_CANNOT_UNLOAD =
            "The Module `{}` can not be unloaded or reloaded";

    // Authenticate
    static constexpr const char* AUTH_INVALID_PASSWORD =
            "The password must be at least 8 characters long and should include at least one "
            "digit, one lowercase letter, and one uppercase letter";

    static constexpr const char* AUTH_USER_EXIST = "User `{}` exist";
    static constexpr const char* AUTH_USER_NOT_EXIST = "User `{}` not exist";
    static constexpr const char* AUTH_USER_DISABLED = "User `{}` disabled";
    static constexpr const char* AUTH_USER_LOCKED =
            "Login blocked for {} minutes due to too many failed attempts";
    static constexpr const char* AUTH_INVALID_USERNAME =
            "The username should be between 4 and 32 characters in length and can only include "
            "letters, numbers, and underscores";

    static constexpr const char* AUTH_INVALID_USERNAME_OR_PASSWORD =
            "Authenticate error: invalid username or password";
    static constexpr const char* AUTH_CANNOT_DISABLE_SYSTEM_USER =
            "Authenticate error: cannot disable system user";
    static constexpr const char* AUTH_NEED_CHANGE_PASSWORD =
            "Authenticate error: need to change password";

    static constexpr const char* AUTH_INVALID_AUTH_INFO =
            "Authenticate error: invalid config {}";

    static constexpr const char* AUTH_INVALID_TOKEN = "Authenticate error: invalid token";

    static constexpr const char* AUTH_INIT_ERROR = "AuthenticatorMangager init error: `{}`";

    // Authorization
    static constexpr const char* AUTHORIZE_ERROR = "Authorizate error: `{}`";
    static constexpr const char* AUTHORIZE_INVALID_ROLENAME =
            "Authorizate error: The role name should be between 4 and 32 characters in length "
            "and can only include letters, numbers, and underscores";

    static constexpr const char* AUTHORIZE_INVALID_OBJECT =
            "Authorizate error: Invalid object name `{}`";

    static constexpr const char* AUTHORIZE_ROLE_EXIST = "Role `{}` exist";
    static constexpr const char* AUTHORIZE_ROLE_NOT_EXIST = "Role `{}` not exist";
    static constexpr const char* AUTHORIZE_USER_NOT_EXIST =
            "User `{}` not exist in this cluster";
    static constexpr const char* AUTHORIZE_USER_EXIST = "User `{}` exist in this cluster";

    static constexpr const char* AUTHORIZE_INSUFFICIENT_PRIVILEGE =
            "Insufficient privilege: `{}`";
    static constexpr const char* AUTHORIZE_CONSTRAINT = "Cannot grant or revoke `{}` on `{}`";

    // Session
    static constexpr const char* CREATE_SESSION_FAILED = "Failed to create session, {}";
    static constexpr const char* UNSUPPORTED_PROTOCOL_VERSION =
            "Your client protocol version({}) is unsupported. Please use one of the following "
            "supported protocol version clients to access: {}.";
    static constexpr const char* SESSION_SET_ERROR =
            "Setting the session parameter: {} failed because: {}.";

    // RPC
    static constexpr const char* RPC_DESERIALIZE_OBJECT_TYPE_ERROR =
            "RPC deserialize error, unknown object type: `{}` ";

    // SYSTEM
    static constexpr const char* NETWORK_ERROR = "Network error: {}: {}";

    static constexpr const char* FILE_CREATE_DIR_FAILED = "Create `{}` directory failed";

    static constexpr const char* FILE_READ_SERVICE_ID_FILE_ERROR =
            "Failed to read service id file";

    static constexpr const char* FILE_PARSE_SERVICE_ID_FILE_ERROR =
            "Failed to parse service id file";

    static constexpr const char* FILE_SAVE_SERVICE_ID_FILE_ERROR =
            "Failed to save service id to file";

    static constexpr const char* FILE_REMOVE_DIR_FAILED = "Remove `{}` directory failed";

    static constexpr const char* NO_VALIDATOR = "No validator implemented for `{}`";

    static constexpr const char* UNKNOWN = "Unknown error";

    static constexpr const char* MEMORY_EXCEEDED = "Memory exceeded";
    static constexpr const char* QUERY_CANCELED = "Query is canceled by user";

    static constexpr const char* MEMORY_CONFIG_ERROR =
            "[MEM] configured max memory {} MiB, exceed system total memory {}";

    static constexpr const char* MEMORY_BACKGROUND_CHECK =
            "Memory background thread start failed";

    static constexpr const char* MEMORY_SYSTEM_INFO =
            "Read system memory info from /proc/meminfo failed";

    static constexpr const char* MEMORY_VECTOR_INVALID_SIZE =
            "Invalid vector size, which should be [0, 134217727], but got {}";

    static constexpr const char* FILE_NOT_FOUND = "File not found: `{}`";

    static constexpr const char* FILE_IS_EMPTY = "File is empty: `{}`";

    static constexpr const char* FILE_READ_PERMISSION = "No permission to read file `{}`";

    static constexpr const char* FILE_READ_LINK = "Read link failed {}: {}";

    static constexpr const char* FILE_READ_PATH = "Read path failed {}: {}";

    static constexpr const char* FILE_OPEN_DIR = "Open dir failed `{}`: {}";

    static constexpr const char* FILE_OPEN_FILE = "Open file failed `{}`: {}";

    static constexpr const char* FILE_TYPE_NOT_SUPPORT = "File type not supported `{}`: {}";

    static constexpr const char* FILE_FILE_STATE = "Get file state failed of `{}`";

    static constexpr const char* NETWORK_CREATE_SOCKET = "Create socket failed: {}";

    static constexpr const char* NETWORK_UNKNOWN_ADDR = "Unknown address family: {}";

    static constexpr const char* NETWORK_BIND_FAILED = "Failed to bind on {}: {}";

    static constexpr const char* NETWORK_LISTEN_FAILED = "Failed to listen on {}: {}";

    static constexpr const char* NETWORK_NBIO_STOPPED =
            "NBIO is not started or has been stopped";

    static constexpr const char* NETWORK_CONN_CLOSED = "{}: Connection closed by remote host";

    static constexpr const char* NETWORK_PROTOCAL = "{}: Protocol Error";

    static constexpr const char* NETWORK_TIMEOUT = "Timeout";

    static constexpr const char* NETWORK_DISCONNECTED = "{}: Connection was disconnected";

    static constexpr const char* NETWORK_ADDR_NOT_FOUND = "No IPv4 address found for `{}`";

    static constexpr const char* NETWORK_INVALID_HOST = "Invalid host address: {}";

    static constexpr const char* JOB_ERROR = "JOB_ERROR: `{}`";
    static constexpr const char* JOB_TYPE_UNKNOWN = "Job type unknown: `{}`";
    static constexpr const char* JOB_NOT_FOUND = "Can't find job: `{}`";
    static constexpr const char* JOB_TASK_NOT_FOUND = "Can't find task `{}` of job `{}`";
    static constexpr const char* JOB_CAN_NOT_RUN = "Can't run job: `{}`";

    static constexpr const char* TASK_NOT_FOUND_IN_STORAGE =
            "Task not found in storage. (jobId: {}, taskId: {})";
    static constexpr const char* TASK_STOPPED = "Task stopped by outside";

    // Optimizer
    static constexpr const char* PLAN_GENERATE_FAILED =
            "Execution plan generation failed due to disabled optimizer rules. Check configs "
            "or consult documentation";

    static constexpr const char* BACKUP_ERROR = "Backup error: {}";

    static constexpr const char* BACKUP_RECORD_EXISTS =
            "A valid backup with same name already exists: `{}`";

    static constexpr const char* BACKUP_RUNNING_JOBS =
            "Cannot do backup now, some conflict jobs are running";

    static constexpr const char* BACKUP_META_TABLE_EMPTY = "Meta table `{}` is empty";

    static constexpr const char* BACKUP_LOST_PART =
            "Some parts are lost, should be {}, actually {}";

    static constexpr const char* BACKUP_DUPLICATE_PART = "Part {} is duplicate backed up";

    static constexpr const char* BACKUP_RESTORE_INVALID_REQEUST = "Invalid Restore request: {}";

    static constexpr const char* BACKUP_REMAP_PART_FAILED =
            "Part info does not match when restore";

    static constexpr const char* BACKUP_RESTORE_TABLE_FAILED = "Restore table failed: {}";

    static constexpr const char* BACKUP_DROP_NAME_EMPTY = "Empty backup name to drop";

    static constexpr const char* BACKUP_DROP_NAME_NOT_EXIST = "Backup name `{}` doesn't exist";
};

}  // namespace nebula


#define N_STATUS(TYPE) nebula::Status(nebula::ErrorCode::TYPE, nebula::ErrorMessage::TYPE)

#define V_STATUS(TYPE, ...) \
    nebula::Status(nebula::ErrorCode::TYPE, nebula::ErrorMessage::TYPE, ##__VA_ARGS__)
