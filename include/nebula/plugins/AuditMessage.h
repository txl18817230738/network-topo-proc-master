// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

#include <date/date.h>
#include <fmt/core.h>

#include "nebula/common/base/ErrorCode.h"
namespace nebula {

/**
 * Audit event types, encoded in 8 bytes bitmap, each type occupy 1 bit in the bitmap.
 * We reserved some bits if more event added to the system in the future.
 *
 * bitmap encoding:
 *    0- 7: connection
 *    9-15: security related
 *   16-31: query related
 *   32-63: job, configuration, general etc
 */
enum EventType : uint64_t {
    AUDIT_CONN_LOGIN = 1UL << 1,          /* event of connection              */
    AUDIT_CONN_SIGNOUT = 1UL << 2,        /* event of disconnection           */
    AUDIT_AUTHENTICATION = 1UL << 8,      /* event of authentication          */
    AUDIT_AUTHORIZATION_GRAPH = 1UL << 9, /* event of graph authorization     */
    AUDIT_AUTHORIZATION_NODE = 1UL << 10, /* event of node authorization      */
    AUDIT_AUTHORIZATION_EDGE = 1UL << 11, /* event of edge authorization      */
    AUDIT_AUTHORIZATION_PROP = 1UL << 12, /* event of property authorization  */
    AUDIT_QUERY_DDL = 1UL << 16,          /* event of DDL                     */
    AUDIT_QUERY_DQL = 1UL << 17,          /* event of DQL                     */
    AUDIT_QUERY_DCL = 1UL << 18,          /* event of DCL                     */
    AUDIT_QUERY_DML_INSERT = 1UL << 20,   /* event of DML insert              */
    AUDIT_QUERY_DML_SET = 1UL << 21,      /* event of DML set                 */
    AUDIT_QUERY_DML_REMOVE = 1UL << 22,   /* event of DML remove              */
    AUDIT_QUERY_DML_DELETE = 1UL << 23,   /* event of DML delete              */
    AUDIT_QUERY_PROCEDURE = 1UL << 24,    /* event of procedure               */
    AUDIT_QUERY_RETURN_ERROR = 1UL << 25, /* event of return error            */
    AUDIT_JOB_START = 1UL << 32,          /* event of job start               */
    AUDIT_JOB_END = 1UL << 33,            /* event of job end                 */
    AUDIT_CONFIGURATION = 1UL << 34,      /* event of configuration access    */
    AUDIT_GENERAL = 1UL << 35,            /* event of other general           */
    AUDIT_UNKNOWN = 1UL << 63,            /* event of unknown                 */
};

/**
 * Get string name of the event type
 */
inline std::string toString(EventType type) {
    switch (type) {
        case AUDIT_CONN_LOGIN:
            return "LOGIN";
        case AUDIT_CONN_SIGNOUT:
            return "SIGNOUT";
        case AUDIT_AUTHENTICATION:
            return "AUTHENTICATION";
        case AUDIT_AUTHORIZATION_GRAPH:
            return "AUTHORIZATION_GRAPH";
        case AUDIT_AUTHORIZATION_NODE:
            return "AUTHORIZATION_NODE";
        case AUDIT_AUTHORIZATION_EDGE:
            return "AUTHORIZATION_EDGE";
        case AUDIT_AUTHORIZATION_PROP:
            return "AUTHORIZATION_PROP";
        case AUDIT_QUERY_DDL:
            return "DDL";
        case AUDIT_QUERY_DQL:
            return "DQL";
        case AUDIT_QUERY_DCL:
            return "DCL";
        case AUDIT_QUERY_DML_INSERT:
            return "DML_INSERT";
        case AUDIT_QUERY_DML_SET:
            return "DML_SET";
        case AUDIT_QUERY_DML_REMOVE:
            return "DML_REMOVE";
        case AUDIT_QUERY_DML_DELETE:
            return "DML_DELETE";
        case AUDIT_QUERY_PROCEDURE:
            return "PROCEDURE";
        case AUDIT_QUERY_RETURN_ERROR:
            return "RETURN_ERROR";
        case AUDIT_JOB_START:
            return "JOB_START";
        case AUDIT_JOB_END:
            return "JOB_END";
        case AUDIT_CONFIGURATION:
            return "CONFIGURATION";
        case AUDIT_GENERAL:
            return "GENERAL";
        default:
            return "";
    }
}

/**
 * Get event types of the passed in category. most category is one-to-one mapping
 * from category to event type. While some category names are super categories represent
 * a class of events, e.g.
 * `DML` represent all dml event
 * `AUTHORIZATION` represent of all authorization event
 *
 * @param category
 * @return event types belongs to that category
 */
inline std::vector<EventType> fromString(std::string &category) {
    std::string uppercase;
    uppercase.resize(category.size());
    std::transform(category.begin(), category.end(), uppercase.begin(), [](unsigned char c) {
        return std::toupper(c);
    });
    if ("LOGIN" == uppercase) {
        return {AUDIT_CONN_LOGIN};
    } else if ("SIGNOUT" == uppercase) {
        return {AUDIT_CONN_SIGNOUT};
    } else if ("AUTHENTICATION" == uppercase) {
        return {AUDIT_AUTHENTICATION};
    } else if ("AUTHORIZATION" == uppercase) {
        return {AUDIT_AUTHORIZATION_GRAPH,
                AUDIT_AUTHORIZATION_NODE,
                AUDIT_AUTHORIZATION_EDGE,
                AUDIT_AUTHORIZATION_PROP};
    } else if ("AUTHORIZATION_GRAPH" == uppercase) {
        return {AUDIT_AUTHORIZATION_GRAPH};
    } else if ("AUTHORIZATION_NODE" == uppercase) {
        return {AUDIT_AUTHORIZATION_NODE};
    } else if ("AUTHORIZATION_EDGE" == uppercase) {
        return {AUDIT_AUTHORIZATION_EDGE};
    } else if ("AUTHORIZATION_PROP" == uppercase) {
        return {AUDIT_AUTHORIZATION_PROP};
    } else if ("DDL" == uppercase) {
        return {AUDIT_QUERY_DDL};
    } else if ("DQL" == uppercase) {
        return {AUDIT_QUERY_DQL};
    } else if ("DCL" == uppercase) {
        return {AUDIT_QUERY_DCL};
    } else if ("DML" == uppercase) {
        return {AUDIT_QUERY_DML_INSERT,
                AUDIT_QUERY_DML_SET,
                AUDIT_QUERY_DML_REMOVE,
                AUDIT_QUERY_DML_DELETE};
    } else if ("DML_INSERT" == uppercase) {
        return {AUDIT_QUERY_DML_INSERT};
    } else if ("DML_SET" == uppercase) {
        return {AUDIT_QUERY_DML_SET};
    } else if ("DML_REMOVE" == uppercase) {
        return {AUDIT_QUERY_DML_REMOVE};
    } else if ("DML_DELETE" == uppercase) {
        return {AUDIT_QUERY_DML_DELETE};
    } else if ("PROCEDURE" == uppercase) {
        return {AUDIT_QUERY_PROCEDURE};
    } else if ("RETURN_ERROR" == uppercase) {
        return {AUDIT_QUERY_RETURN_ERROR};
    } else if ("JOB" == uppercase) {
        return {AUDIT_JOB_START, AUDIT_JOB_END};
    } else if ("JOB_START" == uppercase) {
        return {AUDIT_JOB_START};
    } else if ("JOB_END" == uppercase) {
        return {AUDIT_JOB_END};
    } else if ("CONFIGURATION" == uppercase) {
        return {AUDIT_CONFIGURATION};
    } else if ("GENERAL" == uppercase) {
        return {AUDIT_GENERAL};
    } else {
        return {};
    }
}

/** audit base event */
struct Event {
    EventType type;
    ErrorCode code;
    int64_t authId;
    std::string clientHost;
    std::string serverHost;
    std::string user;
    std::chrono::system_clock::time_point timestamp;

