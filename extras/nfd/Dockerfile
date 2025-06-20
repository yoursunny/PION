FROM debian:bookworm
SHELL ["/bin/bash", "-c"]

RUN --mount=type=cache,id=bookworm-apt-cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,id=bookworm-apt-cache,target=/var/lib/apt,sharing=locked \
<<EOF
  set -euxo pipefail
  rm -f /etc/apt/apt.conf.d/docker-clean
  echo 'Binary::apt::APT::Keep-Downloaded-Packages "true";' >/etc/apt/apt.conf.d/keep-cache
  apt-get update
  apt-get install -y --no-install-recommends ca-certificates jq
  echo "deb [arch=arm64 trusted=yes] https://nfd-nightly-apt.ndn.today/debian bookworm main" >/etc/apt/sources.list.d/nfd-nightly.list
  apt-get update
  apt-get install -y --no-install-recommends infoedit ndnpeek ndnping ndnsec nfd
EOF

COPY start.sh /

RUN <<EOF
  chmod +x /start.sh
  setcap -r /usr/bin/nfd || true
  echo 'transport=unix:///run/ndn/nfd.sock' >/etc/ndn/client.conf
EOF

VOLUME /var/lib/ndn/nfd /run/ndn
EXPOSE 6363/udp
CMD ["/start.sh"]
