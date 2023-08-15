#pragma once

#ifndef INTERNAL_TESTS 
    #define LOG(X)
    #define LOGI(X)
#else
    #define LOG(X) Serial.println(X)
    #define LOGI(X) Serial.print(X)
#endif
#define S_BIT(n,b) n |= (1<<(b))
#define C_BIT(n,b) n &= ~(1<<(b))

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

void selfTestOnInit();
void selfTestOnLoop();