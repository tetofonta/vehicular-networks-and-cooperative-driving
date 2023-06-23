import packetHeader;

packet PlatoonCreateRequestACK extends PacketHeader{
    int platoonId;
    int leaderId;
    bool accepted;
    double coord;
}