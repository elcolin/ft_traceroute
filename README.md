
# FT_TRACEROUTE
Find the path to your destination (ip or domain name), trace your route and check its round trip time.
Don't forget to make !
```bash
make
```
Then
```bash
sudo ./ft_traceroute 8.8.8.8
```
or
```bash
sudo ./ft_traceroute google.com
```

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
* Add id verification to avoid multiple instances of the program
* Add correct timeout for appropriate packet -> smarter based on start and current time
* only reset part of iphdr instead of the whole structure
* make display incremental
