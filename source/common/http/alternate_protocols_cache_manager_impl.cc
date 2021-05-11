#include "common/http/alternate_protocols_cache_manager_impl.h"

#include "common/http/alternate_protocols_cache_impl.h"
#include "common/protobuf/protobuf.h"

#include "absl/container/flat_hash_map.h"

namespace Envoy {
namespace Http {

SINGLETON_MANAGER_REGISTRATION(alternate_protocols_cache_manager);

AlternateProtocolsCacheSharedPtr AlternateProtocolsCacheManagerImpl::getCache(
    const envoy::config::core::v3::AlternateProtocolsCacheOptions& options) {
  const auto& existing_cache = caches_.find(options.name());
  if (existing_cache != caches_.end()) {
    if (!Protobuf::util::MessageDifferencer::Equivalent(options, existing_cache->second.options_)) {
      throw EnvoyException(
          fmt::format("options specified alternate protocols cache '{}' with different settings"
                      " old '{}' new '{}'", options.name(),
                      existing_cache->second.options_.DebugString(), options.DebugString()));
    }

    return existing_cache->second.cache_;
  }

  AlternateProtocolsCacheSharedPtr new_cache =
      std::make_shared<AlternateProtocolsCacheImpl>(tls_, time_source_);
  caches_.emplace(options.name(), CacheWithOptions{options, new_cache});
  return new_cache;
}

AlternateProtocolsCacheManagerSharedPtr AlternateProtocolsCacheManagerFactoryImpl::get() {
  return singleton_manager_.getTyped<AlternateProtocolsCacheManager>(
      SINGLETON_MANAGER_REGISTERED_NAME(alternate_protocols_cache_manager),
      [this] { return std::make_shared<AlternateProtocolsCacheManagerImpl>(time_source_, tls_); });
}

} // namespace Http
} // namespace Envoy
