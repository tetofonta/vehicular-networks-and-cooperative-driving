import packetHeader;

packet PlatoonAdvertiseBeacon extends PacketHeader{
    double platoon_speed;
    unsigned int lane;
    uint8_t platoon_size;
    int platoon_id;
    double coords;
    bool accepting;

    //additional informations such as destination, number of members ecc..
}