    Event(EventType type_,
          ErrorCode code_,
          int64_t authId_,
          const std::string &clientHost_,
          const std::string &serverHost_,
          const std::string &user_)
            : type(type_),
              code(code_),
              authId(authId_),
              clientHost(clientHost_),
              serverHost(serverHost_),
              user(user_),
              timestamp(std::chrono::system_clock::now()) {}
};

const constexpr char *kTimestampFormat = R"(%FT%T %Z)";

#define EVENT_FORMAT(FORMATER, ...)                        \
    fmt::format(FORMATER,                                  \
                toString(type),                            \
                ErrorCodeUtils::format(code),              \
                date::format(kTimestampFormat, timestamp), \
                authId,                                    \
                clientHost,                                \
                serverHost,                                \
                user,                                      \
                ##__VA_ARGS__);

static const constexpr char *kConnectFormat = R"(<AUDIT_RECORD>"
  <TYPE>{}</TYPE>"
  <CODE>{}</CODE>"
  <TIME_STAMP>{}</TIME_STAMP>"
  <AUTH_ID>{}</AUTH_ID>"
  <CLIENT_HOST>{}</CLIENT_HOST>"
  <SERVER_HOST>{}</SERVER_HOST>"
  <USER>{}</USER>
</AUDIT_RECORD>)";

struct EventConnection : public Event {
    EventConnection(EventType type_,
                    ErrorCode code_,
                    int64_t authId_,
                    const std::string &clientHost_,
                    const std::string &serverHost_,
                    const std::string &user_)
            : Event(type_, code_, authId_, clientHost_, serverHost_, user_) {}

