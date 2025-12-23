
# FT_TRACEROUTE

## Packet

                  0      7 8     15 16    23 24    31
                 +--------+--------+--------+--------+
                 |     Source      |   Destination   |
                 |      Port       |      Port       |
                 +--------+--------+--------+--------+
                 |                 |                 |
                 |     Length      |    Checksum     |
                 +--------+--------+--------+--------+
                 |
                 |          data octets ...
                 +---------------- ...

                      User Datagram Header Format

## Pseudo Header

                  0      7 8     15 16    23 24    31
                 +--------+--------+--------+--------+
                 |          source address           |
                 +--------+--------+--------+--------+
                 |        destination address        |
                 +--------+--------+--------+--------+
                 |  zero  |protocol|   UDP length    |
                 +--------+--------+--------+--------+

## Checksum

Checksum is the 16-bit one's complement of the one's complement sum of a
pseudo header of information from the IP header, the UDP header, and the
data,  padded  with zero octets  at the end (if  necessary)  to  make  a
multiple of two octets.

## Notes
* Move the 3 different loops in self contained function
* Make sure all structure are set to zero before sending new infos
* 