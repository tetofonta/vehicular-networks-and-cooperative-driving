import packetHeader;

packet PlatoonAdvertiseBeacon extends PacketHeader{
    double platoon_speed;
    unsigned int lane;

    int platoon_id;

    //additional informations such as destination, number of members ecc..
}