    std::string toXml() const {
        return fmt::format(kConnectFormat,
                           toString(type),
                           ErrorCodeUtils::format(code),
                           date::format(kTimestampFormat, timestamp),
                           authId,
                           clientHost,
                           serverHost,
                           user);
    }
};

static const constexpr char *kAuthenticationFormat = R"(<AUDIT_RECORD>"
  <TYPE>{}</TYPE>"
  <CODE>{}</CODE>"
  <TIME_STAMP>{}</TIME_STAMP>"
  <AUTH_ID>{}</AUTH_ID>"
  <CLIENT_HOST>{}</CLIENT_HOST>"
  <SERVER_HOST>{}</SERVER_HOST>"
  <USER>{}</USER>
  <CLIENG_LANG>{}</CLIENG_LANG>
  <CLIENT_PROTO_VERSION>{}</CLIENT_PROTO_VERSION>
  <CLIENT_VERSION>{}</CLIENT_VERSION>
  <MESSAGE>{}</MESSAGE>
</AUDIT_RECORD>)";

struct EventAuthentication : public Event {
    std::string clientLang;
    std::string clientProtocolVersion;
    std::string clientVersion;
    std::string message;

    EventAuthentication(EventType type_,
                        ErrorCode code_,
                        int64_t authId_,
                        const std::string &clientHost_,
                        const std::string &serverHost_,
                        const std::string &user_,
                        const std::string &clientLang_,
                        const std::string &clientProtocolVersion_,
                        const std::string &clientVersion_,
                        const std::string &message_)
            : Event(type_, code_, authId_, clientHost_, serverHost_, user_),
              clientLang(clientLang_),
              clientProtocolVersion(clientProtocolVersion_),
              clientVersion(clientVersion_),
              message(message_) {}


    std::string toXml() const {
        return EVENT_FORMAT(kAuthenticationFormat,
                            clientLang,
                            clientProtocolVersion,
                            clientVersion,
                            message);
    }
};

