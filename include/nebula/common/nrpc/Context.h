// Copyright (c) 2022 vesoft inc. All rights reserved.

#pragma once

#include <memory>

#include <folly/io/IOBuf.h>

#include "nebula/common/nrpc/Address.h"
#include "nebula/common/nrpc/List.h"

struct ev_timer;
namespace nebula::nrpc {

struct Connection;

// clang-format off
class ServerContext final {
public:
    using BufferPtr = std::unique_ptr<folly::IOBuf>;
    ServerContext(Connection *c, BufferPtr req, uint64_t chid, uint32_t timeout) {
        conn_ = c;
        req_ = std::move(req);
        chid_ = chid;
        timeout_ = timeout;
    }

    ~ServerContext() {}

    /**
     * Get the timed out duration of the client in milliseconds.
     */
    auto timeout() const {
        return timeout_;
    }

    auto conn() const {
        return conn_;
    }

    /**
     * Get the requesting buffer
     */
    folly::IOBuf* req() const {
        return req_.get();
    }

    /**
     * Move out the requesting buffer
     */
    BufferPtr moveReq() {
        return std::move(req_);
    }

    /**
     * Send the responding buffer and finish this rpc session.
     *
     * @resp    : Could be either a single or chained buffer.
     * @more    : Whether there are more responses upcoming.
     *
     * NOTE
     *      When `more' is `false', this context cannot be accessed anymore after it returns.
     *      When `more' is `true', It's usually for streaming or pub/sub use.
     *      This method can be invoked from multiple thread. But the caller should coordinate to
     *      guarantee that `finish(resp, false)` appears only once and is the last invoke.
     */
    void finish(std::unique_ptr<folly::IOBuf> resp, bool more = false);
    void finish(std::string_view resp, bool more = false);

    /**
     * Tell if this context is valid.
     * If not, the caller should invoke `drop' upon it.
     * This is necessary when the context is for streaming or pub/sub use.
     */
    auto valid() const {
        return valid_.load();
    }

    /**
     * Drop this context
     * This method can be invoked only once.
     */
    void drop();

private:
    friend class IOThread;

    auto more() const {
        return more_.load();
    }

    auto chid() const {
        return chid_;
    }

    auto* resp() const {
        return resp_.get();
    }

    std::unique_ptr<folly::IOBuf> moveResp() {
        return std::move(resp_);
    }

    ServerContext* clone();

    void setInvalid() {
        valid_ = false;
    }

public:
    ListNode                                node;

private:
    Connection                             *conn_;
    std::atomic<bool>                       more_{false};
    std::atomic<bool>                       valid_{true};
    uint32_t                                timeout_{0};
    uint64_t                                chid_{0};
    std::unique_ptr<folly::IOBuf>           req_;
    std::unique_ptr<folly::IOBuf>           resp_;
};


class ClientContext final{
public:
    using BufferPtr = std::unique_ptr<folly::IOBuf>;
    using Callback = folly::Function<void(ClientContext*)>;
    ClientContext(const Address &addr, BufferPtr req, Callback cb, int timeout);
    ~ClientContext();

    /**
     * Move out the responding buffer
     */
    BufferPtr moveResp() {
        return std::move(resp_);
    }

    folly::IOBuf* resp() const {
        return resp_.get();
    }

    /**
     * Get the status of this request context
     */
    const Status& status() const {
        return status_;
    }

    uint32_t timeout() const {
        return timeout_;
    }

    const Address& addr() const {
        return addr_;
    }

    bool more() const {
        return more_;
    }

    void abort() {
        aborted_ = true;
    }

private:
    friend class IOThread;
    void finish(BufferPtr resp) {
        resp_ = std::move(resp);
        cb_(this);
    }

    void finish(Status status) {
        status_ = std::move(status);
        cb_(this);
    }

    BufferPtr moveReq() {
        return std::move(req_);
    }

    void setConn(Connection *c) {
        conn_ = c;
    }

    Connection* conn() const {
        return conn_;
    }

    ev_timer* timer() {
        return timer_;
    }

    void setchid(uint64_t chid) {
        chid_ = chid;
    }

    uint32_t chid() const {
        return chid_;
    }

    void more(bool more) {
        more_ = more;
    }

    bool aborted() const {
        return aborted_;
    }

public:
    ListNode                    node;

private:
    Status                      status_;
    Address                     addr_;
    uint64_t                    chid_{0};
    bool                        more_{false};
    bool                        aborted_{false};
    int                         timeout_{0};
    ev_timer                   *timer_{nullptr};
    Connection                 *conn_{nullptr};
    BufferPtr                   req_;
    BufferPtr                   resp_;
    Callback                    cb_;
};


}   // namespace nebula::nrpc

