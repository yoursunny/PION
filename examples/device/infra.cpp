#include "app.hpp"

namespace ndnob_device_app {

#if defined(NDNOB_INFRA_UDP)
static std::unique_ptr<esp8266ndn::UdpTransport> transport;
static constexpr size_t NC_ITEMS = 3;
#elif defined(NDNOB_INFRA_ETHER)
static std::unique_ptr<esp8266ndn::EthernetTransport> transport;
static constexpr size_t NC_ITEMS = 2;
#else
#error "need either NDNOB_INFRA_UDP or NDNOB_INFRA_ETHER"
#endif
static std::unique_ptr<ndnph::Face> face;
static ndnph::StaticRegion<2048> oRegion;
static ndnph::EcPrivateKey pvt;
static ndnph::EcPublicKey pub;
static std::unique_ptr<ndnph::ndncert::client::PossessionChallenge> challenge;
static ndnph::Data oCert;

void
doInfraConnect()
{
  GotoState gotoState;

  char ncBuf[64];
  std::array<const char*, NC_ITEMS> nc;
  {
    auto ncV = getPakeDevice()->getNetworkCredential();
    if (ncV.size() + 1 >= sizeof(ncBuf)) {
      NDNOB_LOG_ERR("NetworkCredential too long");
      return;
    }
    std::copy(ncV.begin(), ncV.end(), ncBuf);
    ncBuf[ncV.size()] = '\0';

    for (size_t i = 0; i < nc.size(); ++i) {
      nc[i] = strtok(i == 0 ? ncBuf : nullptr, "\n");
      if (nc[i] == nullptr) {
        NDNOB_LOG_ERR("NetworkCredential[%zu] missing", i);
        return;
      }
    }
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(nc[0], nc[1]);
  WiFi.waitForConnectResult();
  if (!WiFi.isConnected()) {
    NDNOB_LOG_ERR("WiFi.begin error %d", WiFi.status());
    return;
  }

#if defined(NDNOB_INFRA_UDP)
  transport.reset(new esp8266ndn::UdpTransport);
  IPAddress remote;
  if (!remote.fromString(nc[2])) {
    NDNOB_LOG_ERR("IPAddress.fromString error");
    return;
  }
  if (!transport->beginTunnel(remote)) {
    NDNOB_LOG_ERR("transport.beginTunnel error");
    return;
  }
#elif defined(NDNOB_INFRA_ETHER)
  transport.reset(new esp8266ndn::EthernetTransport);
  if (!transport->begin()) {
    NDNOB_LOG_ERR("transport.begin error");
    return;
  }
#endif

  face.reset(new ndnph::Face(*transport));
  gotoState(State::WaitNdncert);
}

static void
ndncertCallback(void*, ndnph::Data cert)
{
  GotoState gotoState;

  if (!cert) {
    NDNOB_LOG_ERR("ndncert failed");
    return;
  }

  oCert = oRegion.create<ndnph::Data>();
  if (!oCert || !oCert.decodeFrom(cert) || !ndnph::ec::isCertificate(oCert)) {
    NDNOB_LOG_ERR("issued cert error");
    return;
  }
  pvt.setName(oCert.getName());
  gotoState(State::Success);
}

static bool
initNdncert()
{
  auto pakeDevice = getPakeDevice();
  oRegion.reset();
  if (!ndnph::ec::generate(oRegion, pakeDevice->getDeviceName(), pvt, pub)) {
    NDNOB_LOG_ERR("ec::generate error");
    return false;
  }

  challenge.reset(new ndnph::ndncert::client::PossessionChallenge(pakeDevice->getTempCert(),
                                                                  pakeDevice->getTempSigner()));

  ndnph::ndncert::Client::requestCertificate(ndnph::ndncert::Client::Options{
    .face = *face,
    .profile = pakeDevice->getCaProfile(),
    .challenges = { challenge.get() },
    .pub = pub,
    .pvt = pvt,
    .cb = ndncertCallback,
    .ctx = nullptr,
  });
  return true;
}

void
waitNdncert()
{
  face->loop();

  if (challenge == nullptr && !initNdncert()) {
    state = State::Failure;
    return;
  }
}

void
deleteNdncert()
{
  challenge.reset();
}

const ndnph::Data&
getDeviceCert()
{
  return oCert;
}

const ndnph::PrivateKey&
getDeviceSigner()
{
  return pvt;
}

} // namespace ndnob_device_app