static const constexpr char *kAuthorizationFormat = R"(<AUDIT_RECORD>"
  <TYPE>{}</TYPE>"
  <CODE>{}</CODE>"
  <TIME_STAMP>{}</TIME_STAMP>"
  <AUTH_ID>{}</AUTH_ID>"
  <CLIENT_HOST>{}</CLIENT_HOST>"
  <SERVER_HOST>{}</SERVER_HOST>"
  <USER>{}</USER>
  <MESSAGE>{}</MESSAGE>
</AUDIT_RECORD>)";

struct EventAuthorization : public Event {
    std::string message;

    EventAuthorization(EventType type_,
                       ErrorCode code_,
                       int64_t authId_,
                       const std::string &clientHost_,
                       const std::string &serverHost_,
                       const std::string &user_,
                       const std::string &message_)
            : Event(type_, code_, authId_, clientHost_, serverHost_, user_),
              message(message_) {}

    std::string toXml() const {
        return EVENT_FORMAT(kAuthorizationFormat, message);
    }
};

enum class AdminOperationKind : uint8_t {
    // meta
    SHOW_META,
    SET_META_AGENT,
    // user
    LOGIN,
    LOGOUT,
    CREATE_USER,
    DROP_USER,
    ALTER_USER,
    LIST_USER,
    CHANGE_PASSWORD,
    // cluster
    CREATE_CLUSTER,
    LIST_CLUSTER,
    DROP_CLUSTER,
    // host info
    ADD_HOST,
    REMOVE_HOST,
    LIST_HOST,
    // service
    ADD_SERVICE,
    LIST_SERVICE,
    DROP_SERVICE,
    // storage
    INIT_STORAGE,
    // backup
    CREATE_BACKUP,
    RESTORE,
    DROP_BACKUP,
};

inline std::string toString(AdminOperationKind type) {
    switch (type) {
        case AdminOperationKind::SHOW_META:
            return "SHOW_META";
        case nebula::AdminOperationKind::SET_META_AGENT:
            return "SET_META_AGENT";
        case AdminOperationKind::LOGIN:
            return "LOGIN";
        case AdminOperationKind::LOGOUT:
            return "LOGOUT";
        case AdminOperationKind::CREATE_USER:
            return "CREATE_USER";
        case AdminOperationKind::DROP_USER:
            return "DROP_USER";
        case AdminOperationKind::ALTER_USER:
            return "ALTER_USER";
        case AdminOperationKind::LIST_USER:
            return "LIST_USER";
        case AdminOperationKind::CHANGE_PASSWORD:
            return "CHANGE_PASSWORD";
        case AdminOperationKind::CREATE_CLUSTER:
            return "CREATE_CLUSTER";
        case AdminOperationKind::LIST_CLUSTER:
            return "LIST_CLUSTER";
        case AdminOperationKind::DROP_CLUSTER:
            return "DROP_CLUSTER";
        case AdminOperationKind::ADD_HOST:
            return "ADD_HOST";
        case AdminOperationKind::REMOVE_HOST:
            return "REMOVE_HOST";
        case AdminOperationKind::LIST_HOST:
            return "LIST_HOST";
        case AdminOperationKind::ADD_SERVICE:
            return "ADD_SERVICE";
        case AdminOperationKind::LIST_SERVICE:
            return "LIST_SERVICE";
        case AdminOperationKind::DROP_SERVICE:
            return "DROP_SERVICE";
        case AdminOperationKind::INIT_STORAGE:
            return "INIT_STORAGE";
        case AdminOperationKind::CREATE_BACKUP:
            return "CREATE_BACKUP";
        case AdminOperationKind::RESTORE:
            return "RESTORE";
        case AdminOperationKind::DROP_BACKUP:
            return "DROP_BACKUP";
    }
    return "";
}

