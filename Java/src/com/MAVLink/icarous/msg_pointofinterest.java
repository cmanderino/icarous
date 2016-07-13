/* AUTO-GENERATED FILE.  DO NOT MODIFY.
 *
 * This class was automatically generated by the
 * java mavlink generator tool. It should not be modified by hand.
 */

// MESSAGE POINTOFINTEREST PACKING
package com.MAVLink.icarous;
import com.MAVLink.MAVLinkPacket;
import com.MAVLink.Messages.MAVLinkMessage;
import com.MAVLink.Messages.MAVLinkPayload;
        
/**
* Point in space (Waypoint/Vertex etc...)
*/
public class msg_pointofinterest extends MAVLinkMessage{

    public static final int MAVLINK_MSG_ID_POINTOFINTEREST = 220;
    public static final int MAVLINK_MSG_LENGTH = 32;
    private static final long serialVersionUID = MAVLINK_MSG_ID_POINTOFINTEREST;


      
    /**
    * Latitude (deg)/Relative North (m)
    */
    public float latx;
      
    /**
    * Longitude (deg)/Relative East (m)
    */
    public float lony;
      
    /**
    * Altitude (deg)/Relative UP (m)
    */
    public float altz;
      
    /**
    * Heading
    */
    public float heading;
      
    /**
    * Velocity (north m/s)
    */
    public float vx;
      
    /**
    * Velocity (east m/s)
    */
    public float vy;
      
    /**
    * Velocity (down m/s)
    */
    public float vz;
      
    /**
    * Identifier (0-Waypoint,1-geofence,2-Obstacle,3-Traffic,4-Others)
    */
    public byte id;
      
    /**
    * Sub type
    */
    public byte subtype;
      
    /**
    * Index (sequence number)
    */
    public byte index;
      
    /**
    * 1 for lat/lon/alt,0 for NED
    */
    public byte geodesic;
    

    /**
    * Generates the payload for a mavlink message for a message of this type
    * @return
    */
    public MAVLinkPacket pack(){
        MAVLinkPacket packet = new MAVLinkPacket(MAVLINK_MSG_LENGTH);
        packet.sysid = 255;
        packet.compid = 190;
        packet.msgid = MAVLINK_MSG_ID_POINTOFINTEREST;
              
        packet.payload.putFloat(latx);
              
        packet.payload.putFloat(lony);
              
        packet.payload.putFloat(altz);
              
        packet.payload.putFloat(heading);
              
        packet.payload.putFloat(vx);
              
        packet.payload.putFloat(vy);
              
        packet.payload.putFloat(vz);
              
        packet.payload.putByte(id);
              
        packet.payload.putByte(subtype);
              
        packet.payload.putByte(index);
              
        packet.payload.putByte(geodesic);
        
        return packet;
    }

    /**
    * Decode a pointofinterest message into this class fields
    *
    * @param payload The message to decode
    */
    public void unpack(MAVLinkPayload payload) {
        payload.resetIndex();
              
        this.latx = payload.getFloat();
              
        this.lony = payload.getFloat();
              
        this.altz = payload.getFloat();
              
        this.heading = payload.getFloat();
              
        this.vx = payload.getFloat();
              
        this.vy = payload.getFloat();
              
        this.vz = payload.getFloat();
              
        this.id = payload.getByte();
              
        this.subtype = payload.getByte();
              
        this.index = payload.getByte();
              
        this.geodesic = payload.getByte();
        
    }

    /**
    * Constructor for a new message, just initializes the msgid
    */
    public msg_pointofinterest(){
        msgid = MAVLINK_MSG_ID_POINTOFINTEREST;
    }

    /**
    * Constructor for a new message, initializes the message with the payload
    * from a mavlink packet
    *
    */
    public msg_pointofinterest(MAVLinkPacket mavLinkPacket){
        this.sysid = mavLinkPacket.sysid;
        this.compid = mavLinkPacket.compid;
        this.msgid = MAVLINK_MSG_ID_POINTOFINTEREST;
        unpack(mavLinkPacket.payload);        
    }

                          
    /**
    * Returns a string with the MSG name and data
    */
    public String toString(){
        return "MAVLINK_MSG_ID_POINTOFINTEREST -"+" latx:"+latx+" lony:"+lony+" altz:"+altz+" heading:"+heading+" vx:"+vx+" vy:"+vy+" vz:"+vz+" id:"+id+" subtype:"+subtype+" index:"+index+" geodesic:"+geodesic+"";
    }
}
        