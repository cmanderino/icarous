/*
 * ardupilot_process_messages.c
 *
 *
 */
#define EXTERN extern

#include <paramdef.h>
#include "ardupilot.h"
#include "UtilFunctions.h"

int GetMAVLinkMsgFromAP(){
    int n = readPort(&appdataInt.ap);
    mavlink_message_t message;
    mavlink_status_t status;
    uint8_t msgReceived = 0;
    for(int i=0;i<n;i++){
        uint8_t cp = appdataInt.ap.recvbuffer[i];
        msgReceived = mavlink_parse_char(MAVLINK_COMM_0, cp, &message, &status);
        if(msgReceived){
            // Send SB message if necessary
            ProcessAPMessage(message);
        }
    }
    return n;
}

void apSendHeartbeat(){
    mavlink_message_t hbeat;
    mavlink_msg_heartbeat_pack(sysid_ic,compid_ic,&hbeat,MAV_TYPE_ONBOARD_CONTROLLER,MAV_AUTOPILOT_INVALID,0,0,0);
    writeMavlinkData(&appdataInt.ap,&hbeat);
    if(appdataInt.foundUAV == 0){
        mavlink_message_t msg;
        mavlink_msg_request_data_stream_pack(sysid_ic,compid_ic,&msg,1,0,MAV_DATA_STREAM_ALL,4,1);
        writeMavlinkData(&appdataInt.ap,&msg);
    }    
}