static const constexpr char *kAdminOperationFormat = R"(<AUDIT_RECORD>"
  <TYPE>{}</TYPE>"
  <CODE>{}</CODE>"
  <TIME_STAMP>{}</TIME_STAMP>"
  <AUTH_ID>{}</AUTH_ID>"
  <CLIENT_HOST>{}</CLIENT_HOST>"
  <SERVER_HOST>{}</SERVER_HOST>"
  <USER>{}</USER>
  <OPERATION_KIND>{}</OPERATION_KIND>
  <OPERATION>{}</OPERATION>
</AUDIT_RECORD>)";

struct EventAdminOperation : public Event {
    AdminOperationKind operationKind;
    std::string operation;

    EventAdminOperation(EventType type_,
                        ErrorCode code_,
                        int64_t authId_,
                        const std::string &clientHost_,
                        const std::string &serverHost_,
                        const std::string &user_,
                        AdminOperationKind operationKind_,
                        const std::string &operation_)
            : Event(type_, code_, authId_, clientHost_, serverHost_, user_),
              operationKind(operationKind_),
              operation(operation_) {}

    std::string toXml() const {
        return EVENT_FORMAT(kAdminOperationFormat, toString(operationKind), operation);
    }
};

static const constexpr char *kQueryFinishFormat = R"(<AUDIT_RECORD>"
  <TYPE>{}</TYPE>"
  <CODE>{}</CODE>"
  <TIME_STAMP>{}</TIME_STAMP>"
  <AUTH_ID>{}</AUTH_ID>"
  <CLIENT_HOST>{}</CLIENT_HOST>"
  <SERVER_HOST>{}</SERVER_HOST>"
  <USER>{}</USER>
  <GRAPH>{}</GRAPH>
  <QUERY_ID>{}</QUERY_ID>
  <QUERY>{}</QUERY>
</AUDIT_RECORD>)";

struct EventQueryFinish : public Event {
    std::string graph;
    std::string queryId;
    std::string query;

    EventQueryFinish(EventType type_,
                     ErrorCode code_,
                     int64_t authId_,
                     const std::string &clientHost_,
                     const std::string &serverHost_,
                     const std::string &user_,
                     const std::string &graph_,
                     const std::string &queryId_,
                     const std::string &query_)
            : Event(type_, code_, authId_, clientHost_, serverHost_, user_),
              graph(graph_),
              queryId(queryId_),
              query(query_) {}

    std::string toXml() const {
        return EVENT_FORMAT(kQueryFinishFormat, graph, queryId, query);
    }
};

static const constexpr char *kQueryErrorFormat = R"(<AUDIT_RECORD>"
  <TYPE>{}</TYPE>"
  <CODE>{}</CODE>"
  <TIME_STAMP>{}</TIME_STAMP>"
  <AUTH_ID>{}</AUTH_ID>"
  <CLIENT_HOST>{}</CLIENT_HOST>"
  <SERVER_HOST>{}</SERVER_HOST>"
  <USER>{}</USER>
  <GRAPH>{}</GRAPH>
  <QUERY_ID>{}</QUERY_ID>
  <QUERY>{}</QUERY>
  <ERROR>{}</ERROR>
</AUDIT_RECORD>)";

struct EventQueryError : public Event {
    std::string graph;
    std::string queryId;
    std::string query;
    std::string error;

    EventQueryError(EventType type_,
                    ErrorCode code_,
                    int64_t authId_,
                    const std::string &clientHost_,
                    const std::string &serverHost_,
                    const std::string &user_,
                    const std::string &graph_,
                    const std::string &queryId_,
                    const std::string &query_,
                    const std::string &error_)
            : Event(type_, code_, authId_, clientHost_, serverHost_, user_),
              graph(graph_),
              queryId(queryId_),
              query(query_),
              error(error_) {}

    std::string toXml() const {
        return EVENT_FORMAT(kQueryErrorFormat, graph, queryId, query, error);
    }
};

