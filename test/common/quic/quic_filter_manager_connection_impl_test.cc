#include "source/common/quic/quic_filter_manager_connection_impl.h"

#include "quiche/quic/core/quic_connection.h"

#include "test/mocks/event/mocks.h"
#include "test/mocks/network/mocks.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"


namespace Envoy {
namespace Quic {

class TestQuicFilterManagerConnectionImpl : public QuicFilterManagerConnectionImpl {
public:
  TestQuicFilterManagerConnectionImpl(
      QuicNetworkConnection& connection, const quic::QuicConnectionId& connection_id,
      Event::Dispatcher& dispatcher, uint32_t send_buffer_limit)
      : QuicFilterManagerConnectionImpl(connection, connection_id, dispatcher, send_buffer_limit) {}


  void dumpState(std::ostream& /*os*/, int /*indent_level = 0*/) const override {}
  absl::string_view requestedServerName() const override { return {}; }
  bool hasDataToWrite() override { return false; }
  const quic::QuicConnection* quicConnection() const override { return nullptr; }
  quic::QuicConnection* quicConnection() override { return nullptr; }

  void closeNow() {
    const quic::QuicConnectionCloseFrame frame;
    quic::ConnectionCloseSource source = quic::ConnectionCloseSource::FROM_SELF;
    const quic::ParsedQuicVersion version =  quic::AllSupportedVersions().front();
    onConnectionCloseEvent(frame, source, version);
  }
};

TEST(QuicFilterManagerConnectionImplTest, ConnectionInfoProviderSharedPtr) {
  auto socket = std::make_unique<Network::MockConnectionSocket>();
  EXPECT_CALL(*socket, close());
  QuicNetworkConnection connection(std::move(socket));
  const quic::QuicConnectionId connection_id;
  Event::MockDispatcher dispatcher;
  uint32_t send_buffer_limit = 0;

  TestQuicFilterManagerConnectionImpl impl_(connection, connection_id, dispatcher, send_buffer_limit);
  EXPECT_TRUE(impl_.connectionInfoProviderSharedPtr() != nullptr);
  impl_.closeNow();
  EXPECT_TRUE(impl_.connectionInfoProviderSharedPtr() == nullptr);
}

}  // namespace Quic
}  // namespace Envoy