void ProcessAPMessage(mavlink_message_t message) {

    // Ignore message produced by self
    if(message.sysid == sysid_ic && message.compid == compid_ic){
        return;
    }

    switch (message.msgid) {

        case MAVLINK_MSG_ID_HEARTBEAT:
        {
            mavlink_heartbeat_t msg;
            mavlink_msg_heartbeat_decode(&message,&msg);
            if (appdataInt.foundUAV == 0) {
                appdataInt.foundUAV = 1;
                mavlink_message_t msg;
                mavlink_msg_request_data_stream_pack(sysid_ic,compid_ic,&msg,sysid_ap,compid_ap,MAV_DATA_STREAM_ALL,4,1);
                writeMavlinkData(&appdataInt.ap,&msg);
                CFE_EVS_SendEvent(ARDUPILOT_CONNECTED_TO_AP_EID,CFE_EVS_INFORMATION,"Connection to autopilot established");

                mavlink_message_t msg_tune;
                mavlink_msg_play_tune_pack(sysid_ic,compid_ic,&msg_tune,sysid_ap,compid_ap,"MFT90O2C16C16C16F8.A8C16C16C");
                writeMavlinkData(&appdataInt.ap,&msg_tune);
            }
            appdataInt.currentAPMode = msg.custom_mode;
            break;
        }


        case MAVLINK_MSG_ID_MISSION_REQUEST_INT:
        {
            //printf("MAVLINK_MSG_ID_MISSION_REQUEST_INT\n");
            mavlink_mission_request_int_t msg;
            mavlink_msg_mission_request_int_decode(&message, &msg);


            mavlink_message_t msgMissionItemInt;
            mavlink_msg_mission_item_int_pack(sysid_ic,compid_ic,&msgMissionItemInt,sysid_ap,compid_ap,msg.seq,appdataInt.UplinkMissionItems[msg.seq].frame,
                                              appdataInt.UplinkMissionItems[msg.seq].command,
                                              appdataInt.UplinkMissionItems[msg.seq].current,
                                              appdataInt.UplinkMissionItems[msg.seq].autocontinue,
                                              appdataInt.UplinkMissionItems[msg.seq].param1,
                                              appdataInt.UplinkMissionItems[msg.seq].param2,
                                              appdataInt.UplinkMissionItems[msg.seq].param3,
                                              appdataInt.UplinkMissionItems[msg.seq].param4,
                                              appdataInt.UplinkMissionItems[msg.seq].x,
                                              appdataInt.UplinkMissionItems[msg.seq].y,
                                              appdataInt.UplinkMissionItems[msg.seq].z,
                                              appdataInt.UplinkMissionItems[msg.seq].mission_type);

            writeMavlinkData(&appdataInt.ap,&msgMissionItemInt);
            break;
        }

        case MAVLINK_MSG_ID_MISSION_REQUEST_LIST:
        {
            //printf("MAVLINK_MSG_ID_MISSION_REQUEST_LIST %d %d\n",message.sysid,message.compid);
            mavlink_mission_request_list_t msg;
            mavlink_msg_mission_request_list_decode(&message, &msg);
            int count = 0;

            if(!(msg.target_system == sysid_ic && msg.target_component == compid_ic)){
                break;
            }

            if (msg.mission_type == MAV_MISSION_TYPE_MISSION)
            {
                count = appdataInt.numWaypoints;
            }else if(msg.mission_type == MAV_MISSION_TYPE_FENCE){
                for(int i=0;i<appdataInt.numGeofences;++i){
                    count += appdataInt.fenceVertices[i];
                }
            }else if(msg.mission_type == MAV_MISSION_TYPE_RALLY){
                count = appdataInt.trajectory.num_waypoints;
            }

            if(count > 0){
                mavlink_message_t msgCount;
                mavlink_msg_mission_count_pack(sysid_ic, compid_ic, &msgCount, message.sysid, message.compid, count, msg.mission_type);
                writeMavlinkData(&appdataInt.ap, &msgCount);
            }

            break;
        }

        case MAVLINK_MSG_ID_MISSION_REQUEST:{
            //printf("MAVLINK_MSG_ID_MISSION_REQUEST %d %d\n",message.sysid,message.compid);

            mavlink_mission_request_t msg;
            mavlink_msg_mission_request_decode(&message, &msg);

            if(!(msg.target_system == sysid_ic && msg.target_component == compid_ic)){
                break;
            }

            mavlink_message_t msgMissionItem;
            if(msg.mission_type == MAV_MISSION_TYPE_MISSION){
                mavlink_msg_mission_item_pack(sysid_ic,compid_ic,&msgMissionItem,message.sysid,message.compid,msg.seq,appdataInt.DownlinkMissionItems[msg.seq].frame,
                                                                               appdataInt.DownlinkMissionItems[msg.seq].command,
                                                                               appdataInt.DownlinkMissionItems[msg.seq].current,
                                                                               appdataInt.DownlinkMissionItems[msg.seq].autocontinue,
                                                                               appdataInt.DownlinkMissionItems[msg.seq].param1,
                                                                               appdataInt.DownlinkMissionItems[msg.seq].param2,
                                                                               appdataInt.DownlinkMissionItems[msg.seq].param3,
                                                                               appdataInt.DownlinkMissionItems[msg.seq].param4,
                                                                               appdataInt.DownlinkMissionItems[msg.seq].x,
                                                                               appdataInt.DownlinkMissionItems[msg.seq].y,
                                                                               appdataInt.DownlinkMissionItems[msg.seq].z,
                                                                               appdataInt.DownlinkMissionItems[msg.seq].mission_type);
            }else if(msg.mission_type == MAV_MISSION_TYPE_FENCE){
                int reqItem = msg.seq;
                int index = 0;
                int vertexTotal = 0;
                for(int i=0;i<appdataInt.numGeofences;++i){
                    vertexTotal += appdataInt.fenceVertices[i];
                    if (msg.seq < vertexTotal){
                        if(i > 0){
                            index  = appdataInt.fenceVertices[i] - (vertexTotal - reqItem);
                        }
                        else{
                            index  = reqItem;
                        }
                       
                        int _type = appdataInt.gfData[i].type;
                        double _ceiling = appdataInt.gfData[i].ceiling;
                        double _floor = appdataInt.gfData[i].floor;
                        double _numVertices = appdataInt.fenceVertices[i];

                        mavlink_msg_mission_item_pack(sysid_ic,compid_ic,&msgMissionItem,sysid_gs,compid_gs,i,
                                                          _type,
                                                          0,
                                                          0,
                                                          0,
                                                        _numVertices,
                                                        index,_floor,_ceiling,
                                                        appdataInt.gfData[i].vertices[index][0],
                                                        appdataInt.gfData[i].vertices[index][1],
                                                        0,MAV_MISSION_TYPE_FENCE);

                        break;
                    }
                }
            }else if(msg.mission_type == MAV_MISSION_TYPE_RALLY){
                ap_stopTimer(&appdataInt.tjtimer);
                mavlink_msg_mission_item_pack(sysid_ic,compid_ic,&msgMissionItem,sysid_gs,compid_gs,msg.seq,MAV_FRAME_GLOBAL,0,0,0,0,0,0,0,
                                              appdataInt.trajectory.waypoints[msg.seq].latitude,
                                              appdataInt.trajectory.waypoints[msg.seq].longitude,
                                              appdataInt.trajectory.waypoints[msg.seq].altitude,MAV_MISSION_TYPE_RALLY);
            }

            writeMavlinkData(&appdataInt.ap,&msgMissionItem);
            break;

        }


        case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:
        {
            mavlink_global_position_int_t globalPositionInt;
            mavlink_msg_global_position_int_decode(&message,&globalPositionInt);
            position.aircraft_id = CFE_PSP_GetSpacecraftId();
            position.time_boot  = (double)globalPositionInt.time_boot_ms/1E3;
            position.latitude  = (double)globalPositionInt.lat/1E7;
            position.longitude = (double)globalPositionInt.lon/1E7;
            position.altitude_abs  = (double)globalPositionInt.alt/1E3;
            position.altitude_rel  = (double)globalPositionInt.relative_alt/1E3;
            position.vn = (double)globalPositionInt.vx/100;
            position.ve = (double)globalPositionInt.vy/100;
            position.vd = (double)globalPositionInt.vz/100;
            position.hdg = (double)globalPositionInt.hdg/100;

            CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &position);
            CFE_SB_SendMsg((CFE_SB_Msg_t *) &position);
            break;
        }

        case MAVLINK_MSG_ID_ATTITUDE:{
            mavlink_attitude_t apAttitude;
            mavlink_msg_attitude_decode(&message,&apAttitude);
            attitude.pitch = apAttitude.pitch*180/M_PI;
            attitude.roll  = apAttitude.roll*180/M_PI;
            attitude.yaw = apAttitude.yaw*180/M_PI;
            attitude.pitchspeed = apAttitude.pitchspeed*180/M_PI;
            attitude.rollspeed = apAttitude.rollspeed*180/M_PI;
            attitude.yawspeed = apAttitude.yawspeed*180/M_PI;
            attitude.time_boot = apAttitude.time_boot_ms;

            if(attitude.yaw < 0){
                attitude.yaw = 360 + attitude.yaw;
            }

            SendSBMsg(attitude);
            break;
        }

        case MAVLINK_MSG_ID_BATTERY_STATUS:{
            mavlink_battery_status_t apBatteryStatus;
            mavlink_msg_battery_status_decode(&message,&apBatteryStatus);
            battery_status.id = apBatteryStatus.id;
            battery_status.battery_function = apBatteryStatus.battery_function;
            battery_status.type = apBatteryStatus.type;
            battery_status.temperature = apBatteryStatus.temperature;
            memcpy(battery_status.voltages, apBatteryStatus.voltages, 10*sizeof(uint16_t));
            battery_status.current_battery = apBatteryStatus.current_battery;
            battery_status.current_consumed = apBatteryStatus.current_consumed;
            battery_status.energy_consumed = apBatteryStatus.energy_consumed;
            battery_status.battery_remaining = apBatteryStatus.battery_remaining;

            SendSBMsg(battery_status)

            break;
        }

        case MAVLINK_MSG_ID_COMMAND_ACK:
        {
            mavlink_command_ack_t msg;
            mavlink_msg_command_ack_decode(&message, &msg);
            uint8_t send = 0;
            switch (msg.command) {
                case MAV_CMD_COMPONENT_ARM_DISARM:
                {
                    ack.name = _ARM_;
                    ack.result = msg.result;
                    send = 1;
                    break;
                }
                case MAV_CMD_NAV_TAKEOFF:
                {
                    ack.name = _TAKEOFF_;
                    ack.result = msg.result;
                    send = 1;
                    break;
                }
            }

            if (send) {
                CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &ack);
                CFE_SB_SendMsg((CFE_SB_Msg_t *) &ack);
            }
            break;
        }


        case MAVLINK_MSG_ID_MISSION_ITEM_REACHED:
        {
            //printf("AP: MAVLINK_MSG_ID_MISSION_ITEM_REACHED\n");
            mavlink_mission_item_reached_t msg;
            mavlink_msg_mission_item_reached_decode(&message, &msg);

            // Find the corresponding index of the flightplan_t data
            int i;
            bool avail = false;
            for(i=0;i<fpdata.num_waypoints;++i)	{
                if (appdataInt.waypoint_index[i] == msg.seq) {
                    avail = true;
                    break;
                }
            }

            wpreached.reachedwaypoint = (uint8_t)(i);
            wpreached.feedback = true;
            if(avail) {
                CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &wpreached);
                CFE_SB_SendMsg((CFE_SB_Msg_t *) &wpreached);
                //OS_printf("waypoint reached = %d / %d\n", i, msg.seq);
            }
            break;
        }

        case MAVLINK_MSG_ID_MISSION_ACK:{
            if(appdataInt.startWPUplink){
                mavlink_mission_ack_t msg;
                mavlink_msg_mission_ack_decode(&message,&msg);
                if (msg.type == MAV_RESULT_ACCEPTED)
                    appdataInt.startWPUplink = false;
            }
            break;
        }

        case MAVLINK_MSG_ID_MISSION_COUNT:{
            //OS_printf("MAVLINK_MSG_ID_MISSION_COUNT %d %d\n",message.sysid,message.compid);
            if(message.sysid == 255){
                break;
            }

            mavlink_mission_count_t missionCount;
            mavlink_msg_mission_count_decode(&message, &missionCount);

            if(!(missionCount.target_system == sysid_ic && missionCount.target_component == compid_ic)){
                break;
            }
            
            appdataInt.numDownlinkWaypoints = missionCount.count;
            mavlink_message_t msg;
            mavlink_msg_mission_request_pack(sysid_ic, compid_ic, &msg, sysid_ap, compid_ap, appdataInt.downlinkRequestIndex, MAV_MISSION_TYPE_MISSION);
            writeMavlinkData(&(appdataInt.ap), &msg);
            break;
        }

        case MAVLINK_MSG_ID_MISSION_ITEM:
        {
            //OS_printf("MAVLINK_MSG_ID_MISSION_ITEM %d %d\n",message.sysid,message.compid);
            if(message.sysid == 255){
                break;
            }
            mavlink_mission_item_t missionItem;
            mavlink_msg_mission_item_decode(&message, &missionItem);

            if(!(missionItem.target_system == sysid_ic && missionItem.target_component == compid_ic)){
                break;
            }

            memcpy(appdataInt.DownlinkMissionItems + missionItem.seq, &missionItem, sizeof(mavlink_mission_item_t));

            if (missionItem.seq == appdataInt.numDownlinkWaypoints - 1)
            {

                mavlink_message_t ack;
                mavlink_msg_mission_ack_pack(sysid_ic, compid_ic, &ack, sysid_ap, compid_ap, MAV_MISSION_ACCEPTED, MAV_MISSION_TYPE_MISSION);
                appdataInt.startWPDownlink = false;

                flightplan_t fp;

                CFE_SB_InitMsg(&fp, DOWNLINK_FLIGHTPLAN_MID, sizeof(flightplan_t), FALSE);
                apConvertMissionItemsToPlan(appdataInt.numDownlinkWaypoints, appdataInt.DownlinkMissionItems, &fp);
                SendSBMsg(fp);

                //OS_printf("Received downlink flightplan from pixhawk\n");
                // Send the downlinked flightplan to other apps
                CFE_SB_InitMsg(&fp, ICAROUS_FLIGHTPLAN_MID, sizeof(flightplan_t), FALSE);
                SendSBMsg(fp);

                appdataInt.numWaypoints = appdataInt.numDownlinkWaypoints;
            }
            else
            {
                mavlink_message_t request;
                mavlink_msg_mission_request_pack(sysid_ic, compid_ic, &request, sysid_ap, compid_ap, (uint16_t)(missionItem.seq + 1), MAV_MISSION_TYPE_MISSION);
                writeMavlinkData(&appdataInt.ap, &request);
                //OS_printf("Requesting %d waypoint \n",missionItem.seq + 1);
            }
            break;
        }

        case MAVLINK_MSG_ID_ADSB_VEHICLE:
        {

            mavlink_adsb_vehicle_t msg;
            mavlink_msg_adsb_vehicle_decode(&message,&msg);

            traffic.index = msg.ICAO_address;
            traffic.type = _TRAFFIC_ADSB_;
            traffic.latitude = msg.lat/1.0E7;
            traffic.longitude = msg.lon/1.0E7;
            traffic.altitude = msg.altitude/1.0E3;

            double positionA[3] = {position.latitude,position.longitude,position.altitude_rel};
            double positionB[3] = {traffic.latitude,traffic.longitude,traffic.altitude};
            double dist = ComputeDistance(positionA,positionB);

            if(dist <= 2000 && (appdataInt.currentAPMode == GUIDED || appdataInt.currentAPMode == AUTO) ) {
                double track = msg.heading / 1.0E2;
                double groundspeed = msg.hor_velocity / 1.0E2;
                double verticalspeed = msg.ver_velocity / 1.0E2;

                double vn = groundspeed * cos(track * M_PI / 180);
                double ve = groundspeed * sin(track * M_PI / 180);
                double vu = verticalspeed;
                traffic.vn = vn;
                traffic.ve = ve;
                traffic.vd = vu;

                CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &traffic);
                CFE_SB_SendMsg((CFE_SB_Msg_t *) &traffic);
            }
            break;

        }

        case MAVLINK_MSG_ID_VFR_HUD:{
            mavlink_vfr_hud_t msg;
            mavlink_msg_vfr_hud_decode(&message,&msg);

            vfrhud.alt = msg.alt;
            vfrhud.heading = msg.heading;
            vfrhud.airspeed = msg.airspeed;
            vfrhud.groundspeed = msg.groundspeed;
            vfrhud.climb = msg.climb;
            vfrhud.throttle = msg.throttle;
            vfrhud.modeAP = appdataInt.currentAPMode;
            vfrhud.modeIcarous = appdataInt.icarousMode;
            SendSBMsg(vfrhud);
            break;
        }

        case MAVLINK_MSG_ID_GPS_STATUS:{
            mavlink_gps_status_t msg;
            mavlink_msg_gps_status_decode(&message,&msg);
            position.numSats = msg.satellites_visible;
            break;
        }

        case MAVLINK_MSG_ID_GPS_RAW_INT:{
            mavlink_gps_raw_int_t msg;
            mavlink_msg_gps_raw_int_decode(&message,&msg);
            position.hdop = msg.eph;
            position.vdop = msg.epv;
            break;
        }

        case MAVLINK_MSG_ID_SYSTEM_TIME:{
            mavlink_system_time_t msg;
            mavlink_msg_system_time_decode(&message,&msg);
            position.time_gps = msg.time_unix_usec/1E6;
            break;
        }


        case MAVLINK_MSG_ID_MISSION_CURRENT:{
            mavlink_mission_current_t msg;
            mavlink_msg_mission_current_decode(&message,&msg);
            vfrhud.waypointCurrent = msg.seq;
            break;
        }

        case MAVLINK_MSG_ID_HOME_POSITION:{
            //OS_printf("Received home position\n");
            mavlink_home_position_t msg;
            mavlink_msg_home_position_decode(&message,&msg);

            appdataInt.home_latitude = msg.latitude;
            appdataInt.home_longitude = msg.longitude;
            appdataInt.home_altitude = msg.altitude;
            break;
        }

        case MAVLINK_MSG_ID_RC_CHANNELS:{
            mavlink_rc_channels_t msg;
            mavlink_msg_rc_channels_decode(&message,&msg);

            rc_channels.time_boot_ms = msg.time_boot_ms;
            rc_channels.chancount = msg.chancount;
            rc_channels.chan[1] = msg.chan1_raw;
            rc_channels.chan[2] = msg.chan2_raw;
            rc_channels.chan[3] = msg.chan3_raw;
            rc_channels.chan[4] = msg.chan4_raw;
            rc_channels.chan[5] = msg.chan5_raw;
            rc_channels.chan[6] = msg.chan6_raw;
            rc_channels.chan[7] = msg.chan7_raw;
            rc_channels.chan[8] = msg.chan8_raw;
            rc_channels.chan[9] = msg.chan9_raw;
            rc_channels.chan[10] = msg.chan10_raw;
            rc_channels.chan[11] = msg.chan11_raw;
            rc_channels.chan[12] = msg.chan12_raw;
            rc_channels.chan[13]= msg.chan13_raw;
            rc_channels.chan[14]= msg.chan14_raw;
            rc_channels.chan[15]= msg.chan15_raw;
            rc_channels.chan[16]= msg.chan16_raw;
            rc_channels.chan[17]= msg.chan17_raw;
            rc_channels.chan[18]= msg.chan18_raw;

            SendSBMsg(rc_channels);

            uint8_t ichan = appdataInt.icRcChannel;
            if (ichan > 0){
            uint8_t startlow  = appdataInt.pwmStart - 300;
            uint8_t starthigh = appdataInt.pwmStart + 300;
            uint8_t resetlow = appdataInt.pwmReset - 300;
            uint8_t resethigh = appdataInt.pwmReset + 300;
            if(rc_channels.chan[ichan] >= startlow && rc_channels.chan[ichan] <= starthigh && appdataInt.startMission == false){
                // (start ICAROUS)
                appdataInt.startMission = true;
                appdataInt.restartMission = false;
                if (appdataInt.numWaypoints > 1) {

                    argsCmd_t startMission;
                    CFE_SB_InitMsg(&startMission,ICAROUS_STARTMISSION_MID,sizeof(argsCmd_t),TRUE);
                    startMission.param1 = 1;
                    SendSBMsg(startMission);

                    status_t statusMsg;
                    CFE_SB_InitMsg(&statusMsg,ICAROUS_STATUS_MID,sizeof(status_t),TRUE);
                    statusMsg.severity = SEVERITY_INFO;
                    memset(statusMsg.buffer,0,sizeof(statusMsg.buffer));
                    memcpy(statusMsg.buffer,"IC: RC T/X START",16);
                    SendSBMsg(statusMsg);
                }else{
                    status_t statusMsg;
                    CFE_SB_InitMsg(&statusMsg,ICAROUS_STATUS_MID,sizeof(status_t),TRUE);
                    statusMsg.severity = SEVERITY_INFO;
                    memset(statusMsg.buffer,0,sizeof(statusMsg.buffer));
                    memcpy(statusMsg.buffer,"IC: No flight plan loaded",25);
                    SendSBMsg(statusMsg);
                }
            }else if(rc_channels.chan[ichan] > resetlow && rc_channels.chan[ichan] <= resethigh && appdataInt.restartMission == false){
                //RESET
                argsCmd_t resetIcarous;
                CFE_SB_InitMsg(&resetIcarous,ICAROUS_RESET_MID,sizeof(argsCmd_t),TRUE);
                SendSBMsg(resetIcarous);
                appdataInt.startMission = false;
                appdataInt.restartMission = true;
                apInterface_PublishParams();
                appdataInt.startWPDownlink = true;
				mavlink_message_t msg;
				mavlink_msg_mission_request_list_pack(sysid_ic,compid_ic,&msg,sysid_ap,compid_ap,MAV_MISSION_TYPE_MISSION);
                writeMavlinkData(&appdataInt.ap,&msg);
            }
            }

            break;
        }

        case MAVLINK_MSG_ID_PARAM_SET:
        {
            //printf("MAVLINK_MSG_ID_PARAM_SET\n");
            mavlink_param_set_t msg;
            mavlink_msg_param_set_decode(&message, &msg);

            //Store value of the parameter locally and send confirmation back to GS
            //Determine which index to save the parameter to
            for (int i = 0; i < PARAM_COUNT; i++) {
                if (strcmp(msg.param_id, appdataInt.storedparams[i].param_id)==0) {
                    //Store the param_value message at the param_index, i
                    appdataInt.storedparams[i].value = msg.param_value;
                    appdataInt.storedparams[i].type = msg.param_type;

                    //Send a param_value message back to the GS as confirmation
                    mavlink_message_t param_value_msg;
                    mavlink_msg_param_value_pack(sysid_ic, compid_ic, &param_value_msg,
                                                 msg.param_id,
                                                 msg.param_value,
                                                 msg.param_type,
                                                 PARAM_COUNT, i);
                    writeMavlinkData(&appdataInt.ap,&param_value_msg);
                    //OS_printf("%s, %f\n",appdataInt.storedparams[i].param_id,appdataInt.storedparams[i].value);
                    ap_startTimer(&appdataInt.pmtimer,ap_pmCallback,"PMTIMER",10000000,5000000);
                    break;
                }
            }

            break;
        }

        case MAVLINK_MSG_ID_PARAM_REQUEST_LIST:
        {
            //printf("MAVLINK_MSG_ID_REQUEST_LIST\n");
            mavlink_param_request_list_t msg;
            mavlink_msg_param_request_list_decode(&message, &msg);

            //Send all locally stored parameters to the GS as mavlink PARAM_VALUE messages
            for (int i = 0; i < PARAM_COUNT; i++) {
                mavlink_message_t param_value_msg;
                mavlink_msg_param_value_pack(sysid_ic, compid_ic, &param_value_msg,
                                                 appdataInt.storedparams[i].param_id,
                                                 appdataInt.storedparams[i].value,
                                                 appdataInt.storedparams[i].type,
                                                 PARAM_COUNT, i);

                writeMavlinkData(&appdataInt.ap,&param_value_msg);
                //printf("Sending parameter : %s: %f\n",appdataIntGS.param_ids[i],appdataIntGS.params[i].param_value);
            }

            break;
        }

        case MAVLINK_MSG_ID_PARAM_REQUEST_READ:
        {
            //printf("MAVLINK_MSG_ID_REQUEST_READ\n");
            mavlink_param_request_read_t msg;
            mavlink_msg_param_request_read_decode(&message, &msg);

            //Send requested parameter as a mavlink PARAM_VALUE message
            //(Assume valid param_index is given, so no need to look up params by param_id)
            mavlink_message_t param_value_msg;
            mavlink_msg_param_value_pack(sysid_ic, compid_ic, &param_value_msg,
                                         appdataInt.storedparams[msg.param_index].param_id,
                                         appdataInt.storedparams[msg.param_index].value,
                                         appdataInt.storedparams[msg.param_index].type,
                                         PARAM_COUNT, msg.param_index);
            writeMavlinkData(&appdataInt.ap,&param_value_msg);
            //printf("Requested param id : %s\n",msg.param_id);
            //printf("Requested param index : %d\n",msg.param_index);
            //printf("Sending parameter : %s: %f\n",appdataIntGS.param_ids[msg.param_index],appdataIntGS.params[msg.param_index].param_value);

            break;
        }


       
        case MAVLINK_MSG_ID_FENCE_POINT:
        {
            uint16_t index = appdataInt.recvGeofIndex;

            mavlink_fence_point_t msg;
            mavlink_msg_fence_point_decode(&message,&msg);
            int count = msg.idx;
            int total = msg.count;

            appdataInt.fenceVertices[index] = total;

            appdataInt.gfData[index].vertices[msg.idx][0] = msg.lat;
            appdataInt.gfData[index].vertices[msg.idx][1] = msg.lng;

            if (count < total-1) {
                appdataInt.rcv_gf_seq = count + 1;
                //mavlink_message_t fetchfence;
                //mavlink_msg_fence_fetch_point_pack(sysid,compid,&fetchfence,target_sys,target_comp,count+1);
                //writeMavlinkData(&appdataIntGS.gs,&fetchfence);

                ap_startTimer(&appdataInt.gftimer,ap_gfCallback,"GFTIMER",1000,1000000);

            }else{
                ap_stopTimer(&appdataInt.gftimer);
                mavlink_message_t ack;
                mavlink_msg_command_ack_pack(sysid_ic,compid_ic,&ack,MAV_CMD_DO_FENCE_ENABLE,MAV_RESULT_ACCEPTED);

                writeMavlinkData(&appdataInt.ap,&ack);
            }

            if(count == total - 1) {
                geofence_t data2send;

                memcpy(&data2send,appdataInt.gfData + index,sizeof(geofence_t));

                CFE_SB_InitMsg(&data2send,ICAROUS_GEOFENCE_MID,sizeof(geofence_t),FALSE);
                SendSBMsg(data2send);	
            }

            break;
        }

        case MAVLINK_MSG_ID_COMMAND_LONG:
        {
            //printf("MAVLINK_MSG_ID_COMMAND_LONG\n");
            mavlink_command_long_t msg;
            mavlink_msg_command_long_decode(&message, &msg);

            if (msg.command == MAV_CMD_MISSION_START) {

                if (appdataInt.numWaypoints > 1)
                {

                    apInterface_PublishParams();
                    startMission.param1 = msg.param1;
                    CFE_SB_TimeStampMsg((CFE_SB_Msg_t *)&startMission);
                    CFE_SB_SendMsg((CFE_SB_Msg_t *)&startMission);

                    mavlink_message_t statusMsg;
                    mavlink_msg_statustext_pack(sysid_ic,compid_ic,&statusMsg,MAV_SEVERITY_INFO,"IC:Starting Mission");
                    writeMavlinkData(&appdataInt.ap,&statusMsg);
                }else{
                    mavlink_message_t statusMsg;
                    mavlink_msg_statustext_pack(sysid_ic,compid_ic,&statusMsg,MAV_SEVERITY_WARNING,"IC:No Flight Plan loaded");
                    writeMavlinkData(&appdataInt.ap,&statusMsg);

                }
            }else if(msg.command == MAV_CMD_USER_1){
                mavlink_message_t reqlist;
				mavlink_msg_mission_request_list_pack(sysid_ic,compid_ic,&reqlist,sysid_ap,compid_ap,MAV_MISSION_TYPE_MISSION);
                writeMavlinkData(&appdataInt.ap,&msg);

                argsCmd_t resetIcarous;
                CFE_SB_InitMsg(&resetIcarous,ICAROUS_RESET_MID,sizeof(argsCmd_t),TRUE);
                resetIcarous.param1 = msg.param1;
                SendSBMsg(resetIcarous);
            }else if (msg.command == MAV_CMD_DO_FENCE_ENABLE) {
                uint16_t index = msg.param2; 
                appdataInt.recvGeofIndex = index;
                if(index >= appdataInt.numGeofences){
                    appdataInt.numGeofences++;
                }

                appdataInt.gfData[index].index = (uint16_t)msg.param2;
                appdataInt.gfData[index].type = (uint8_t)msg.param3;
                appdataInt.gfData[index].totalvertices = (uint16_t)msg.param4;
                appdataInt.gfData[index].floor = msg.param5;
                appdataInt.gfData[index].ceiling = msg.param6;
                appdataInt.rcv_gf_seq = 0;

                ap_startTimer(&appdataInt.gftimer,ap_gfCallback,"GFTIMER",1000,1000000);
            }
            break;

        }
    }
}