static const constexpr char *kJobFormat = R"(<AUDIT_RECORD>"
  <TYPE>{}</TYPE>"
  <CODE>{}</CODE>"
  <TIME_STAMP>{}</TIME_STAMP>"
  <AUTH_ID>{}</AUTH_ID>"
  <CLIENT_HOST>{}</CLIENT_HOST>"
  <SERVER_HOST>{}</SERVER_HOST>"
  <USER>{}</USER>
  <GRAPH>{}</GRAPH>
  <COMMAND_ID>{}</COMMAND_ID>
  <COMMAND>{}</COMMAND>
  <ERROR>{}</ERROR>
</AUDIT_RECORD>)";

struct EventJob : public Event {
    std::string graph;
    std::string commandId;
    std::string command;
    std::string error;

    EventJob(EventType type_,
             ErrorCode code_,
             int64_t authId_,
             const std::string &clientHost_,
             const std::string &serverHost_,
             const std::string &user_,
             const std::string &graph_,
             const std::string &commandId_,
             const std::string &command_,
             const std::string &error_)
            : Event(type_, code_, authId_, clientHost_, serverHost_, user_),
              graph(graph_),
              commandId(commandId_),
              command(command_),
              error(error_) {}

    std::string toXml() {
        return EVENT_FORMAT(kJobFormat, graph, commandId, command, error);
    }
};

enum ConfigurationType {
    SET,         /* set to a specific value */
    SET_DEFAULT, /* set to default value */
    GET          /* get value */
};

inline std::string toString(ConfigurationType type) {
    switch (type) {
        case SET:
            return "SET";
        case SET_DEFAULT:
            return "SET_DEFAULT";
        case GET:
            return "GET";
        default:
            return "";
    }
}

static const constexpr char *kConfigurationFormat = R"(<AUDIT_RECORD>"
  <TYPE>{}</TYPE>"
  <CODE>{}</CODE>"
  <TIME_STAMP>{}</TIME_STAMP>"
  <AUTH_ID>{}</AUTH_ID>"
  <CLIENT_HOST>{}</CLIENT_HOST>"
  <SERVER_HOST>{}</SERVER_HOST>"
  <USER>{}</USER>
  <CONFIGURATION_TYPE>{}</CONFIGURATION_TYPE>
  <NAME>{}</NAME>
  <VALUE>{}</VALUE>
</AUDIT_RECORD>)";

struct EventConfiguration : public Event {
    ConfigurationType configurationType;
    std::string name;  /* configuration name                      */
    std::string value; /* configuration value formatted to string */

    EventConfiguration(EventType type_,
                       ErrorCode code_,
                       int64_t authId_,
                       const std::string &clientHost_,
                       const std::string &serverHost_,
                       const std::string &user_,
                       ConfigurationType configurationType_,
                       const std::string &name_,
                       const std::string &value_)
            : Event(type_, code_, authId_, clientHost_, serverHost_, user_),
              configurationType(configurationType_),
              name(name_),
              value(value_) {}

    std::string toXml() const {
        return EVENT_FORMAT(kConfigurationFormat, toString(configurationType), name, value);
    }
};

inline std::ostream &operator<<(std::ostream &os, const EventConnection &event) {
    return os << event.toXml();
}

inline std::ostream &operator<<(std::ostream &os, const EventAuthentication &event) {
    return os << event.toXml();
}

inline std::ostream &operator<<(std::ostream &os, const EventAuthorization &event) {
    return os << event.toXml();
}

inline std::ostream &operator<<(std::ostream &os, const EventQueryFinish &event) {
    return os << event.toXml();
}

inline std::ostream &operator<<(std::ostream &os, const EventQueryError &event) {
    return os << event.toXml();
}

inline std::ostream &operator<<(std::ostream &os, const EventConfiguration &event) {
    return os << event.toXml();
}

inline std::ostream &operator<<(std::ostream &os, const EventAdminOperation &event) {
    return os << event.toXml();
}

}  // namespace nebula
