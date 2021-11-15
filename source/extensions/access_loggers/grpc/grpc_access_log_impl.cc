#include "source/extensions/access_loggers/grpc/grpc_access_log_impl.h"

#include "envoy/data/accesslog/v3/accesslog.pb.h"
#include "envoy/extensions/access_loggers/grpc/v3/als.pb.h"
#include "envoy/grpc/async_client_manager.h"
#include "envoy/local_info/local_info.h"

#include "source/common/config/utility.h"
#include "source/common/grpc/typed_async_client.h"

const char GRPC_LOG_STATS_PREFIX[] = "access_logs.grpc_access_log.";

namespace Envoy {
namespace Extensions {
namespace AccessLoggers {
namespace GrpcCommon {

GrpcAccessLoggerImpl::GrpcAccessLoggerImpl(
    const Grpc::RawAsyncClientSharedPtr& client,
    const envoy::extensions::access_loggers::grpc::v3::CommonGrpcAccessLogConfig& config,
    std::chrono::milliseconds buffer_flush_interval_msec, uint64_t max_buffer_size_bytes,
    Event::Dispatcher& dispatcher, const LocalInfo::LocalInfo& local_info, Stats::Scope& scope)
    : GrpcAccessLogger(std::move(client), buffer_flush_interval_msec, max_buffer_size_bytes,
                       dispatcher, scope, GRPC_LOG_STATS_PREFIX,
                       *Protobuf::DescriptorPool::generated_pool()->FindMethodByName(
                           "envoy.service.accesslog.v3.AccessLogService.StreamAccessLogs"),
                       config.grpc_stream_retry_policy()),
      log_name_(config.log_name()), local_info_(local_info) {}

void GrpcAccessLoggerImpl::addEntry(envoy::data::accesslog::v3::HTTPAccessLogEntry&& entry) {
  message_.mutable_http_logs()->mutable_log_entry()->Add(std::move(entry));
}

void GrpcAccessLoggerImpl::addEntry(envoy::data::accesslog::v3::TCPAccessLogEntry&& entry) {
  message_.mutable_tcp_logs()->mutable_log_entry()->Add(std::move(entry));
}

bool GrpcAccessLoggerImpl::isEmpty() {
  return !message_.has_http_logs() && !message_.has_tcp_logs();
}

void GrpcAccessLoggerImpl::initMessage() {
  auto* identifier = message_.mutable_identifier();
  *identifier->mutable_node() = local_info_.node();
  identifier->set_log_name(log_name_);
}

GrpcAccessLoggerCacheImpl::GrpcAccessLoggerCacheImpl(Grpc::AsyncClientManager& async_client_manager,
                                                     Stats::Scope& scope,
                                                     ThreadLocal::SlotAllocator& tls,
                                                     const LocalInfo::LocalInfo& local_info)
    : GrpcAccessLoggerCache(async_client_manager, scope, tls), local_info_(local_info) {}

GrpcAccessLoggerImpl::SharedPtr GrpcAccessLoggerCacheImpl::createLogger(
    const envoy::extensions::access_loggers::grpc::v3::CommonGrpcAccessLogConfig& config,
    const Grpc::RawAsyncClientSharedPtr& client,
    std::chrono::milliseconds buffer_flush_interval_msec, uint64_t max_buffer_size_bytes,
    Event::Dispatcher& dispatcher) {
  return std::make_shared<GrpcAccessLoggerImpl>(client, config, buffer_flush_interval_msec,
                                                max_buffer_size_bytes, dispatcher, local_info_,
                                                scope_);
}

} // namespace GrpcCommon
} // namespace AccessLoggers
} // namespace Extensions
} // namespace Envoy