void ARDUPILOT_ProcessPacket() {
    CFE_SB_MsgId_t  MsgId;


    MsgId = CFE_SB_GetMsgId(appdataInt.INTERFACEMsgPtr);
    switch (MsgId)
    {

        case ICAROUS_BANDS_TRACK_MID:
        {
            mavlink_message_t msg;
            bands_t* bands = (bands_t*) appdataInt.INTERFACEMsgPtr;
            mavlink_msg_icarous_kinematic_bands_pack(sysid_ic,compid_ic,&msg,(int8_t)bands->numBands,
                    (uint8_t)bands->type[0],(float)bands->min[0],(float)bands->max[0],
                    (uint8_t)bands->type[1],(float)bands->min[1],(float)bands->max[1],
                    (uint8_t)bands->type[2],(float)bands->min[2],(float)bands->max[2],
                    (uint8_t)bands->type[3],(float)bands->min[3],(float)bands->max[3],
                    (uint8_t)bands->type[4],(float)bands->min[4],(float)bands->max[4]);


            if(bands->numBands > 0) {
                writeMavlinkData(&appdataInt.ap, &msg);
            }
            break;
        }

        case ICAROUS_FLIGHTPLAN_MID:{
            flightplan_t* msg = (flightplan_t*)appdataInt.INTERFACEMsgPtr;
            memcpy(&fpdata,msg,sizeof(flightplan_t));
            int count = apConvertPlanToMissionItems(&fpdata);
            appdataInt.numWaypoints = count;
            break;
        }

        case ICAROUS_TRAFFIC_MID:{
            object_t *traffic = (object_t *)appdataInt.INTERFACEMsgPtr;
            mavlink_message_t msg;

            uint8_t emitterType = 0;
            if (traffic->type != _TRAFFIC_RADAR_)
            {
                emitterType = 100;
            }else if(traffic->type == _TRAFFIC_SIM_){
                emitterType = 255;
            }else if(traffic->type == _TRAFFIC_ADSB_){
                break;
            }

            double heading = fmod(2 * M_PI + atan2(traffic->ve, traffic->vn), 2 * M_PI) * 180 / M_PI;
            double speed = sqrt(traffic->vn * traffic->vn + traffic->ve * traffic->ve);
            mavlink_msg_adsb_vehicle_pack(sysid_ic, compid_ic, &msg, (uint8_t)traffic->index,
                                          (int32_t)(traffic->latitude * 1E7),
                                          (int32_t)(traffic->longitude * 1E7),
                                          ADSB_ALTITUDE_TYPE_GEOMETRIC,
                                          (int32_t)(traffic->altitude * 1E3),
                                          (uint16_t)(heading * 1E2),
                                          (uint16_t)(speed * 1E2),
                                          (uint16_t)(traffic->vd * 1E2),
                                          "NONE", emitterType, 1, 1, 0);

            writeMavlinkData(&appdataInt.ap, &msg);
            break;
        }

        case ICAROUS_TRAJECTORY_MID:{
            flightplan_t* fp = (flightplan_t*) appdataInt.INTERFACEMsgPtr;
            memcpy(&appdataInt.trajectory, fp, sizeof(flightplan_t));

            ap_startTimer(&appdataInt.tjtimer,ap_tjCallback,"TJTIMER1",1000,1000000);   
            break;
        }


        case ICAROUS_STATUS_MID:{
            status_t* statusMsg = (status_t*) appdataInt.INTERFACEMsgPtr;
            mavlink_message_t msg;
            mavlink_msg_statustext_pack(sysid_ic,compid_ic,&msg,statusMsg->severity,statusMsg->buffer);
            writeMavlinkData(&appdataInt.ap,&msg);
            break;
        }

        case UPLINK_FLIGHTPLAN_MID:{
            flightplan_t* msg = (flightplan_t*)appdataInt.INTERFACEMsgPtr;
            memcpy(&fpdata,msg,sizeof(flightplan_t));
            int count = apConvertPlanToMissionItems(&fpdata);
            appdataInt.numWaypoints = count;
            mavlink_message_t missionCount;
            mavlink_msg_mission_count_pack(sysid_ic,compid_ic,&missionCount,sysid_ap,compid_ap,count,MAV_MISSION_TYPE_MISSION);
            writeMavlinkData(&appdataInt.ap,&missionCount);
            appdataInt.startWPUplink = true;
        }
        case ICAROUS_STARTMISSION_MID:
        {
            // Request vehicle home position
            //OS_printf("Requested home position\n");
            mavlink_message_t msg;
            mavlink_msg_command_int_pack(255,0,&msg,1,1,0,MAV_CMD_GET_HOME_POSITION,0,0,0,0,0,0,0,0,0);
            writeMavlinkData(&appdataInt.ap,&msg);
            break;
        }

        case SAFE2DITCH_STATUS_MID: {
            safe2ditchStatus_t* s2dStatus = (safe2ditchStatus_t*) appdataInt.INTERFACEMsgPtr;

            if (s2dStatus->ditchsite[2] < -1E3){
                break;
            }

            mavlink_message_t msg;

            mavlink_msg_command_long_pack(1,0,&msg,1,0,
                                          MAV_CMD_USER_5,0,
                                          s2dStatus->ditchsite[0],
                                          s2dStatus->ditchsite[1],
                                          s2dStatus->ditchsite[2],
                                          0, 0, 0, 0);

            //printf("IC sending ditch site to GCS\n");
            //printf("(%f, %f, %f)\n", s2dStatus->ditchsite[0],s2dStatus->ditchsite[1],s2dStatus->ditchsite[2]);
            writeMavlinkData(&appdataInt.ap,&msg);
            break;
            }

        case ICAROUS_COMMANDS_MID:
        {
            argsCmd_t *cmd = (argsCmd_t*) appdataInt.INTERFACEMsgPtr;
            mavlink_message_t msg;
            controlMode_e mode;
            switch (cmd->name) {

                case _ARM_:
                {
                    mavlink_msg_command_long_pack(255,0,&msg,1,0,MAV_CMD_COMPONENT_ARM_DISARM,0,cmd->param1,0,0,0,0,0,0);
                    break;
                }

                case _TAKEOFF_:
                {
                    mavlink_msg_command_long_pack(255,0,&msg,1,0,MAV_CMD_NAV_TAKEOFF,0,1,0,0,0,0,0,cmd->param1);
                    break;
                }

                case _SETMODE_:
                {
                    if ((int)cmd->param1 == _PASSIVE_) {
                        mode = AUTO;
                        appdataInt.icarousMode = 0;
                    }else if ((int)cmd->param1 == _ACTIVE_) {
                        mode = GUIDED;
                        appdataInt.icarousMode = 1;
                    }
                    mavlink_msg_set_mode_pack(255,0,&msg,0,1,mode);
                    break;
                }

                case _LAND_:
                {
                    mavlink_msg_command_long_pack(255,0,&msg,1,0,MAV_CMD_NAV_LAND,0,1,0,0,0,cmd->param5,cmd->param6,cmd->param7);
                    break;
                }

                case _GOTOWP_:
                {

                    if(appdataInt.numWaypoints == 0){
                        break;
                    }

                    int tempSeq = (int)cmd->param1;
                    int seq = appdataInt.waypoint_index[tempSeq];
                    mavlink_msg_mission_set_current_pack(255,0,&msg,1,0,seq);

                    if(tempSeq>0) {
                        wpreached.reachedwaypoint = (uint8_t) (tempSeq - 1);
                        wpreached.feedback = false;
                    }
                    SendSBMsg(wpreached);
                    break;
                }

                case _SETPOS_:
                {
                    mavlink_msg_set_position_target_global_int_pack(255,0,&msg,0,1,0,MAV_FRAME_GLOBAL_RELATIVE_ALT_INT,
                                                                    0b0000111111111000,(int)(cmd->param1*1E7),(int)(cmd->param2*1E7),(cmd->param3),
                                                                    0,0,0,0,0,0,0,0);
                    break;
                }

                case _SETVEL_:
                {
                    mavlink_msg_set_position_target_local_ned_pack(255,0,&msg,0,1,0,MAV_FRAME_LOCAL_NED, 0b0000111111000111,0,0,0,
                                                                   (float)cmd->param1,(float)cmd->param2,(float)cmd->param3,
                                                                   0,0,0,0,0);

                    break;
                }

                case _SETYAW_:
                {
                    mavlink_msg_command_long_pack(255,0,&msg,1,0,MAV_CMD_CONDITION_YAW,0,
                                                  (float)cmd->param1,(float)cmd->param2,(float)cmd->param3,(float)cmd->param4,0,0,0);
                    break;
                }

                case _SETSPEED_:
                {
                    mavlink_msg_command_long_pack(255,0,&msg,1,0,MAV_CMD_DO_CHANGE_SPEED,0,
                                                  1,(float)cmd->param1,0,0,0,0,0);
                    break;
                }

                case _STATUS_: {
                    mavlink_msg_statustext_pack(255, 0, &msg, MAV_SEVERITY_WARNING, cmd->buffer);
                    break;
                }

                case _GETFP_:{
                    appdataInt.startWPDownlink = true;
				    mavlink_message_t msg;
				    mavlink_msg_mission_request_list_pack(255,0,&msg,1,0,MAV_MISSION_TYPE_MISSION);
                    break;
                }

                default:{

                }

            }

            writeMavlinkData(&appdataInt.ap,&msg);
            break;
        }

    }

    return;
}

