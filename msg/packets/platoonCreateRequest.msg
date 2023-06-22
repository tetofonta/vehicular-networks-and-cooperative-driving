import packetHeader;

packet PlatoonCreateRequest extends PacketHeader{
    int platoonId;
    int leaderId;

    double coord;
}