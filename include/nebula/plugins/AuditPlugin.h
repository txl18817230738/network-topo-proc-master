// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include "nebula/plugins/AuditMessage.h"
#include "nebula/plugins/Plugin.h"

#define CACHE_LINE_SIZE 64

namespace nebula {

class LogStreamVoidify {
public:
    void operator&(std::ostream& s) {
        // flush triggers LogBuf::sync(), which invoke
        // Subclass::record(std::string formattedLog).
        //
        // NODE: audit log's persistence strategy is Subclass implement independent,
        //       abstract super class AuditPlugin do not decide how to persist log.
        s.flush();
    }
};

namespace audit {

/**
 * A audit event mask that filter which events to be audited.
 */
struct AuditMask {
    alignas(CACHE_LINE_SIZE) uint64_t mask_{0};

    /**
     * Add the event type to the collection of events to audit.
     */
    void add(EventType type) {
        mask_ = mask_ | type;
    }

    /**
     * Return true when this type of event should be audited
     */
    FOLLY_ALWAYS_INLINE bool filter(EventType type) const {
        return mask_ & type;
    }
};

/**
 * Base class for all audit plugins.
 */
class AuditPlugin : public plugin::Plugin {
    // an internal log buffer help invoke record()
    class LogBuf : public std::stringbuf {
    public:
        LogBuf() {}

        int sync() override {
            this->auditor_->record(this->str());
            this->buf_.clear();
            this->str(buf_);
            return 0;
        }

        inline void setAuditor(AuditPlugin* auditor) {
            auditor_ = auditor;
        }

    private:
        std::string buf_;
        AuditPlugin* auditor_;
    };

    struct LogStream {
        LogStream() : buf{}, ostream(&buf) {}
        LogBuf buf;
        std::ostream ostream;
    };

public:
    static const constexpr char* kAuditLogCategories = "audit_log_categories";

public:
    explicit AuditPlugin(const plugin::PluginInfo& info) : plugin::Plugin(info) {}

    /**
     * Initialize this audit plugin.
     * NOTE: subclass should invoke AuditPlugin::init()
     *       if subclass have their own override init() function
     *
     * @param config
     * @return
     */
    Status init(const std::unordered_map<std::string, std::string>& config) override;

    /**
     * Return true when this audit plugin need audit specific type of event.
     *
     * NOTE: This filter function is provided to enable checking whether AuditPlugin
     * interested specific type of event without create the AuditMessage object.
     * @related AUDIT_LOG(AUDITOR, TYPE)
     */
    FOLLY_ALWAYS_INLINE bool filter(EventType type) const {
        return mask_.filter(type);
    }

    /**
     * Return a ostream that audit message should be streamed to.
     * @related AUDIT_LOG(AUDITOR, TYPE)
     */
    FOLLY_ALWAYS_INLINE std::ostream& stream() {
        logStream_.buf.setAuditor(this);
        return logStream_.ostream;
    }

    /**
     * Subclass need override this function to record formatted log.
     * The persistence of the log is subclass implement independent.
     *
     * e.g. a file-based implementation may persist to disk.
     *      a network-based (e.g.kafka) implementation may transfer to remote server.
     *
     * @param formatted log string
     */
    virtual void record(std::string formattedLog) = 0;

protected:
    AuditMask mask_;
    // we use thread local log stream to make each thread have its own format buffer.
    static thread_local LogStream logStream_;
};

}  // namespace audit
}  // namespace nebula

/**
 * AUDIT_LOG macro helps create event message object only when that specific event's
 * type passed the filter. It use the same glog's LOG(XXX) like technology to only
 * evaluation the message body when we do need log that message.
 */
#define AUDIT_LOG_TEST(AUDITOR, TYPE) \
    !AUDITOR->filter(TYPE) ? void(0) : nebula::LogStreamVoidify{} & AUDITOR->stream()  // NOLINT