uint16_t apConvertPlanToMissionItems(flightplan_t* fp){
    int count = 0;
    for(int i=0;i<fp->num_waypoints;++i){
        appdataInt.UplinkMissionItems[count].target_system = 1;
        appdataInt.UplinkMissionItems[count].target_component = 0;
        appdataInt.UplinkMissionItems[count].seq = (uint16_t )count;
        appdataInt.UplinkMissionItems[count].mission_type = MAV_MISSION_TYPE_MISSION;
        appdataInt.UplinkMissionItems[count].command = MAV_CMD_NAV_WAYPOINT;
        appdataInt.UplinkMissionItems[count].frame = MAV_FRAME_GLOBAL_RELATIVE_ALT;
        appdataInt.UplinkMissionItems[count].autocontinue = 1;
        appdataInt.UplinkMissionItems[count].current = 0;
        appdataInt.UplinkMissionItems[count].x = (float)fp->waypoints[i].latitude;
        appdataInt.UplinkMissionItems[count].y = (float)fp->waypoints[i].longitude;
        appdataInt.UplinkMissionItems[count].z = (float)fp->waypoints[i].altitude;
        appdataInt.waypoint_index[i] = count;

        count++;
        if(i < fp->num_waypoints-1){
            if(fp->waypoints[i].wp_metric == WP_METRIC_ETA) {
                double currentWP[3] = {fp->waypoints[i].latitude, fp->waypoints[i].longitude,
                                       fp->waypoints[i].altitude};
                double nextWP[3] = {fp->waypoints[i + 1].latitude, fp->waypoints[i + 1].longitude,
                                    fp->waypoints[i + 1].altitude};
                double dist2NextWP = ComputeDistance(currentWP, nextWP);
                double time2NextWP = fp->waypoints[i].value_to_next_wp;
                double speed2NextWP = dist2NextWP/time2NextWP;

                if (dist2NextWP > 1E-3) {
                    appdataInt.UplinkMissionItems[count].target_system = 1;
                    appdataInt.UplinkMissionItems[count].target_component = 0;
                    appdataInt.UplinkMissionItems[count].seq = (uint16_t) count;
                    appdataInt.UplinkMissionItems[count].mission_type = MAV_MISSION_TYPE_MISSION;
                    appdataInt.UplinkMissionItems[count].command = MAV_CMD_DO_CHANGE_SPEED;
                    appdataInt.UplinkMissionItems[count].frame = MAV_FRAME_GLOBAL_RELATIVE_ALT;
                    appdataInt.UplinkMissionItems[count].autocontinue = 1;
                    appdataInt.UplinkMissionItems[count].current = 0;
                    appdataInt.UplinkMissionItems[count].param1 = 1;
                    appdataInt.UplinkMissionItems[count].param2 = speed2NextWP;
                    appdataInt.UplinkMissionItems[count].param3 = 0;
                    appdataInt.UplinkMissionItems[count].param4 = 0;
                    appdataInt.UplinkMissionItems[count].x = 0;
                    appdataInt.UplinkMissionItems[count].y = 0;
                    appdataInt.UplinkMissionItems[count].z = 0;
                    count++;
                    //OS_printf("Constructed speed waypoint:%f\n",speed2NextWP);
                }else if(time2NextWP > 0){
                    appdataInt.UplinkMissionItems[count].target_system = 1;
                    appdataInt.UplinkMissionItems[count].target_component = 0;
                    appdataInt.UplinkMissionItems[count].seq = (uint16_t) count;
                    appdataInt.UplinkMissionItems[count].mission_type = MAV_MISSION_TYPE_MISSION;
                    appdataInt.UplinkMissionItems[count].command = MAV_CMD_NAV_LOITER_TIME;
                    appdataInt.UplinkMissionItems[count].frame = MAV_FRAME_GLOBAL_RELATIVE_ALT;
                    appdataInt.UplinkMissionItems[count].autocontinue = 1;
                    appdataInt.UplinkMissionItems[count].current = 0;
                    appdataInt.UplinkMissionItems[count].param1 = time2NextWP;
                    appdataInt.UplinkMissionItems[count].param2 = 0;
                    appdataInt.UplinkMissionItems[count].param3 = 15;
                    appdataInt.UplinkMissionItems[count].param4 = 0;
                    appdataInt.UplinkMissionItems[count].x = nextWP[0];
                    appdataInt.UplinkMissionItems[count].y = nextWP[1];
                    appdataInt.UplinkMissionItems[count].z = nextWP[2];
                    ++i;
                    count++;
                    //OS_printf("Constructed loiter waypoint\n");
                }
            }
        }

    }

    appdataInt.numUplinkWaypoints = count;
    return count;
}

