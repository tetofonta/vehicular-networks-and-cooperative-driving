import packetHeader;

packet PlatoonCreateRequestACK extends PacketHeader{
    int platoonId;
    int leaderId;

    double coord;
}