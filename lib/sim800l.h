#ifndef SIM800L_H
#define SIM800L_H

extern void SIM800L_Init(void);
extern void sendMQTTMessage(const char* clientId, const char* topic, const char* message);

#endif /* SIM800L_H */