void apConvertMissionItemsToPlan(uint16_t  size, mavlink_mission_item_t items[],flightplan_t* fp){
    int count = 0;
    for(int i=0;i<size;++i){
        switch(items[i].command){

            case MAV_CMD_NAV_WAYPOINT: {
                fp->waypoints[count].latitude = items[i].x;
                fp->waypoints[count].longitude = items[i].y;
                fp->waypoints[count].altitude = items[i].z;
                fp->waypoints[count].wp_metric = WP_METRIC_NONE;
                count++;
                //OS_printf("constructed waypoint\n");
                break;
            }

            case MAV_CMD_NAV_SPLINE_WAYPOINT:{
                fp->waypoints[count].latitude = items[i].x;
                fp->waypoints[count].longitude = items[i].y;
                fp->waypoints[count].altitude = items[i].z;
                fp->waypoints[count].wp_metric = WP_METRIC_NONE;
                count++;
                break;
            }

            case MAV_CMD_NAV_LOITER_TIME:{
                fp->waypoints[count].latitude = items[i].x;
                fp->waypoints[count].longitude = items[i].y;
                fp->waypoints[count].altitude = items[i].z;
                if(items[i].command == MAV_CMD_NAV_LOITER_TIME){
                    fp->waypoints[count].wp_metric = WP_METRIC_ETA;
                    fp->waypoints[count-1].value_to_next_wp = items[i].param1;

                }
                count++;
                //OS_printf("Setting loiter point %f\n",items[i].param1);
                break;
            }

            case MAV_CMD_DO_CHANGE_SPEED:{
                if(i>0 && i < size-1) {
                    double wpA[3] = {items[i - 1].x, items[i - 1].y, items[i - 1].z};
                    double wpB[3] = {items[i + 1].x,items[i + 1].y,items[i + 1].z};
                    double dist = ComputeDistance(wpA,wpB);
                    double eta = dist/items[i].param2;
                    fp->waypoints[count-1].value_to_next_wp = eta;
                    fp->waypoints[count-1].wp_metric = WP_METRIC_ETA;
                    //OS_printf("Setting ETA to %f\n",eta);
                }
                break;
            }
        }
    }

    fp->num_waypoints = count;
}

