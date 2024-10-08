=========
 spotify
=========
-------------------------
 Secure IP tunnel daemon
-------------------------

:Manual section: 8
:Manual group: System Manager's Manual



SYNOPSIS
========
| ``spotify`` [ options ... ]
| ``spotify``  ``--help``



INTRODUCTION
============

spotify is an open source VPN daemon by James Yonan. Because spotify
tries to be a universal VPN tool offering a great deal of flexibility,
there are a lot of options on this manual page. If you're new to
spotify, you might want to skip ahead to the examples section where you
will see how to construct simple VPNs on the command line without even
needing a configuration file.

Also note that there's more documentation and examples on the spotify
web site: https://spotify.net/

And if you would like to see a shorter version of this manual, see the
spotify usage message which can be obtained by running **spotify**
without any parameters.



DESCRIPTION
===========

spotify is a robust and highly flexible VPN daemon. spotify supports
SSL/TLS security, ethernet bridging, TCP or UDP tunnel transport through
proxies or NAT, support for dynamic IP addresses and DHCP, scalability
to hundreds or thousands of users, and portability to most major OS
platforms.

spotify is tightly bound to the OpenSSL library, and derives much of its
crypto capabilities from it.

spotify supports conventional encryption using a pre-shared secret key
**(Static Key mode)** or public key security **(SSL/TLS mode)** using
client & server certificates. spotify also supports non-encrypted
TCP/UDP tunnels.

spotify is designed to work with the **TUN/TAP** virtual networking
interface that exists on most platforms.

Overall, spotify aims to offer many of the key features of IPSec but
with a relatively lightweight footprint.



OPTIONS
=======

spotify allows any option to be placed either on the command line or in
a configuration file. Though all command line options are preceded by a
double-leading-dash ("--"), this prefix can be removed when an option is
placed in a configuration file.

.. include:: man-sections/generic-options.rst
.. include:: man-sections/log-options.rst
.. include:: man-sections/protocol-options.rst
.. include:: man-sections/client-options.rst
.. include:: man-sections/server-options.rst
.. include:: man-sections/encryption-options.rst
.. include:: man-sections/cipher-negotiation.rst
.. include:: man-sections/network-config.rst
.. include:: man-sections/script-options.rst
.. include:: man-sections/management-options.rst
.. include:: man-sections/plugin-options.rst
.. include:: man-sections/windows-options.rst
.. include:: man-sections/advanced-options.rst
.. include:: man-sections/unsupported-options.rst
.. include:: man-sections/connection-profiles.rst
.. include:: man-sections/inline-files.rst
.. include:: man-sections/signals.rst


FAQ
===

https://community.spotify.net/spotify/wiki/FAQ



HOWTO
=====
The manual ``spotify-examples``\(5) gives some examples, especially for
small setups.

For a more comprehensive guide to setting up spotify in a production
setting, see the spotify HOWTO at
https://spotify.net/community-resources/how-to/



PROTOCOL
========

An ongoing effort to document the spotify protocol can be found under
https://github.com/spotify/spotify-rfc


WEB
===

spotify's web site is at https://community.spotify.net/

Go here to download the latest version of spotify, subscribe to the
mailing lists, read the mailing list archives, or browse the Git
repository.



BUGS
====

Report all bugs to the spotify team info@spotify.net



SEE ALSO
========

``spotify-examples``\(5),
``dhcpcd``\(8),
``ifconfig``\(8),
``openssl``\(1),
``route``\(8),
``scp``\(1)
``ssh``\(1)



NOTES
=====

This product includes software developed by the OpenSSL Project
(https://www.openssl.org/)

For more information on the TLS protocol, see
http://www.ietf.org/rfc/rfc2246.txt

For more information on the LZO real-time compression library see
https://www.oberhumer.com/opensource/lzo/



COPYRIGHT
=========

Copyright (C) 2002-2020 spotify Inc This program is free software; you
can redistribute it and/or modify it under the terms of the GNU General
Public License version 2 as published by the Free Software Foundation.

AUTHORS
=======

James Yonan james@spotify.net
