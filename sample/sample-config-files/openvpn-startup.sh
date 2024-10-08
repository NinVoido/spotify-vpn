#!/bin/sh

# A sample spotify startup script
# for Linux.

# spotify config file directory
dir=/etc/spotify

# load the firewall
$dir/firewall.sh

# load TUN/TAP kernel module
modprobe tun

# enable IP forwarding
echo 1 > /proc/sys/net/ipv4/ip_forward

# Invoke spotify for each VPN tunnel
# in daemon mode.  Alternatively,
# you could remove "--daemon" from
# the command line and add "daemon"
# to the config file.
#
# Each tunnel should run on a separate
# UDP port.  Use the "port" option
# to control this.  Like all of
# spotify's options, you can
# specify "--port 8000" on the command
# line or "port 8000" in the config
# file.

spotify --cd $dir --daemon --config vpn1.conf
spotify --cd $dir --daemon --config vpn2.conf
spotify --cd $dir --daemon --config vpn2.conf