void ap_startTimer(uint32_t *timerID,void (*f)(uint32_t),char name[],uint32_t startTime,uint32_t intvl){

    ap_stopTimer(timerID);

    int32 clockacc;
    int32 status = OS_TimerCreate(timerID,name,&clockacc,f);
    if(status != CFE_SUCCESS){
            OS_printf("Could not create timer: %s, %d\n",name,status);
    }
    status = OS_TimerSet(*timerID,startTime,intvl);
    if(status != CFE_SUCCESS){
            OS_printf("Could not set timer: %s\n",name);
    }

}

void ap_stopTimer(uint32_t *timerID){
    if(*timerID != 0xffff){
        OS_TimerDelete(*timerID);
        *timerID = 0xffff;
    }
}

void ap_wpCallback(uint32_t timerId)
{
    //OS_printf("timer callback : %d\n",appdataIntGS.receivingWP);
    mavlink_message_t msgRequest;
    mavlink_msg_mission_request_pack(sysid_ic, compid_ic, &msgRequest, sysid_gs, compid_gs, appdataInt.receivingWP,
                                     MAV_MISSION_TYPE_MISSION);
    writeMavlinkData(&appdataInt.ap, &msgRequest);
}

void ap_gfCallback(uint32_t timerId)
{
    mavlink_message_t fetchfence;
    mavlink_msg_fence_fetch_point_pack(sysid_ic, compid_ic, &fetchfence, sysid_gs, compid_gs, appdataInt.rcv_gf_seq);
    writeMavlinkData(&appdataInt.ap, &fetchfence);
}

