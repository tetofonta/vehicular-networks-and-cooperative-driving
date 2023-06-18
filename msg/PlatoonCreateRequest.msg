import PlatoonUnicast;

packet PlatoonCreateRequest extends PlatoonUnicast{
    double coordinate;
    double platooning_speed_min;
    double platooning_speed_max;
    int platoon_id;
    int leader_id;
    unsigned int lane;
}