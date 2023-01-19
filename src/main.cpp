#include "bridge.h"
#include <Arduino.h>
#include <Spidermesh.h>

Spidermesh smk900;
const int server_serial_port = 5556;
Bridge bridge(server_serial_port);
bool prevPollMode;

void WhenPacketReceived(apiframe packet);

void setup()
{

	Serial.begin(115200);

	//init the spiffs file system that contain the nodes definition
	initDrive();

	//set the mesh network speed at those parameters
	int nb_hops = 8;
	int duty_cycle = 5;
	int nb_byte_per_packet = 20;


	//callback that will be call at every packet received
	//smk900.setWhenPacketReceived(CallbackWhenPacketReceived);

	#ifdef TEST_AUTOMATIC_NODE_POLLING
	smk900.setCallbackAutomaticPolling(CallbackAutoPolling);
	#endif

	#if defined(TEST_REQUEST_BUILDER) && defined(TEST_AUTOMATIC_NODE_POLLING)
	smk900.setCallbackAutoRequestBuilder(CallbackAutoRequestBuilder);
	#endif


	//will start autopolling nodes listed in nodes.json file
	//if will send RF command to the virtual machine of each nodes and will use cbAutomaticPolling when contacted or unable to reach node
	#if !defined(TEST_OTA_UPDATE) && defined(TEST_AUTOMATIC_NODE_POLLING)
	smk900.enableAutomaticPolling();
	#endif

	#ifdef TEST_LOAD_EXTERNAL_FILES_DEFINITION
	smk900.setCallbackLoadExternalParamFiles(CallbackLoadExternalFileDefinition);
	#endif


	//to check or not all transaction with smk900 module
	smk900.setApiMsgFlag(true,true,false); 

	//radio will have already started to synch nodes at this point, but at the speed of mesh saved in eeprom
	smk900.begin(nb_hops,duty_cycle,nb_byte_per_packet);


    
    // Bridge TCP to Serial API
    bridge.init();
    smk900.cbWhenPacketReceived=WhenPacketReceived;
    bridge.cbAddPacketToSmkTxBuffer = static_cast<bool(*)(apiframe)> (smk900.addApiPacketLowPriority);
    bridge.cbWhenSmkPacketReceived = static_cast<void(*)(apiframe, String)> (printApiPacket);
    
    bridge.cbConnecting= [](){
        Serial.println("Bridge connected");
        prevPollMode = smk900.getAutoPolling();
        smk900.setAutoPolling(false);        
    };
    bridge.cbDisconnecting= [](){
        Serial.println("Bridge disconnected");
        smk900.setAutoPolling(prevPollMode);            
    };
}

void loop()
{

}



void WhenPacketReceived(apiframe packet)
{
    if(smk900.isInitDone())
    {
     bridge.WhenNewSmkPacketRx(packet);
    }  
}