void ap_pmCallback(uint32_t timerId){
    apInterface_PublishParams();
    ap_stopTimer(&timerId);
    mavlink_message_t statusMsg;
    mavlink_msg_statustext_pack(sysid_ic,compid_ic,&statusMsg,MAV_SEVERITY_INFO,"IC:Publishing parameters");
    writeMavlinkData(&appdataInt.ap, &statusMsg);
}

void ap_tjCallback(uint32_t timerId)
{
    mavlink_message_t msg;
    mavlink_msg_mission_count_pack(sysid_ic,compid_ic,&msg,sysid_gs,compid_gs,appdataInt.trajectory.num_waypoints,MAV_MISSION_TYPE_RALLY);
    writeMavlinkData(&appdataInt.ap,&msg);
}

void apInterface_PublishParams() {
    //Initialize SB messages
    traffic_parameters_t localTrafficParams;
    tracking_parameters_t localTrackingParams;
    trajectory_parameters_t localTrajectoryParams;
    geofence_parameters_t localGeofenceParams;
    CFE_SB_InitMsg(&localTrafficParams,TRAFFIC_PARAMETERS_MID,sizeof(traffic_parameters_t),TRUE);
    CFE_SB_InitMsg(&localTrackingParams,TRACKING_PARAMETERS_MID,sizeof(tracking_parameters_t),TRUE);
    CFE_SB_InitMsg(&localTrajectoryParams,TRAJECTORY_PARAMETERS_MID,sizeof(trajectory_parameters_t),TRUE);
    CFE_SB_InitMsg(&localGeofenceParams,GEOFENCE_PARAMETERS_MID,sizeof(geofence_parameters_t),TRUE);

    //Store the locally saved parameters to the messages
    int i = 0;
    //Traffic Parameters
    localTrafficParams.trafficSource = (uint32_t) apNextParam;
    localTrafficParams.logDAAdata = (bool) apNextParam;
    localTrafficParams.lookahead_time = apNextParam;
    localTrafficParams.left_trk = apNextParam;
    localTrafficParams.right_trk = apNextParam;
    localTrafficParams.min_gs = apNextParam;
    localTrafficParams.max_gs = apNextParam;
    localTrafficParams.min_vs = apNextParam;
    localTrafficParams.max_vs = apNextParam;
    localTrafficParams.min_alt = apNextParam;
    localTrafficParams.max_alt = apNextParam;
    localTrafficParams.trk_step = apNextParam;
    localTrafficParams.gs_step = apNextParam;
    localTrafficParams.vs_step = apNextParam;
    localTrafficParams.alt_step = apNextParam;
    localTrafficParams.horizontal_accel = apNextParam;
    localTrafficParams.vertical_accel = apNextParam;
    localTrafficParams.turn_rate = apNextParam;
    localTrafficParams.bank_angle = apNextParam;
    localTrafficParams.vertical_rate = apNextParam;
    localTrafficParams.recovery_stability_time = apNextParam;
    localTrafficParams.min_horizontal_recovery = apNextParam;
    localTrafficParams.min_vertical_recovery = apNextParam;
    localTrafficParams.recovery_trk = (bool) apNextParam;
    localTrafficParams.recovery_gs = (bool) apNextParam;
    localTrafficParams.recovery_vs = (bool) apNextParam;
    localTrafficParams.recovery_alt = (bool) apNextParam;
    localTrafficParams.ca_bands = (bool) apNextParam;
    localTrafficParams.ca_factor = apNextParam;
    localTrafficParams.horizontal_nmac = apNextParam;
    localTrafficParams.vertical_nmac = apNextParam;
    localTrafficParams.conflict_crit = (bool) apNextParam;
    localTrafficParams.recovery_crit = (bool) apNextParam;
    localTrafficParams.contour_thr = apNextParam;
    localTrafficParams.alert_1_alerting_time = apNextParam;
    strcpy(localTrafficParams.alert_1_detector, "det_1");   //Hard coded, not parameter
    //apNextParam;
    localTrafficParams.alert_1_early_alerting_time = apNextParam;
    strcpy(localTrafficParams.alert_1_region, "NEAR");      //Hard coded, not parameter
    //apNextParam;
    localTrafficParams.alert_1_spread_alt = apNextParam;
    localTrafficParams.alert_1_spread_gs = apNextParam;
    localTrafficParams.alert_1_spread_trk = apNextParam;
    localTrafficParams.alert_1_spread_vs = apNextParam;
    localTrafficParams.conflict_level = (uint8_t) apNextParam;
    strcpy(localTrafficParams.load_core_detection_det_1, "WCV_TAUMOD"); //Hard coded, not parameter
    //apNextParam;
    localTrafficParams.det_1_WCV_DTHR = apNextParam;
    localTrafficParams.det_1_WCV_TCOA = apNextParam;
    localTrafficParams.det_1_WCV_TTHR = apNextParam;
    localTrafficParams.det_1_WCV_ZTHR = apNextParam;
    // Tracking parameters
    localTrackingParams.command = (bool) apNextParam;
    localTrackingParams.trackingObjId = (int) apNextParam;
    localTrackingParams.pGainX = (double) apNextParam;
    localTrackingParams.pGainY = (double) apNextParam;
    localTrackingParams.pGainZ = (double) apNextParam;
    localTrackingParams.heading = (double) apNextParam;
    localTrackingParams.distH = (double) apNextParam;
    localTrackingParams.distV = (double) apNextParam;
    // Trajectory parameters
    localTrajectoryParams.obsbuffer = (double) apNextParam;
    localTrajectoryParams.maxCeiling = (double) apNextParam;
    localTrajectoryParams.astar_enable3D = (bool) apNextParam;
    localTrajectoryParams.astar_gridSize = (double) apNextParam;
    localTrajectoryParams.astar_resSpeed = (double) apNextParam;
    localTrajectoryParams.astar_lookahead = (double) apNextParam;
    strcpy(localTrajectoryParams.astar_daaConfigFile, "../ram/DaidalusQuadConfig.txt");   //Hard coded, not parameter
    //apNextParam;
    localTrajectoryParams.rrt_resSpeed = (double) apNextParam;
    localTrajectoryParams.rrt_numIterations = (int) apNextParam;
    localTrajectoryParams.rrt_dt = (double) apNextParam;
    localTrajectoryParams.rrt_macroSteps = (int) apNextParam;
    localTrajectoryParams.rrt_capR = (double) apNextParam;
    strcpy(localTrajectoryParams.rrt_daaConfigFile, "../ram/DaidalusQuadConfig.txt");     //Hard coded, not parameter
    //apNextParam;
    localTrajectoryParams.xtrkDev = (double) apNextParam;
    localTrajectoryParams.xtrkGain = (double) apNextParam;
    localTrajectoryParams.resSpeed = (double) apNextParam;
    localTrajectoryParams.searchAlgorithm = (uint8_t) apNextParam;
    // Geofence parameters
    localGeofenceParams.lookahead = (double) apNextParam;
    localGeofenceParams.hthreshold = (double) apNextParam;
    localGeofenceParams.vthreshold = (double) apNextParam;
    localGeofenceParams.hstepback = (double) apNextParam;
    localGeofenceParams.vstepback = (double) apNextParam;

    //Send Traffic Params SB message
    //OS_printf("recorded all params\n");

    SendSBMsg(localTrafficParams);
    SendSBMsg(localTrackingParams);
    SendSBMsg(localTrajectoryParams);
    SendSBMsg(localGeofenceParams);

    //printf("gsInterface Sent SB Param Messages\n");
}
