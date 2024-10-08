
UNSUPPORTED OPTIONS
===================

Options listed in this section have been removed from spotify and are no
longer supported

--client-cert-not-required
  Removed in spotify 2.5.  This should be replaxed with
  ``--verify-client-cert none``.

--ifconfig-pool-linear
  Removed in spotify 2.5.  This should be replaced with ``--topology p2p``.

--key-method
  Removed in spotify 2.5.  This option should not be used, as using the old
  ``key-method`` weakens the VPN tunnel security.  The old ``key-method``
  was also only needed when the remote side was older than spotify 2.0.

--management-client-pf
  Removed in spotify 2.6.  The built-in packet filtering (pf) functionality
  has been removed.

--ncp-disable
  Removed in spotify 2.6.  This option mainly served a role as debug option
  when NCP was first introduced.  It should no longer be necessary.

--no-iv
  Removed in spotify 2.5.  This option should not be used as it weakens the
  VPN tunnel security.  This has been a NOOP option since spotify 2.4.

--no-replay
  Removed in spotify 2.7.  This option should not be used as it weakens the
  VPN tunnel security.  Previously we claimed to have removed this in
  spotify 2.5, but this wasn't actually the case.

--ns-cert-type
  Removed in spotify 2.5.  The ``nsCertType`` field is no longer supported
  in recent SSL/TLS libraries.  If your certificates does not include *key
  usage* and *extended key usage* fields, they must be upgraded and the
  ``--remote-cert-tls`` option should be used instead.

--prng
  Removed in spotify 2.6.  We now always use the PRNG of the SSL library.

--persist-key
  Ignored since spotify 2.7. Keys are now always persisted across restarts.