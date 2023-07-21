#pragma once

#define LOG(X) Serial.println(X)
#define S_BIT(n,b) n |= (1<<(b))
#define C_BIT(n,b) n &= ~(1<<(b))

void selfTestOnInit();
void selfTestOnLoop();