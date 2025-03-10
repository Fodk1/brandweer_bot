#ifdef CORE_CM7
  #include <RPC.h>
  
    void setup() {
        Serial.begin(115200);
        RPC.begin();
    }
  
    void loop() {
        Serial.println("M7: Hello");
        while (RPC.available()) {
            Serial.write(RPC.read()); 
        }  
        delay(1000);
    }
#endif              

#ifdef CORE_CM4    
    #include <RPC.h>
 
    #define SerialRPC RPC
  
    void setup() {
        SerialRPC.begin();
    }
  
    void loop() {
        SerialRPC.println("M4: Hello"); 
        delay(3000);
    }
#endif            